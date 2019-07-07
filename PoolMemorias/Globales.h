/*
 * Globales.h
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include<commons/sockets.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/parser.h>
#include<commons/serializacion.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/inotify.h>
#include <stdbool.h>

typedef struct
{
	char* value;
	int key;
	long timestamp;
} Registro;



typedef struct
{
	int numero_pagina;
	int indice_registro;
	int flag_modificado;

} Pagina;

typedef struct
{
	int numero_segmento;
	char *nombre_tabla;//[NOMBRE_TABLA];
	t_list* puntero_tpaginas;

} Segmento;

typedef struct
{
	Pagina* pagina;
	Segmento* segmento;
}NodoTablaPaginas;

typedef struct{
	char* ip;
	char* puerto;
	int socket;
	uint32_t numero;
}Memoria;

t_log* g_logger;
t_config* g_config;

t_list* tabla_segmentos;
t_list* tabla_paginas_global;


int posLibres;

char* memoria;
char* pathConfig;
int* bitmap;
int cantidadFrames;
int serverSocket;
int tamValue;
int TAM_MEMORIA_PRINCIPAL;
int retardoJournaling;
int retardoGossiping;
int retardoMemoria;
int retardoLFS;
int offset;
bool ejecutando;
bool estaHaciendoJournal;
char* PUERTO_M;
char* PUERTO;
char* IP_M;

Memoria* yo;
t_list* memoriasSeeds;
t_list* memoriasConocidas;

#endif /* GLOBALES_H_ */
