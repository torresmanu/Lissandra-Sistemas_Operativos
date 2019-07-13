/*
 * PoolMem.h
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#ifndef POOLMEM_H_
#define POOLMEM_H_

#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/parser.h>
#include <commons/serializacion.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>

// Semaforos
t_dictionary* mutexsConexiones;

//las puse aca para que no me rompa, buscar un lugar mejor
t_log* g_logger;
t_config* g_config;

int memoriaSocket;			// Almaceno el socket de la memoria para usarlo posteriormente
t_list* socketsPool;

typedef struct{
	char* ipMemoria;
	char* puerto;
	uint32_t id;
	int socket;
}Memoria;

// Pool de Memorias
t_list *pool;
Memoria* MemDescribe;

// MANEJO DE MEMORIAS
void establecerConexionPool();

int obtenerMemorias(int socket);
void gossiping();
Memoria obtenerMemoria(t_config* config);
Memoria *buscarMemoria(int numero);
void obtenerMemoriaDescribe();
bool tengoMemoria(Memoria* memNueva);
bool estoyConectado(Memoria* mem);

void agregarMutex(Memoria* mem);
void liberarMutexs();
void liberarMutex(void*);
pthread_mutex_t* obtenerMutex(Memoria* mem);
void bloquearConexion(Memoria* mem);
void desbloquearConexion(Memoria* mem);
void gestionarConexionAMemoria(Memoria *mem);

#endif /* POOLMEM_H_ */
