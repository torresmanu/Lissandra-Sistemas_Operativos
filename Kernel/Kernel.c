
#include "Kernel.h"

sem_t sNuevo; // Semáforo para el estado NEW
sem_t sListo; // Semáforo para el estado READY

pthread_mutex_t mNew;
pthread_mutex_t mReady;
pthread_mutex_t mExit;

int main(void) {

	sem_init(&sNuevo,0,0);
	sem_init(&sListo,0,0);

	pthread_mutex_init(&mNew,NULL);
	pthread_mutex_init(&mReady,NULL);
	pthread_mutex_init(&mExit,NULL);

	iniciar_programa();

	//Lanzamos tantos hilos como nivelMultiprocesamiento haya
	pthread_t *executer = malloc(nivelMultiprocesamiento * sizeof(pthread_t));
	for(int i=0; i<nivelMultiprocesamiento; i++ )
	{
	    pthread_create(&executer[i], NULL, (void*)ejecutador, NULL);
	}

	pthread_t plp; // Planificador a largo plazo
	pthread_create(&plp,NULL,(void*)planificadorLargoPlazo,NULL);

	pthread_t describeGlobal;
	pthread_create(&describeGlobal,NULL,(void*)realizarDescribeGlobal,NULL);

	leerConsola();											/// ACA COMIENZA A ITERAR Y LEER DE STDIN /////

	pthread_join(plp,NULL);
	pthread_join(describeGlobal,NULL);

	for(int i=0; i<nivelMultiprocesamiento; i++)
	{
		pthread_join(executer[i], NULL);
	}

	terminar_programa();


	printf("Termine programa\n");
	return 0;
}

void iniciar_programa(void)
{
	//Inicio el logger
	g_logger = log_create("Kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Kernel");

	//Inicio las configs
	g_config = config_create("Kernel.config");
	log_info(g_logger,"Configuraciones inicializadas");

	//Inicializo los estados
	iniciarEstados();

	//Obtengo el quantum
	quantum = config_get_int_value(g_config,"QUANTUM");

	//Nivel de multiprocesamiento
	nivelMultiprocesamiento = config_get_int_value(g_config,"MULTIPROCESAMIENTO");

	//Tasa de refresh de la metada
	metadataRefresh = config_get_int_value(g_config,"METADATA_REFRESH");

	pool = list_create();			// POOL DE MEMORIAS
	tablas = list_create();			// ESTRUCTURA QUE CONTIENE TODAS LAS TABLAS (METADATA)
	socketsPool = list_create();	// SOCKETS DE TODO EL POOL

	// Todoo para el describe
	obtenerMemoriaDescribe();
	gestionarConexionAMemoria(MemDescribe);

	iniciarCriterios();				/// INICIALIZO LISTAS DE CRITERIOS ///
	add(MemDescribe,&sc);

	//obtenerMemorias();				/// GENERO EL POOL DE MEMORIAS CON EL GOSSIPING DE LA MEMORIA EN EL .CONFIG ///
	//establecerConexionPool(); 		/// ACA ME CONECTO CON TODAS LAS MEMORIAS DEL POOL ///
}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Libero los estados y elimino sus elementos
	finalizarEstados();

	//Libero el pool de memorias
	list_destroy_and_destroy_elements(pool,destroy_nodo_memoria);

	//Libero las memorias de los criterios
	liberarCriterios();

}

int gestionarConexionAMemoria(Memoria* mem)
{
	struct addrinfo hints;
	struct addrinfo* serverInfo;

	memset(&hints, 0, sizeof(hints)); // Relleno con 0 toda la estructura de hints.
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(mem->ipMemoria,mem->puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	memoriaSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	int res =	connect(memoriaSocket, serverInfo->ai_addr, serverInfo->ai_addrlen); // Me conecto al socket

	if(res == -1)
	{
		log_error(g_logger, "Memoria innacesible: %s", strerror(errno));
	}
	else
	{
		log_info(g_logger, "Se conecto a la memoria N°:%d, listo para enviar scripts.",mem->id);
	}
	freeaddrinfo(serverInfo); // Libero

	// Envio un 1
	uint32_t codigo = 1;
	send(memoriaSocket,&codigo,sizeof(uint32_t),0);

	MemDescribe->socket = memoriaSocket;
	return memoriaSocket;
}


// Propias de Kernel

void agregarScriptAEstado(void* elem, nombreEstado estado)  // Aca hace las comprobaciones si pueden
{
	switch(estado){
		case NEW: {
			queue_push(new, (resultadoParser*)elem);
			break;
		}
		case READY: {
			queue_push(ready,(Script*)elem);
			break;
		}
		case EXEC: {
			if(nivelActual < nivelMultiprocesamiento){
				//queue_push(exec,script);
				nivelActual++;
			}
			else{
				// Que hacer si ya esta ejecutando 3 procesos a la vez?
				// pthread_join?
			}
			break;
		}
		case EXIT: {
			queue_push(exi,(Script*)elem);
			break;
		}
		default:
			break;
	}
}

void leerConsola(){

	resultado result;
	result.resultado = OK;

	printf("\nBienvenido! Welcome! Youkoso!\n");
	while(result.resultado != SALIR)
	{
		resultadoParser* res = malloc(sizeof(resultadoParser));

		char* linea = readline(">");					// Leo stdin
		if(linea)
			add_history(linea);							// Para recordar el comando

		resultadoParser aux = parseConsole(linea);
		memcpy(res,&aux,sizeof(resultadoParser));

		if(res->accionEjecutar==SALIR_CONSOLA){
			result.resultado = SALIR;
			printf("Entre en el if de salir_consola\n");
			return;
		}

		pthread_mutex_lock(&mNew);
		agregarScriptAEstado(res, NEW);
		pthread_mutex_unlock(&mNew);

		log_info(g_logger,"Agrego resParser con accion: %d a new\n",res->accionEjecutar);
		sem_post(&sNuevo);	// Habilito el estado NEW

		//free(linea); HAY QUE VOLVER A PONERLO
	}

	terminar_programa();
}

void planificadorLargoPlazo(){
	resultadoParser *r;

	while(1){
		sem_wait(&sNuevo);

		pthread_mutex_lock(&mNew);
		r = queue_pop(new);
		pthread_mutex_unlock(&mNew);

		Script *s = crearScript(r);

		pthread_mutex_lock(&mReady);
		agregarScriptAEstado(s,READY);
		pthread_mutex_unlock(&mReady);

		log_info(g_logger,"Paso el script a ready, cant rq:%d",s->instrucciones->elements_count);

		sem_post(&sListo);

		if(r->accionEjecutar==SALIR_CONSOLA)
			break;

	}
}

void ejecutador(){ // ACTUA COMO ESTADO EXEC
	status e;
	while(1){
		sem_wait(&sListo);

		pthread_mutex_lock(&mReady);
		Script *s = queue_pop(ready);
		pthread_mutex_unlock(&mReady);

		if(deboSalir(s))//hay que ver cuando termina, buscar una mejor forma
			return;


		for(int i=0; i <= quantum ;i++){ //ver caso en que falla, ejecutarS podria retornar un estado

			if(!terminoScript(s)){
				log_info(g_logger,"Voy a ejecutar un script con %d request",s->instrucciones->elements_count);
				e = ejecutarScript(s);

				if(e == REQUEST_ERROR)
				{
					mandarAexit(s);
					return;
				}
			} else {
				break;
			}

		}

		if(!terminoScript(s))
			mandarAready(s);
		else
			mandarAexit(s);
	}
}

bool deboSalir(Script *s){
	return ((resultadoParser *)list_get(s->instrucciones,0))-> accionEjecutar == SALIR_CONSOLA;
}

void mandarAready(Script *s){
	pthread_mutex_lock(&mReady);
	agregarScriptAEstado(s,READY);
	pthread_mutex_unlock(&mReady);

	sem_post(&sListo);
}

void mandarAexit(Script *s){
	pthread_mutex_lock(&mExit);
	agregarScriptAEstado(s,EXIT);
	pthread_mutex_unlock(&mExit);
}

////////////////////////////////////////////////////

void realizarDescribeGlobal()
{
	while(1)
	{
		describe();
		sleep(metadataRefresh/1000); // Lo paso a ms
	}
}

void describe()
{
	t_list* TablaLFS = list_create();
	int size;
	int status;
	int valueResponse;
	resultado res;
	accion acc;

	resultadoParser* describe = malloc(sizeof(resultadoParser));
	describe->accionEjecutar = DESCRIBE;
	contenidoDescribe* cd = malloc(sizeof(contenidoDescribe));
	cd->nombreTabla = NULL;
	describe->contenido = cd;


	char* msg = serializarPaquete(describe,&size);
	send(memoriaSocket, msg, size, 0);
	// Pido el describe a la memoria
	char* buffer = malloc(sizeof(int));
	valueResponse = recv(memoriaSocket,buffer,sizeof(int),0);
	memcpy(&acc,buffer,sizeof(int));								// Me fijo que accion para saber como deserializar

	if(valueResponse < 0)
	{
		log_error(g_logger,strerror(errno));
	}
	else if(valueResponse == 0)
	{
		log_error(g_logger,"Posiblemente la memoria se desconectó.");
	}
	else
	{
		res.accionEjecutar=acc;
		status = recibirYDeserializarRespuesta(memoriaSocket,&res); // Recibo la lista de tablas
		if(status<0)
			{
				log_error(g_logger,"Describe fallido");
			}
			else
			{
				TablaLFS = (t_list*)res.contenido;
				for(int i=0;i<list_size(TablaLFS);i++){
					metadataTabla* tabla = list_get(TablaLFS,i);
					printf("TABLA: %s;%s;%i\n",tabla->nombreTabla,tabla->consistency,tabla->partitions);
				}
				list_clean(tablas);						// Para no agregar repetidas
				list_add_all(tablas,TablaLFS);
				log_info(g_logger,"Describe global realizado con éxito");
				log_info(g_logger,"Cantidad de tablas indexadas: %d", tablas->elements_count);

				for(int i = 0; i<tablas->elements_count; i++)
				{
					printf("Tablas indexada n°:%d -> %s\n", i ,((metadataTabla*)list_get(tablas,i))->nombreTabla);
				}
			}
	}
	free(buffer);
	free(describe);
	free(msg);
	free(cd);
	list_destroy(TablaLFS);
}

void establecerConexionPool()
{
	// Tengo que ir estableciendo conexion con todas las memorias del pool, y agregar el socket a la lista

	Memoria* mem;
	int socket;
	t_list* aux;

	aux = list_create();

	for(int i = 0; i<pool->elements_count; i++)
	{
		mem = list_get(pool,i);
		socket = gestionarConexionAMemoria(mem);
		mem->socket = socket;
		list_add(aux,mem);
	}
	pool = aux; // Y obtengo una lista con todos los sockets
}


