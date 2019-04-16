/*
 ============================================================================
 Name        : LFS.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "LFS.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <netdb.h>


#define BACKLOG 5
#define PACKAGESIZE 1024
#define PUERTO_SALIENTE "8081"
int main(void) {
	resultado res;
	char* mensaje;
	res.resultado= OK;
	iniciar_programa();
	gestionarConexion();

	while(res.resultado != SALIR)
	{
		mensaje = readline(">");
		res = parsear_mensaje(mensaje);
		if(res.resultado == OK)
		{
			log_info(g_logger,res.mensaje);
		}
		else if(res.resultado == ERROR)
		{
			log_info(g_logger,"Ocurrio un error al ejecutar la acción");
		}
		else if(res.resultado == MENSAJE_MAL_FORMATEADO)
		{
			log_info(g_logger,"Mensaje incorrecto");
		}
	}
	terminar_programa();

}

void iniciar_programa(void)
{
	//Inicio el logger
	g_logger = log_create("LFS.log", "LFS", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion LFS");

	//Inicio las configs
	g_config = config_create("LFS.config");
	log_info(g_logger,"Configuraciones inicializadas");

}

resultado parsear_mensaje(char* mensaje)
{
	//Logeo el mensaje que llego
	log_info(g_logger,mensaje);
	resultado res;
	char * accion = strsep(&mensaje," ");
	if(strcmp(accion,"SELECT") == 0){
		char * tabla = strsep(&mensaje," ");
		char * str_key = strsep(&mensaje," ");
		res = select_acc(tabla,atoi(str_key));
	}
	else if(strcmp(accion,"INSERT") == 0)
	{
		char * tabla = strsep(&mensaje," ");
		char * str_key = strsep(&mensaje," ");
		char * value = strsep(&mensaje," ");
		char * timestamp = strsep(&mensaje," ");
		res = insert(tabla,atoi(str_key),value,atol(timestamp));
	}
	else if(strcmp(accion,"CREATE") == 0)
	{
		char * tabla = strsep(&mensaje," ");
		char * consistencia = strsep(&mensaje," ");
		char * cant_part = strsep(&mensaje," ");
		char * tiempo_compr = strsep(&mensaje," ");
		res = create(tabla,consistencia,atoi(cant_part),atoi(tiempo_compr));
	}
	else if(strcmp(accion,"DESCRIBE") == 0)
	{
		char * tabla = strsep(&mensaje," ");
		res = describe(tabla);
	}
	else if(strcmp(accion,"DROP") == 0)
	{
		char * tabla = strsep(&mensaje," ");
		res = drop(tabla);
	}
	else if(strcmp(accion,"JOURNAL") == 0)
	{
		res = journal();
	}
	else if(strcmp(accion,"SALIR") == 0)
	{
		res.resultado = SALIR;
		res.mensaje = "";
	}
	else
	{
		res.resultado = MENSAJE_MAL_FORMATEADO;
		res.mensaje = "";
	}
	return res;
}

resultado select_acc(char* tabla,int key)
{
	resultado res;
	res.mensaje="Mensaje prueba";
	res.resultado=OK;
	return res;
}

resultado insert(char* tabla,int key,char* value,long timestamp)
{
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado create(char* tabla,char* t_cons,int cant_part,int tiempo_comp)
{
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado describe(char* tabla)
{
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado drop(char* tabla)
{
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado journal()
{
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

}

void gestionarConexion(){
	int estado=1;
	int server_fd = iniciarServidor();
	int cliente_fd = esperarCliente(server_fd);
	while(estado != 0){
		estado=recibir_mensaje(cliente_fd);
	}
    close(cliente_fd);
    close(server_fd);
}

int iniciarServidor(){
	int socket_servidor;

    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PUERTO_SALIENTE, &hints, &serverInfo);

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

int recibir_mensaje(int socketCliente)
{

	char *buffer;
	buffer = malloc(100);
	recv(socketCliente, buffer, 100, MSG_WAITALL);

	printf( "Me llego el mensaje: %s\n", buffer);
	if(strcmp(buffer,"exit")==0){
		return 0;
	}
	return 1;
}

