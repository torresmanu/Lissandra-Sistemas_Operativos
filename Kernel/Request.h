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
#include <commons/serializacion.h>
#include "Estados.h"
#include "Criterio.h"

//Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

// Semaforos
pthread_mutex_t mConexion;

typedef struct{
	t_list *instrucciones;
	int pc;
}Script;

typedef struct{
	Criterio* criterio;
	char* nombre;
}Tabla;

t_list* tablas;

// EJECUTAR ARCHIVOS LQL //
Script* run(char*);

// MANEJO DE REQUESTS //
resultadoParser leerLineaSQL(char*);
resultadoParser leerRequest(FILE*);
Script* parsearScript(FILE*);
Script* crearScript(resultadoParser*);
bool terminoScript(Script *s);

// EJECUCION DE REQUESTS
resultado ejecutarRequest(resultadoParser*);
resultado ejecutarScript(Script*);
resultado ejecutar(Criterio*, resultadoParser*);
resultado enviarRequest(Memoria*,resultadoParser*);

metadataTabla* buscarTabla(char*);
metadataTabla* obtenerTabla(resultadoParser*);
bool usaTabla(resultadoParser*);

resultado recibir(int conexion);
resultado describe();
resultado journal();

#endif /* REQUEST_H_ */
