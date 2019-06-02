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


	//Lanzamos tantos hilos como nivelMultiprocesamiento haya
	pthread_t *executer = malloc( nivelMultiprocesamiento * sizeof(pthread_t) );

	for(int i=0; i<nivelMultiprocesamiento; i++ )
	    pthread_create( &executer[i], NULL, ejecutador, NULL );

	pthread_t plp;
	pthread_create(&plp,NULL,(void*) planificadorLargoPlazo,NULL);

	iniciar_programa();
	//obtenerMemorias();
	//gestionarConexion();
	leerConsola();

	pthread_join(&plp,NULL);

	for(int i=0; i<nivelMultiprocesamiento; i++ )
		pthread_join( &executer[i], NULL);

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

	quantum = config_get_int_value(g_config,"QUANTUM");

	// Nivel de multiprocesamiento
	nivelMultiprocesamiento = config_get_int_value(g_config,"MULTIPROCESAMIENTO");

	pool = list_create();

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

void gestionarConexion()
{
	int i=1;
	PUERTO = config_get_string_value(g_config, "PUERTO_MEMORIA");
	int socketServer = conectarseAlServidor(PUERTO,"Me conecte a la memoria");	//Conecto con las memorias
	while(i){
		i=enviar_mensaje(socketServer);	//Con "exit" i=0 y salgo
	}
}

// Propias de Kernel
// Estados -> Colas

void iniciarEstado(Estado *est){
	est = queue_create();
}

void iniciarEstados(){ 			// Son 4 colas.
	iniciarEstado(new);
	iniciarEstado(ready);
	iniciarEstado(exec);
	iniciarEstado(exi);
}

void liberarRequest(void* elem){
	resultadoParser* nodo_elem = (resultadoParser *) elem;
	free(nodo_elem);
}

void liberarScript(void* elem){
	Script* nodo_elem = (Script *) elem;
	list_destroy_and_destroy_elements(nodo_elem->instrucciones,liberarRequest);
}

void finalizarEstados(){
	queue_clean_and_destroy_elements(new,liberarScript);
	queue_clean_and_destroy_elements(ready,liberarScript);
	queue_clean_and_destroy_elements(exec,liberarScript);
	queue_clean_and_destroy_elements(exi,liberarScript);
}

void agregarScriptAEstado(Script *script, nombreEstado estado)  // Aca hace las comprobaciones si pueden
{
	switch(estado){
		case NEW:
			queue_push(new,script);
			break;
		case READY:
			queue_push(ready,script);
			break;

		case EXEC:
			if(nivelActual < nivelMultiprocesamiento){
				//queue_push(estado,res);
				nivelActual++;
			}
			else{
				// Que hacer si ya esta ejecutando 3 procesos a la vez?
			}
			break;
		case EXIT:
			queue_push(exi,script);
		}

}


void moverScriptDeEstado(){}
void finalizarScript(){} // Debe hacer un free y sacarlo de la cola

//////////////////////////////////////////////////////////
// Leer LQL
// Por archivo

resultadoParser leerRequest(FILE* fd){
	char linea[MAX_BUFFER];
	resultadoParser r;
	fgets(linea,sizeof(linea),fd);
	r = parseConsole(linea);

	return r;
}

Script *parsearScript(FILE* arch){
	Script *script = (Script*) malloc(sizeof(Script));
	script->instrucciones = list_create();
	script->pc=0;

	while(!feof(arch)){
		resultadoParser *req = malloc(sizeof(resultadoParser));
		resultadoParser aux = leerRequest(arch);
		memcpy(req,&aux,sizeof(aux));

		list_add(script->instrucciones,req);
	}
	return script;
}

void run(char* path){
	FILE* arch = fopen(path, "r");

	Script *script = parsearScript(arch);

	queue_push(new,script);
	//agregarScriptAEstado(script,NEW); //no va a hacer falta mepa xq los cambios entre colas lo hacen solo los hilos

	fclose(arch);
}

// Linea por consola


resultadoParser leerLineaSQL(char* mensaje)
{
	resultadoParser r;
	r = parseConsole(mensaje);
	return r;
}

void leerConsola(){

	printf("\nBienvenido! Welcome! Youkoso!\n");
	while(1)
	{
		resultadoParser *aux = malloc(sizeof(resultadoParser));

		char* linea = readline(">");
		resultadoParser res = parseConsole(linea);

		memcpy(aux,&res,sizeof(res));

		pthread_mutex_lock(&mNew);
		queue_push(new,aux);
		pthread_mutex_unlock(&mNew);

		sem_post(&sNuevo);
		if(res.accionEjecutar==SALIR_CONSOLA)
			break;
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
		queue_push(ready,s);
		pthread_mutex_unlock(&mReady);

		sem_post(&sListo);

		if(r->accionEjecutar==SALIR_CONSOLA)
			break;

		free(r);
	}
	free(r);
}

Script* crearScript(resultadoParser *r){
	Script *s;

	if(r->accionEjecutar==RUN){
		s = parsearScript(r->contenido);
	}else{
		s = (Script*) malloc(sizeof(Script));
		s->instrucciones = list_create();
		s->pc = 0;

		list_add(s->instrucciones,r);
	}
	return s;
}

void ejecutador(){
	while(1){
		sem_wait(&sListo);

		pthread_mutex_lock(&mReady);
		Script *s = queue_pop(ready);
		pthread_mutex_unlock(&mReady);

		if(deboSalir(s))//hay que ver cuando termina, buscar una mejor forma
			return;

		for(int i=0; i <= quantum ;i++){//ver caso en que falla, ejecutarS podria retornar un estado
			if(!terminoScript(s)){
				ejecutarScript(s);
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

bool terminoScript(Script *s){
	return s->pc == list_size(s->instrucciones);
}

void mandarAready(Script *s){
	pthread_mutex_lock(&mReady);
	queue_push(ready,s);
	pthread_mutex_unlock(&mReady);

	sem_post(&sListo);
}

void mandarAexit(Script *s){
	pthread_mutex_lock(&mExit);
	queue_push(exi,s);
	pthread_mutex_unlock(&mExit);

}

void ejecutarScript(Script *s){
	resultadoParser *r = list_get(s->instrucciones,s->pc);
	ejecutarRequest(r);
	(s->pc)++;
}

void ejecutarRequest(resultadoParser *r){
	if(usaTabla(r)){
		char *tabla = obtenerTabla(r);
		Criterio c = buscarConsistencia(tabla);
		ejecutar(c,r);
	}
	switch (r->accionEjecutar){
		case JOURNAL:
			journal();
			break;
		case METRICS:
			metrics();
			break;
		case ADD:
			Memoria *mem = malloc(sizeof(Memoria));
			mem->idMemoria = ((contenidoAdd *)(r->contenido))->numMem;

			t_consist cons = toConsistencia(((contenidoAdd *)(r->contenido))->criterio);

			add(mem,cons);
			break;

	}

}

//////////////////////////////////////////////////////////
// Criterios y memorias

void obtenerMemorias(){
	Memoria *mem;
	int id = config_get_int_value(g_config,"MEMORIA");
	mem->idMemoria = id;

	gossiping(mem);//meto en pool la lista de memorias encontradas

}

void gossiping(Memoria *mem){
	Memoria *m1 = malloc(sizeof(Memoria));
	m1->idMemoria = 1;
	list_add(pool,m1);

	Memoria *m2 = malloc(sizeof(Memoria));
	m2->idMemoria = 2;
	list_add(pool,m2);

	Memoria *m3 = malloc(sizeof(Memoria));
	m3->idMemoria = 3;
	list_add(pool,m3);

}

//Agrego la memoria en la lista de memorias del criterio
void add(Memoria *memoria,t_consist tipo){

	switch(tipo){
		case SC:
			list_add(sc.memorias,memoria);
			break;

		case SHC:
			list_add(shc.memorias,memoria);
			break;

		case EC:
			list_add(shc.memorias,memoria);
			break;
	}
}


