/*
 * Criterio.h
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#ifndef CRITERIO_H_
#define CRITERIO_H_

#include "PoolMem.h"

typedef enum{SC, SHC, EC} t_consist;

typedef struct{
	t_consist tipo;
	t_list *memorias;
}Criterio;

Criterio sc,shc,ec;

// MANEJO DE CRITERIOS
void iniciarCriterios();
void liberarCriterios();
void liberarMemorias();
void destroy_nodo_memoria(void*);
Criterio* toConsistencia(char*);
Memoria* masApropiada(Criterio*, resultadoParser*);
void add(Memoria*,Criterio*);

// MANEJO DE MEMORIAS
Memoria* buscarMemoriaPorID(int, t_list*);
bool coincideID(void*);

//RANDOM (EC)
int memoriaRandom();

//HASHEO (SHC)
int hash(resultadoParser*);
int obtenerHash(resultadoParser*);

#endif /* CRITERIO_H_ */
