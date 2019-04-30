/*
 * sockets.h
 *
 *  Created on: 17 abr. 2019
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#define BACKLOG 5
#define PACKAGESIZE 100

#define PUERTO_ENTRANTE "4000"  // SOLO SE USA PARA PRUEBAS. LOS PUERTOS SE SACAN DE LOS .CONFIG
#define PUERTO_SALIENTE "5003"

void gestionarConexion();

int iniciarServidor(char* puerto);

int esperarCliente(int listenningSocket,char* mensaje);

int conectarseAlServidor(char* puerto,char* mensaje);

int enviar_mensaje(int socket_cliente);

void limpiar (char *cadena);



#endif /* SOCKETS_H_ */
