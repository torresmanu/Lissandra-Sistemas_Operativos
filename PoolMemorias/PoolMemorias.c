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
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#define PUERTO_ENTRANTE "6667"
#define BACKLOG 5
#define PACKAGESIZE 1024
#define PUERTO_SALIENTE "8081"

int main(void) {

	pthread_t hiloKernelLFS;
	pthread_create(&hiloKernelLFS,NULL,(void*) gestionarConexionEntrante(),NULL);
	iniciar_programa();
	gestionarConexionEntrante();
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

void gestionarConexionEntrante(){
	int estado=1;
	char *buffer;
	int server_fd = iniciarServidor();
	int cliente_fd = esperarCliente(server_fd);
	int socketCliente = gestionarConexionSaliente();
	while(estado != 0){
		recibir_mensaje(cliente_fd);
		send(socketCliente, buffer, 100, 0);
		if(strcmp(buffer,"exit")==0){
				free(buffer);
				estado = 0;
			}
	}
    close(cliente_fd);
    close(server_fd);
    close(socketCliente);
}

int iniciarServidor(){
	int socket_servidor;

    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PUERTO_ENTRANTE, &hints, &serverInfo);

    int listenningSocket;
    listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

    if(bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
		printf("error");

    freeaddrinfo(serverInfo);
    listen(listenningSocket, BACKLOG);

    return listenningSocket;
}

int esperarCliente(int listenningSocket){
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

    int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
    printf("Se conecto un cliente!\n");

	return socketCliente;
}

void* recibir_mensaje(int socketCliente)
{
	char *buffer;
	buffer = malloc(100);
	recv(socketCliente, (void*) buffer, 100, MSG_WAITALL);
	printf( "Me llego el mensaje: %s\n", buffer);
	return buffer;
}

int gestionarConexionSaliente()
{
	int socketCliente = iniciarCliente();
	return socketCliente;
}


int iniciarCliente()
{
	struct addrinfo hints;
	struct addrinfo *server_infoc;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PUERTO_SALIENTE, &hints, &server_infoc);
	int socketCliente = socket(server_infoc->ai_family, server_infoc->ai_socktype, server_infoc->ai_protocol);

	if(connect(socketCliente, server_infoc->ai_addr, server_infoc->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_infoc);

	return socketCliente;
}

int enviar_mensaje(int socket_cliente, char *buffer)
{

	//mensaje = readline("Escriba su mensaje (exit para salir) >");


	send(socket_cliente, buffer, 100, 0);
	if(strcmp(buffer,"exit")==0){
		free(buffer);
		return 0;
	}
	free(buffer);
	return 1;
}
//void* recibir_buffer(int* size, int socketCliente)
//{
//	void * buffer;
//	buffer = malloc(100);
//	recv(socketCliente, buffer, *size, MSG_WAITALL);
//
//	return buffer;
//}
