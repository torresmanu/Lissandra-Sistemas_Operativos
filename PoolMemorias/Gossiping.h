/*
 * Gossiping.h
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */

#ifndef GOSSIPING_H_
#define GOSSIPING_H_

#include "Globales.h"


void gossiping();
void gossipear(void* elem);
bool estaConectada(Memoria* mem);
bool conectarMemoria(Memoria* mem);
char* serializarTabla(t_list* memoriasConocidas,uint32_t *totalSize);
void* sumarTamanios(void* seed,void* elem);
int mandarYrecibir(Memoria* mem);
int recibirYmandar(int socket);
int mandarTabla(int socket);
int recibirTablas(int socket);
void agregarMemoria(Memoria* mem);
void cerrarConexion(Memoria* mem);

#endif /* GOSSIPING_H_ */

