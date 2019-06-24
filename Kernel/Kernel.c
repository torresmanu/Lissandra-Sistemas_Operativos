/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"

sem_t sNuevo;
sem_t sListo;

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
	pthread_t *executer = malloc( nivelMultiprocesamiento * sizeof(pthread_t) );

	for(int i=0; i<nivelMultiprocesamiento; i++ )
	{
	    pthread_create(&executer[i], NULL, (void*)ejecutador, NULL);
	}

	pthread_t plp;
	pthread_create(&plp,NULL,(void*)planificadorLargoPlazo,NULL);

	//obtenerMemorias();
	//gestionarConexionAMemoria();
	leerConsola();										/// ACA COMIENZA A ITERAR Y LEER DE STDIN /////

	pthread_join(plp,NULL);

	for(int i=0; i<nivelMultiprocesamiento; i++)
	{
		pthread_join(executer[i], NULL);
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

	//Inicializo los estados
	iniciarEstados();

	//Obtengo el quantum
	quantum = config_get_int_value(g_config,"QUANTUM");

	// Nivel de multiprocesamiento
	nivelMultiprocesamiento = config_get_int_value(g_config,"MULTIPROCESAMIENTO");

	pool = list_create();
	tablas = list_create();

	Tabla* colores = malloc(sizeof(Tabla));
	colores->criterio = &sc;
	char *auxc = "COLORES";
	colores->nombre = strdup(auxc);

	list_add(tablas,colores);

	iniciarCriterios();

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


void gestionarConexionAMemoria()
{
	struct addrinfo hints;
	struct addrinfo* serverInfo;

	memset(&hints, 0, sizeof(hints)); // Relleno con 0 toda la estructura de hints.
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(config_get_string_value(g_config, "IP_MEMORIA"), config_get_string_value(g_config, "PUERTO_MEMORIA"), &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	memoriaSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	int res = connect(memoriaSocket, serverInfo->ai_addr, serverInfo->ai_addrlen); // Me conecto al socket

	if(res == -1)
	{
		perror("No se pudo conectar: \n");
	}
	else
	{
		printf("Conectado a la memoria! Listo para enviar\n");
		log_info(g_logger, "Se conecto a la memoria, listo para enviar scripts.");
	}
	freeaddrinfo(serverInfo); // Libero
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

	printf("\nBienvenido! Welcome! Youkoso!\n");
	while(1)
	{
		resultadoParser *aux = malloc(sizeof(resultadoParser));
		//Script* s = malloc(sizeof(Script));

		char* linea = readline(">");					// Leo stdin
		if(linea)
			add_history(linea);							// Para recordar el comando

		resultadoParser res = parseConsole(linea);
		aux->accionEjecutar = res.accionEjecutar;
		aux->contenido = res.contenido;

		pthread_mutex_lock(&mNew);
		agregarScriptAEstado(&res, NEW);
		pthread_mutex_unlock(&mNew);

		sem_post(&sNuevo);
		printf("\nAgrego resParser con accion: %d a new\n",res.accionEjecutar);
		if(res.accionEjecutar==SALIR_CONSOLA)
			break;

		free(linea);
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

		printf("Paso el script a ready, cant rq: %d",s->instrucciones->elements_count);
		sem_post(&sListo);

		if(r->accionEjecutar==SALIR_CONSOLA)
			break;

		free(r);
	}
}

void ejecutador(){
	status e;
	while(1){
		sem_wait(&sListo);

		pthread_mutex_lock(&mReady);
		Script *s = queue_pop(ready);
		pthread_mutex_unlock(&mReady);

		if(deboSalir(s))//hay que ver cuando termina, buscar una mejor forma
			return;

		printf("Entro a ejecutar\n");
		for(int i=0; i <= quantum ;i++){ //ver caso en que falla, ejecutarS podria retornar un estado

			if(!terminoScript(s)){

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

//Agrego la memoria en la lista de memorias del criterio
void add(Memoria *memoria,Criterio cons)
{
	list_add(cons.memorias,memoria);
	log_info(g_logger,"Agrege memoria");

}


