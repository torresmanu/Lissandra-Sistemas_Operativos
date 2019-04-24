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
