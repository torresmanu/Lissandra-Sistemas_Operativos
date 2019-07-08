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

// CRITERIO STRONG CONSISTENCY
Memoria criterioSC();

// MANEJO DE CRITERIOS
void iniciarCriterios();
void liberarCriterios();
void destroy_nodo_memoria(void * elem);
Criterio* toConsistencia(char*);
Memoria* masApropiada(Criterio*);
void add(Memoria*,Criterio*);


#endif /* CRITERIO_H_ */
