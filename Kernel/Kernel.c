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
	    pthread_create( &executer[i], NULL, (void*)ejecutador, NULL);
	}

	pthread_t plp;
	pthread_create(&plp,NULL,(void*) planificadorLargoPlazo,NULL);

	//obtenerMemorias();
	//gestionarConexion();
	leerConsola();										/// ACA COMIENZA A ITERAR Y LEER DE STDIN /////

	pthread_join(plp,NULL);

	for(int i=0; i<nivelMultiprocesamiento; i++ )
		pthread_join(executer[i], NULL);

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
	tablas = list_create();

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

/*
void gestionarConexion()
{
	int i=1;
	PUERTO = config_get_string_value(g_config, "PUERTO_MEMORIA");
	int socketServer = conectarseAlServidor(PUERTO,"Me conecte a la memoria");	//Conecto con las memorias
	while(i){
		i=enviar_mensaje(socketServer);	//Con "exit" i=0 y salgo
	}
}
*/

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
			queue_push(new, script);
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
				// pthread_join?
			}
			break;
		case EXIT:
			queue_push(exi,script);
		}

}


void moverScriptDeEstado(){}
void finalizarScript(){} // Debe hacer un free y sacarlo de la cola

///////////////////////////////////////////////
///////////////// Leer LQL ////////////////////
///////////////// Por archivo /////////////////

Script* run(char* path){
	printf("Entro en run\n");

	FILE* arch = fopen(path, "r+b");
	printf("Abro el archivo\n");

	Script* script = parsearScript(arch);

	fclose(arch);
	return script;
}

Script* parsearScript(FILE* fd){
	Script* script = malloc(sizeof(Script));
	//script->instrucciones = (resultadoParser*) list_create();
	script->instrucciones = list_create(); // No se castea el resultadoParser?
	script->pc=0; //Se modifica en ejecución

	while(!feof(fd)){
		resultadoParser *req = malloc(sizeof(resultadoParser));
		resultadoParser aux = leerRequest(fd);
		memcpy(req,&aux,sizeof(aux));
		list_add(script->instrucciones,req);

	}
	printf("Script prepara3\n");
	return script;
}

resultadoParser leerRequest(FILE* fd){
	char linea[MAX_BUFFER];
	resultadoParser r;
	fgets(linea,sizeof(linea),fd); // fgets lee hasta el salto de linea
	r = parseConsole(linea);
	return r;
}

///////////////// Linea por consola /////////////////
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
		Script* s = malloc(sizeof(Script));

		char* linea = readline(">");					// Leo stdin
		add_history(linea);								// Para recordar el comando

		resultadoParser res = parseConsole(linea);
		memcpy(aux,&res,sizeof(res));

		pthread_mutex_lock(&mNew);
		s = crearScript(aux);
		agregarScriptAEstado(s, NEW);
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

Script* crearScript(resultadoParser* r){
	Script* s = malloc(sizeof(Script));
	printf("Entro a crearScript\n");

	if(r->accionEjecutar==RUN){
		char* path;
		path = ((contenidoRun*) r->contenido)->path;
		printf("Aqui la ruta: %s\n", path);
		s = run(path);
	}else{
		s->instrucciones = list_create();
		s->pc = 0;
		list_add(s->instrucciones,r);
	}
	return s;
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

		for(int i=0; i <= quantum ;i++){//ver caso en que falla, ejecutarS podria retornar un estado
			if(!terminoScript(s)){
				e = ejecutarScript(s);
				if(e == ERROR)
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

status ejecutarScript(Script *s){
	resultadoParser *r = list_get(s->instrucciones,s->pc);
	status estado = ejecutarRequest(r);

	(s->pc)++;
	return estado;
}

status ejecutarRequest(resultadoParser *r){
	if(usaTabla(r)){
		Tabla* tabla = obtenerTabla(r);
		if(tabla != NULL)
			return ejecutar(tabla->criterio,r);
		else
			return ERROR; 											// HACER UN ENUM
	}
	else{
		switch (r->accionEjecutar){
			case JOURNAL:
				//journal();
				break;
			case METRICS:
				//metrics();
				break;
			case ADD:
			{
				Memoria* mem = malloc(sizeof(Memoria));
				mem->idMemoria = ((contenidoAdd *)(r->contenido))->numMem;
				Criterio cons = toConsistencia(((contenidoAdd *)(r->contenido))->criterio);
				add(mem,cons);
				break;
			}
			default:
				break;
		}
	return OK;
	}
}

status ejecutar(Criterio* criterio, resultadoParser* request){
	Memoria* mem = masApropiada(criterio);
	status resultado = enviarRequest(mem, request); 		// Seguramente se cambie status por una estructura Resultado dependiendo lo que devuelva
	return resultado;										// la memoria. enviarRequest está sin implementar, usa sockets.
}

status enviarRequest(Memoria* mem, resultadoParser* request)
{
	status resultado;
	//Aca se realiza toda la logica de enviar paquete por sockets (serializando?)
	return resultado;
}

Memoria* masApropiada(Criterio* c){
	Memoria* mem;
	switch(c->tipo)
	{
		case SC:
			mem = (Memoria*)list_get(sc.memorias,0);
			break;
		case SHC:											// Necesito aplicar Hash
			break;
		case EC:
		{
			int cantidadMemorias = rand()%list_size(ec.memorias)+1;	// Que sea aleatoria
			mem = (Memoria*)list_get(ec.memorias,cantidadMemorias);
			break;
		}
		default:
			break;
	}
	return mem;
}

Criterio toConsistencia(char* cadena)
{
	if(strcmp(cadena, "SC") == 0)
		return sc;
	else if(strcmp(cadena, "SHC") == 0)
		return shc;
	else
		return ec;
}

bool usaTabla(resultadoParser* r){
	return r->accionEjecutar == SELECT || r->accionEjecutar == INSERT || r->accionEjecutar == DROP || r->accionEjecutar == DESCRIBE || r->accionEjecutar == CREATE;
}
Tabla* obtenerTabla(resultadoParser* r){
	switch(r->accionEjecutar)
	{
		case SELECT:
		{
			contenidoSelect* c = (contenidoSelect*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case INSERT:
		{
			contenidoInsert* c = (contenidoInsert*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case DROP:
		{
			contenidoDrop* c = (contenidoDrop*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case CREATE:
		{
			contenidoCreate* c = (contenidoCreate*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case DESCRIBE:
		{
			contenidoDescribe* c = (contenidoDescribe*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		default:
			return NULL;
	}
}

Tabla* buscarTabla(char* nom)
{
	bool coincideNombre(void* element)					//Subfunción de busqueda
	{
		return strcmp(nom,((Tabla*)element)->nombre);
	}

	return (Tabla*)list_find(tablas,coincideNombre);
}


//////////////////////////////////////////////////////////
// Criterios y memorias

void obtenerMemorias(){
	Memoria *mem;
	int id = config_get_int_value(g_config,"MEMORIA");
	mem->idMemoria = id;
	gossiping(mem);//meto en pool la lista de memorias encontradas

}

// HARDCODEADO SOLO COMO PARA EJEMPLO.	////////////////////
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
////////////////////////////////////////////////////

//Agrego la memoria en la lista de memorias del criterio
void add(Memoria *memoria,Criterio cons){

	list_add(cons.memorias,memoria);
}


