#include "Kernel.h"

int main(void) {

	sem_init(&sNuevo,0,0);
	sem_init(&sListo,0,0);
	sem_init(&sDescribe,0,1);
	sem_init(&sRequest,0,0);

	pthread_mutex_init(&mNew,NULL);
	pthread_mutex_init(&mReady,NULL);
	pthread_mutex_init(&mExit,NULL);

	iniciar_programa();

	//Lanzamos tantos hilos como nivelMultiprocesamiento haya
	executer = malloc(nivelMultiprocesamiento * sizeof(pthread_t));
	for(int i=0; i<nivelMultiprocesamiento; i++ )
	{
	    pthread_create(&executer[i], NULL, (void*)ejecutador, NULL);
	}

	pthread_create(&plp,NULL,(void*)planificadorLargoPlazo,NULL);

	pthread_create(&describeGlobal,NULL,(void*)realizarDescribeGlobal,NULL);
	pthread_create(&gossipingAutomatico,NULL,(void*)realizarGossipingAutomatico,NULL);
	pthread_create(&monitoreador,NULL,(void*)controlConfig,NULL);
	pthread_create(&metricas,NULL,(void*)realizarMetrics,NULL);

	leerConsola();											/// ACA COMIENZA A ITERAR Y LEER DE STDIN /////

	while(finalizar.resultado != SALIR)
	{
		pthread_join(plp,NULL);
		pthread_join(describeGlobal,NULL);
		pthread_join(metricas,NULL);
		pthread_join(monitoreador,NULL);

		for(int i=0; i<nivelMultiprocesamiento; i++)
		{
			pthread_join(executer[i], NULL);
		}
	}

	terminar_programa();
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

	mutexsConexiones = dictionary_create();

	//Inicializo los estados
	iniciarEstados();

	//Obtengo el quantum
	quantum = config_get_int_value(g_config,"QUANTUM");

	//Nivel de multiprocesamiento
	nivelMultiprocesamiento = config_get_int_value(g_config,"MULTIPROCESAMIENTO");

	//Tasa de refresh de la metada
	metadataRefresh = config_get_int_value(g_config,"METADATA_REFRESH");

	//Tasa de refresh de la metada
	retardoGossiping = config_get_int_value(g_config,"RETARDO_GOSSIPING");

	//Tiempo de pausa en la ejecucion (NO USADA AUN)
	sleepEjecucion = config_get_int_value(g_config,"SLEEP_EJECUCION");

	//Pongo el WATCH en el directorio del kernel
	getcwd(pathDirectorioActual, sizeof(pathDirectorioActual));

	pool = list_create();			// POOL DE MEMORIAS
	tablas = list_create();			// ESTRUCTURA QUE CONTIENE TODAS LAS TABLAS (METADATA)

//	socketsPool = list_create();	// SOCKETS DE TODO EL POOL

	// Tiempos de requests
	tTotal = malloc(sizeof(int));

	// Todoo para el describe
	obtenerMemoriaDescribe();

	gossiping();

	iniciarCriterios();				/// INICIALIZO LISTAS DE CRITERIOS ///
	add(MemDescribe,&sc);
}

void terminar_programa()
{
	log_info(g_logger,"Finalizando programa..");

	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Libero hilos
	pthread_cancel(plp);
	pthread_cancel(describeGlobal);
	pthread_cancel(metricas);
	pthread_cancel(monitoreador);

	for(int i=0; i<nivelMultiprocesamiento; i++)
	{
		pthread_cancel(executer[i]);
	}

	//Libero los estados y elimino sus elementos
	finalizarEstados();

	//Libero las memorias de los criterios
	liberarCriterios();

	//Libero el pool de memorias
	liberarMemorias();

	//Libero el diccionario de mutexs
	liberarMutexs();

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

void leerConsola()
{
	printf("\nBienvenido! Welcome! Youkoso!\n");
	while(1)
	{
		resultadoParser* res = malloc(sizeof(resultadoParser));

		char* linea = readline(">");					// Leo stdin
		if(linea)
			add_history(linea);							// Para recordar el comando

		resultadoParser aux = parseConsole(linea);
		memcpy(res,&aux,sizeof(resultadoParser));

		if(res->accionEjecutar==SALIR_CONSOLA){
			finalizar.resultado = SALIR;
			break;
		}

		pthread_mutex_lock(&mNew);
		agregarScriptAEstado(res, NEW);
		pthread_mutex_unlock(&mNew);

		sem_post(&sNuevo);	// Habilito el estado NEW

		//free(linea); HAY QUE VOLVER A PONERLO
	}

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

		sem_post(&sListo);

		if(r->accionEjecutar==SALIR_CONSOLA)
			break;

	}
}

void ejecutador(){ // ACTUA COMO ESTADO EXEC
	resultado e;
	while(1){

		sem_wait(&sListo);

		pthread_mutex_lock(&mReady);
		Script *s = queue_pop(ready);
		pthread_mutex_unlock(&mReady);

		if(deboSalir(s)) // ACA PUEDE ESTAR ROMPIENDO
			return;

		for(int i=0; i < quantum ;i++) //ver caso en que falla, ejecutarS podria retornar un estado
		{
			e = ejecutarScript(s);

			//Logueo el resultado
			if (e.resultado == OK){
				log_info(g_logger,"%s", e.mensaje);
			}
			else if(e.resultado == ERROR){
				log_error(g_logger, "Error en request n°: %d", s->pc);
				mandarAexit(s);
				break;
			}

			if (terminoScript(s)) {
				log_info(g_logger, "Termino script");
				mandarAexit(s);
				break;
			}
		}
		if(e.resultado == OK && !terminoScript(s)){
			log_info(g_logger,"Fin de quantum, vuelvo a ready");
			mandarAready(s);
		}

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
		sem_wait(&sDescribe);
		printf("La metadata refresh es: %d\n", metadataRefresh);
		describe(NULL);
		sleep(metadataRefresh/1000); // Lo paso a ms
		sem_post(&sDescribe);
	}
}

void realizarGossipingAutomatico(){
	while(1){
		sleep(retardoGossiping/1000);
		gossiping();
	}
}

resultado describe(char* nombreTabla)
{
	t_list* tablaLFS = list_create();
	int size;
	int statusRespuesta;
	int valueResponse;
	resultado res;
	accion acc;

	resultadoParser* describe = malloc(sizeof(resultadoParser));
	describe->accionEjecutar = DESCRIBE;
	contenidoDescribe* cd = malloc(sizeof(contenidoDescribe));
	cd->nombreTabla = nombreTabla;
	describe->contenido = cd;

	Memoria* mem = list_get(pool,0);

	char* msg = serializarPaquete(describe,&size);

	bloquearConexion(mem);

	send(mem->socket, msg, size, 0);
	// Pido el describe a la memoria
	char* buffer = malloc(sizeof(int));
	valueResponse = recv(mem->socket,buffer,sizeof(int),0);

	memcpy(&acc,buffer,sizeof(int));								// Me fijo que accion para saber como deserializar

	if(valueResponse < 0)
	{
		log_error(g_logger,strerror(errno));
		desbloquearConexion(mem);
		res.resultado = ERROR;
	}
	else if(valueResponse == 0)
	{
		log_error(g_logger,"Posiblemente la memoria se desconectó.");
		desbloquearConexion(mem);
		res.resultado = ERROR;
	}
	else
	{
		res.accionEjecutar=acc;
		statusRespuesta = recibirYDeserializarRespuesta(mem->socket,&res); // Recibo la lista de tablas

		desbloquearConexion(mem);

		if(statusRespuesta<0 || res.resultado == ERROR)
			{
				log_error(g_logger,"Describe fallido");
			}
			else
			{
				tablaLFS = (t_list*)res.contenido;

				if(list_size(tablaLFS)>0){
					list_clean(tablas);						// Para no agregar repetidas
					list_add_all(tablas,tablaLFS);
					log_info(g_logger,"Describe global realizado con éxito");
					log_info(g_logger,"Cantidad de tablas indexadas: %d", tablas->elements_count);

					for(int i = 0; i<tablas->elements_count; i++)
					{
						log_info(g_logger,"Tablas indexada n°:%d -> %s", i ,((metadataTabla*)list_get(tablas,i))->nombreTabla);
					}
				}
				else
					log_info(g_logger,"No hay tablas");

				res.resultado = OK;
			}
	}
	free(buffer);
	free(describe);
	free(msg);
	free(cd);
	list_destroy(tablaLFS);

	return res;
}

void gestionarConexionAMemoria(Memoria* mem)
{

	struct addrinfo hints;
	struct addrinfo* serverInfo;

	memset(&hints, 0, sizeof(hints)); // Relleno con 0 toda la estructura de hints.
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(mem->ipMemoria,mem->puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	mem->socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	agregarMutex(mem);

	bloquearConexion(mem);

	int res =connect(mem->socket, serverInfo->ai_addr, serverInfo->ai_addrlen); // Me conecto al socket

	if(res == -1)
	{
		log_error(g_logger, "Memoria inaccesible: %s", strerror(errno));
	}
	else
	{
		log_info(g_logger, "Conectado con IP: %s:%s",mem->ipMemoria, mem->puerto);

		// Envio un 1
		uint32_t codigo = 1;
		send(mem->socket,&codigo,sizeof(uint32_t),0);
		int status = 0;
		status = recv(mem->socket,&(mem->id),sizeof(mem->id),0);


		if(status != sizeof(uint32_t))
			log_info(g_logger, "Error al recibir id de memoria");
		log_info(g_logger, "ID Memoria: %i", mem->id);

	}
	freeaddrinfo(serverInfo); // Libero
	desbloquearConexion(mem);
}

////////////////////////////////////////////////////

void controlConfig()
{
	int length, i = 0;
	    int fd;
	    int wd;
	    char buffer[BUF_LEN];

	    fd = inotify_init();

	    if (fd < 0) {
	        perror("Error en el inicio del Inotify: ");
	    }

	    wd = inotify_add_watch(fd, pathDirectorioActual, IN_MODIFY);

	    while(finalizar.resultado != SALIR){
	    	i=0;
	        length = read(fd, buffer, BUF_LEN);
	        if (length < 0) {
	            perror("Error en la lectura del archivo de config: ");
	        }
	        if(length == 0){
	        	printf("Sin cambios en el archivo de configuración.\n");
	        }

	        while (i < length) {
	            struct inotify_event *event =
	                (struct inotify_event *) &buffer[i];
	            if (event->len) {
	                if (event->mask & IN_MODIFY) {
	                	if(strcmp("Kernel.config",event->name)==0){
	                		log_info(g_logger,"El archivo %s fue modificado.", event->name);
	                		actualizarRetardos();
	                	}
	                }
	            }
	            i += EVENT_SIZE + event->len;
	        }
	    }
	    (void) inotify_rm_watch(fd, wd);
	    (void) close(fd);
}

void actualizarRetardos()
{
	// Un problema de sincronización

	config_destroy(g_config);
	g_config = config_create("Kernel.config");

	// Obtengo los nuevos valores
	quantum = config_get_int_value(g_config,"QUANTUM");
	metadataRefresh = config_get_int_value(g_config,"METADATA_REFRESH");
	sleepEjecucion = config_get_int_value(g_config,"SLEEP_EJECUCION");


}

////////////////////////////////////////////////////

void realizarMetrics()
{
	while(1)
	{
		sleep(30);
		metrics();
		limpiarEstadisticas();
	}
}

resultado metrics(){
	resultado res;

	mostrarMetrics(&sc);
	mostrarMetrics(&ec);
	mostrarMetrics(&shc);
	list_iterate(pool,mostrarMemoryLoad);

	res.accionEjecutar=METRICS;
	res.contenido=NULL;
	res.mensaje="Métricas informadas.";
	res.resultado=OK;

	return res;
}

void mostrarMetrics(Criterio* crit)
{
	log_info(g_logger, "Métricas del criterio: %s", mostrarConsistencia(crit->tipo));
	int reads = sumarReads(crit->memorias);
	int writes = sumarWrites(crit->memorias);
	int readLatency = obtenerLatency(crit->reads);
	int writeLateny = obtenerLatency(crit->writes);

	log_info(g_logger, "Cantidad de INSERTS en los ultimos 30s del criterio %s: %d", mostrarConsistencia(crit->tipo), writes);
	log_info(g_logger, "Cantidad de SELECTS en los ultimos 30s del criterio %s: %d", mostrarConsistencia(crit->tipo), reads);
	log_info(g_logger, "Tiempo promedio de ejecucion de un INSERT del criterio %s: %d", mostrarConsistencia(crit->tipo), readLatency);
	log_info(g_logger, "Tiempo promedio de ejecucion de un SELECT del criterio %s: %d", mostrarConsistencia(crit->tipo), writeLateny);
}

void mostrarMemoryLoad(void* elem)
{
	int select, insert;
	Memoria* mem = (Memoria*)elem;
	if(mem->totalOperaciones > 0)
	{
		select = (mem->selectsTotales * 100) / mem->totalOperaciones;
		insert = (mem->insertsTotales * 100) / mem->totalOperaciones;
	}
	else
	{
		select = 0;
		insert = 0;
	}
	log_info(g_logger,"Memory Load de la memoria ID: %d", mem->id);
	log_info(g_logger,"El %d%% de las operaciones totales son SELECTS",select);
	log_info(g_logger,"El %d%% de las operaciones totales son INSERTS",insert);
}

int sumarReads(t_list* memorias)
{
	int total = 0;
	for(int i = 0; i<list_size(memorias); i++)
	{
		total += ((Memoria*)list_get(memorias,i))->selectsTotales;
	}
	return total;
}

int sumarWrites(t_list* memorias)
{
	int total = 0;
	for(int i = 0; i<list_size(memorias); i++)
	{
		total += ((Memoria*)list_get(memorias,i))->insertsTotales;
	}
	return total;
}

void limpiarEstadisticas()
{
	list_iterate(pool,setearEstadisticas);
}

void setearEstadisticas(void* elem)
{
	Memoria* mem = (Memoria*)elem;
	mem->insertsTotales = 0;
	mem->selectsTotales = 0;
	mem->totalOperaciones = 0;
}

int obtenerLatency(t_list* tiempos)
{

}

