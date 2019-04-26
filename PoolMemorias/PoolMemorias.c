/*
 ============================================================================
 Name        : PoolMemorias.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "PoolMemorias.h"

int main(void) {

	//pthread_t hiloKernelLFS;
	//pthread_create(&hiloKernelLFS,NULL,(void*) gestionarConexionEntrante(),NULL);
	iniciar_programa();
	gestionarConexion();
	terminar_programa();

}

void iniciar_programa(void)
{
	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "LFS", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Pool Memorias");

	//Inicio las configs
	g_config = config_create("PoolMemorias.config");
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
	int estado=1;
	char buffer[PACKAGESIZE];

	PUERTO_M = config_get_string_value(g_config,"PUERTO_FS");  // 8001
	PUERTO = config_get_string_value(g_config,"PUERTO");	   // 8000

	int socketMem_fd = iniciarServidor(PUERTO_M);										//Conecto el socket de las memorias al puerto "8001"
	int clienteKer_fd = esperarCliente(socketMem_fd,"Se conecto el Kernelcito!");
	int clienteMem = conectarseAlServidor("5003","Me conecte a Lissandra");             // Si harcodeo poniendo 5003 si funciona

	while(estado){

		recibir_mensaje(clienteKer_fd,buffer,"El kernel me mando el mensaje");

		send(clienteMem,buffer,strlen(buffer)+1,0);

		if(strcmp(buffer,"exit")==0)
			estado=0;
	}

    close(clienteKer_fd);
    close(socketMem_fd);
    close(clienteMem);
}
