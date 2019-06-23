/*
 * Request.h
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */

#ifndef REQUEST_H_
#define REQUEST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <commons/parser.h>
#include "Estados.h"
#include "Criterio.h"

//Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
int memoriaSocket;

typedef struct{
	t_list *instrucciones;
	int pc;
}Script;

typedef struct{
	Criterio* criterio;
	char* nombre;
}Tabla;

typedef enum {REQUEST_OK, REQUEST_ERROR} status;
t_list* tablas;

#define MAX_BUFFER 100

// EJECUTAR ARCHIVOS LQL //
Script* run(char*);

// MANEJO DE REQUESTS //
resultadoParser leerLineaSQL(char*);
resultadoParser leerRequest(FILE*);
Script* parsearScript(FILE*);
Script* crearScript(resultadoParser*);
bool terminoScript(Script *s);

// EJECUCION DE REQUESTS
status ejecutarRequest(resultadoParser*);
status ejecutarScript(Script*);
status ejecutar(Criterio*, resultadoParser*);
status enviarRequest(Memoria*,resultadoParser*);

Tabla* buscarTabla(char*);
Tabla* obtenerTabla(resultadoParser*);
bool usaTabla(resultadoParser*);


#endif /* REQUEST_H_ */
