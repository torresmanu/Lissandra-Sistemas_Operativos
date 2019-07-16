/*
 * Gossiping.h
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */

#ifndef GOSSIPING_H_
#define GOSSIPING_H_

#include "Globales.h"
#include <sys/time.h>


void gossiping();
void gossipear(void* elem);
bool estaConectada(Memoria* mem);
bool conectarMemoria(Memoria* mem);
char* serializarTabla(uint32_t *totalSize);
void* sumarTamanios(void* seed,void* elem);
int mandarYrecibir(Memoria* mem);
int recibirYmandar(int socket);
int mandarTabla(int socket);
int recibirTablas(int socket);
bool agregarMemoria(Memoria* mem);
void cerrarConexion(Memoria* mem);
void destroy_nodo_memoria(void * elem);
void destroy_nodo_memoria_conocida(void* elem);
void sacarMemoria(Memoria* mem);
bool estaEnElPool(Memoria* memVieja, t_list* memoriasRecibidas);
t_list* obtenerMemoriasGossiping();
bool noSoyYo(void* elem);
void agregarSeeds(t_list* memoriasGossiping);
bool coincideIPyPuerto(Memoria* mem1,Memoria* mem2);
void ponerTimestampActual(Memoria* mem);

#endif /* GOSSIPING_H_ */

