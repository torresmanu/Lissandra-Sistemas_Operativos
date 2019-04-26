/*
 * sockets.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#include "sockets.h"
#include "log.h"
#include "config.h"

int conectarseAlServidor(char* puerto,char* mensaje)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");
	else
		printf("%s\n",mensaje);

	freeaddrinfo(server_info);

	return socket_cliente;
}

int enviar_mensaje(int socketServer)
{
	char buffer[PACKAGESIZE];
	printf("\nEscriba su mensaje (exit para salir) >");
	fgets(buffer, PACKAGESIZE, stdin);
	if(strcmp(buffer,"exit")==0){
			return 0;
		}
	else{
		printf("Tamaño: %d\n",strlen(buffer));
	}
	send(socketServer, buffer, strlen(buffer)+1, 0);
	limpiar(buffer);
	return 1;
}

void limpiar (char *cadena)		//Reemplazo el '\n' con un '\0'
{
  char *p;
  p = strchr (cadena, '\n');
  if (p)
    *p = '\0';
}


int iniciarServidor(char* puerto)
{
	//int socket_servidor;

    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &serverInfo);

    int listenningSocket;
    listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

    int yes=1;
    if (setsockopt(listenningSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
    	perror("setsockopt");
      	exit(1);
    }

    if(bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
    	perror("Error en bind");


    freeaddrinfo(serverInfo);
    listen(listenningSocket, BACKLOG);

    return listenningSocket;
}

int esperarCliente(int listenningSocket,char* mensaje)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("%s\n",mensaje);

	return socketCliente;
}

void recibir_mensaje(int cliente_fd,char* buffer,char* mensaje)
{
	recv(cliente_fd, (void*) buffer, PACKAGESIZE, 0);
	printf( "\n%s: %s\n",mensaje, buffer);
	printf( "Tamaño: %d\n", strlen(buffer));
}


