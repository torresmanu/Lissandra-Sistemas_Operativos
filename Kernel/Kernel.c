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

int main(void) {

	iniciar_programa();
	//obtenerMemorias();
	//gestionarConexion();
	leerConsola();
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

Estado iniciarEstado(nombreEstado nom){
	Estado est;
	est.nombre = nom;
	est.cola = queue_create();
	return est;
}

void iniciarEstados(){ 			// Son 4 colas.
	iniciarEstado(NEW);
	iniciarEstado(READY);
	iniciarEstado(EXEC);
	iniciarEstado(EXIT);
}

void liberarEstado(void* elem){
	// Falta inicializar una estructura que contenga el proceso
	// y hacerle un free. Pero me falta pensar que estructura lo contiene.
	// free(t_proceso*); por asi decirlo
	return;
}

void finalizarEstados(){
	queue_clean_and_destroy_elements(NEW,liberarEstado);
	queue_clean_and_destroy_elements(READY,liberarEstado);
	queue_clean_and_destroy_elements(EXEC,liberarEstado);
	queue_clean_and_destroy_elements(EXIT,liberarEstado);
}

void agregarScriptAEstado(resultadoParser res, t_queue* estado)  // Aca hace las comprobaciones si pueden
{
	if(estado == NEW || READY){
		//queue_push(estado,res);
	}
	else if(estado == EXEC){
		if(nivelActual < nivelMultiprocesamiento)
		{
			//queue_push(estado,res);
			nivelActual++;
		}
		else
		{
			// Que hacer si ya esta ejecutando 3 procesos a la vez?
		}
	}
}

void moverScriptDeEstado(){}
void finalizarScript(){} // Debe hacer un free y sacarlo de la cola

//////////////////////////////////////////////////////////
// Leer LQL
// Por archivo

resultadoParser leerScriptLQL(FILE* fd){
	char linea[MAX_BUFFER];
	resultadoParser r;
	fgets(linea,sizeof(linea),fd);
	r = parseConsole(linea);
	return r;
}

void run(char* path){
	FILE* arch = fopen(path, "r");
	resultadoParser res;
	while(!feof(arch))
	{
		res = leerScriptLQL(arch);
		//agregarScriptAEstado(res,NEW);        POR AHORA NO LA USO
		printf("Accion: %d\n",res.accionEjecutar);			//Solo sirve para mostrar que parsea

	}
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
	char* linea;
	char* accion;
	char* path;
	char* contenido;
	char* consola;
	resultadoParser r;

	printf("\nBienvenido! Welcome! Youkoso!\n");
	while(1)
	{
		linea = readline(">");
		add_history(linea);
		consola = strdup(linea);

		accion = strsep(&linea," ");
		printf("Request: %s\n", accion);
		if(strcmp(accion,"RUN") == 0)
		{
			path = strsep(&linea,"\n");
			printf("Path: %s\n", path);
			run(path);
		}
		else
		{
			contenido = strsep(&linea,"\n");
			printf("Contenido: %s\n",contenido);
			r = leerLineaSQL(consola);
			agregarScriptAEstado(r,NEW); 			// Pendiente de revision
		}
		free(linea);
		free(consola);
	}
}

//////////////////////////////////////////////////////////
// Criterios y memorias

void obtenerMemorias(){
	t_list *t = list_create();
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
