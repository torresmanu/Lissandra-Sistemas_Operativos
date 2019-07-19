/*
 * Request.h
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */

#ifndef REQUEST_H_
#define REQUEST_H_


#include <inttypes.h>
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



typedef struct{
	t_list *instrucciones;
	int pc;
}Script;

typedef struct{
	Criterio* criterio;
	char* nombre;
}Tabla;

t_list* tablas;

//uint64_t tTotal; // Tiempo de ejecucion de la request
//uint64_t tInicio;
//uint64_t tFinal;
sem_t sRequest;

// EJECUTAR ARCHIVOS LQL //
Script* run(char*);

// MANEJO DE REQUESTS //
resultadoParser leerLineaSQL(char*);
resultadoParser leerRequest(FILE*);
Script* parsearScript(FILE*);
Script* crearScript(resultadoParser*);
bool terminoScript(Script *s);

// EJECUCION DE REQUESTS
resultado ejecutarRequest(resultadoParser*,char* id);
resultado ejecutarScript(Script*,char* id);
resultado ejecutar(Criterio*, resultadoParser*,char* id);
resultado enviarRequest(Memoria*,resultadoParser*,char* id);

metadataTabla* buscarTabla(char*);
metadataTabla* obtenerTabla(resultadoParser*);
bool usaTabla(resultadoParser*);
void reemplazarMetadata(metadataTabla* tablaNueva);

resultado recibir(int conexion);
resultado describe();
resultado journal();
resultado metrics();
void enviarJournal(void*,char* id);

#endif /* REQUEST_H_ */
