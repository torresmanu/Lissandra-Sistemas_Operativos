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

}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

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
	EXIT = queue_create();
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

void agregarScriptAEstado(){}
void moverScriptDeEstado(){}
void finalizarScript(){} // Debe hacer un free y sacarlo de la cola

// Leer LQL

void leerArchivoLQL(){		// Itera ejecutando leer ScriptLQL

}

void leerScriptLQL(){}

void RUN(FILE* path){
	FILE* lql = fopen(path,"r+b");
	// t_proceso* proc;
	while(!feof(lql)){
		//
	}
}

/* Necesito parsear_mensaje() para ir haciendolo con cada script. Asi no uso otra funcion igual */

