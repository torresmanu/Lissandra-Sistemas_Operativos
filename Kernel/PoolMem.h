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
#include <unistd.h>
#include <pthread.h>
#include "Globales.h"
#include <sys/time.h>




// MANEJO DE MEMORIAS
void establecerConexionPool();

int obtenerMemorias(int socket);
void gossiping();
Memoria obtenerMemoria(t_config* config);
Memoria *buscarMemoria(int numero);
void obtenerMemoriaDescribe();
bool tengoMemoria(Memoria* memNueva);
bool estoyConectado(int* conexion);

void agregarMutex(Memoria* mem);
void liberarMutexs();
void liberarMutex(void*);
pthread_mutex_t* obtenerMutex(Memoria* mem);
void bloquearConexion(Memoria* mem);
void desbloquearConexion(Memoria* mem);
int gestionarConexionAMemoria(Memoria *mem,char* id);
void destroy_nodo_memoria(void*);
void sacarMemoria(Memoria* mem);
void conectarMemorias(void* id);
void cerrarConexion(char* key, void* value);
void conectarEjecutadores();
void conectarGossiping();
bool sigueConectada(Memoria* memVieja, t_list* memoriasRecibidas);

bool agregarMemoria(Memoria* mem);
bool coincideIPyPuerto(Memoria* mem1,Memoria* mem2);
void inicializarMemoria(Memoria* memNueva);
void inicializarConexiones(Memoria* mem);
void ponerTimestampActual(Memoria* mem);

#endif /* POOLMEM_H_ */
