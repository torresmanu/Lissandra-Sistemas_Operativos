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
char* serializarTabla(t_list* memoriasConocidas,int *totalSize);
void* sumarTamanios(void* seed,void* elem);
void intercambiarTablas(Memoria* mem);
int mandarTabla(Memoria* mem);
int recibirTablas(Memoria* mem);
void agregarMemoria(Memoria* mem);
void cerrarConexion(Memoria* mem);

#endif /* GOSSIPING_H_ */
