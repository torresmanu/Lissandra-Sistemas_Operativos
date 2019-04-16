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
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>


#define PUERTO "6667"



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
	int socketCliente = iniciarCliente();
	while(i != 0){
		i=enviar_mensaje(socketCliente);
	}

}


int iniciarCliente()
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PUERTO, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

int enviar_mensaje(int socket_cliente)
{
	char *mensaje;

	mensaje = readline("Escriba su mensaje (exit para salir) >");


	send(socket_cliente, mensaje, strlen(mensaje)+1, 0);
	if(strcmp(mensaje,"exit")==0){
		free(mensaje);
		return 0;
	}
	free(mensaje);
	return 1;
}


