/*
 * sockets.c
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#include "sockets.h"
#include "log.h"
#include "config.h"

/*int conectarseAlServidor(char* puerto,char* mensaje)
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
		printf("No me pude conectar al servidor");
	else
		printf("%s\n",mensaje);

	freeaddrinfo(server_info);

	return socket_cliente;
}*/

/*int enviar_mensaje(int socket_cliente)
{
	char buffer[PACKAGESIZE];


	printf("\nEscriba su mensaje (exit para salir) >");
	fgets(buffer, PACKAGESIZE, stdin);
	limpiar(buffer);

	send(socket_cliente, buffer, strlen(buffer)+1, 0);
	if(strcmp(buffer,"exit")==0){
		return 0;
	}

	printf("Tamaño: %d\n",strlen(buffer));
	return 1;
}

void limpiar (char *cadena)		//Reemplazo el '\n' con un '\0'
{
  char *p;
  p = strchr (cadena, '\n');
  if (p)
    *p = '\0';
}
*/

/*int iniciarServidor(char* puerto)
{
	//int socket_servidor;

    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto, &hints, &serverInfo);
    int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

    printf("puerto: %s\n", puerto);
    printf("listenningSocket: %i\n", listenningSocket);

    int yes=1;
    if (setsockopt(listenningSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {	//Esto es para reutilizar los puertos si fallan los procesos y no esperar
    	perror("setsockopt");
      	exit(1);
    }

    if(bind(listenningSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
    	perror("Error en bind");

    freeaddrinfo(serverInfo);

    int listenReturn = listen(listenningSocket, BACKLOG);

    return listenningSocket;
}

// ----BORRAR
int esperarCliente(int listenningSocket,char* mensaje)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("%s\n",mensaje);

	return socketCliente;
}

*/


