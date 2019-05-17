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
	gestionarConexion();
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
	iniciarEstados()();

	// Nivel de multiprocesamiento

	nivelMultiprocesamiento = config_get_int_value(g_config,"MULTIPROCESAMIENTO");
}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Libero los estados y elimino sus elementos
	finalizarEstados();
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
void iniciarEstados(){
	NEW = queue_create();
	READY = queue_create();
	EXEC = queue_create();
	NEW = queue_create();

}

void finalizarEstados(){
	queue_clean_and_destroy_elements(NEW,liberarEstado);
	queue_clean_and_destroy_elements(READY,liberarEstado);
	queue_clean_and_destroy_elements(EXEC,liberarEstado);
	queue_clean_and_destroy_elements(EXIT,liberarEstado);

}

void liberarEstado(void* elem){
	// Falta inicializar una estructura que contenga el proceso
	// y hacerle un free. Pero me falta pensar que estructura lo contiene.
	// free(t_proceso*); por asi decirlo
}

void agregarScriptAEstado(resultadoParser res, t_queue estado)  // Aca hace las comprobaciones si pueden
{
	if(estado == NEW || READY){
		queue_push(estado,res);
	}
	else if(estado == EXEC){
		if(nivelActual < nivelMultiprocesamiento)
		{
			queue_push(estado,res);
		}
		else
		{
			// Que hacer si ya esta ejecutando 3 procesos a la vez?
		}
	}
}
void moverScriptDeEstado(){}
void finalizarScript(){} // Debe hacer un free y sacarlo de la cola

//
// Leer LQL

resultadoParser leerScriptLQL(FILE* path){
	char* linea;
	resultadoParser res;
	fread(&linea,sizeof(char*),1,path);
	res = parseConsole(linea);
	return res;
}


void run(FILE* path){
	FILE* arch = fopen(path, "r+b");
	resultadoParser res;
	res = leerScriptLQL(arch);
	while(!feof(arch))
	{
		agregarScriptAEstado(res,NEW);
		res = leerScriptLQL(path);
	}
	fclose(path);
}


