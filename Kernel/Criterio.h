/*
 * Criterio.h
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#ifndef CRITERIO_H_
#define CRITERIO_H_

#include <commons/config.h>
#include <commons/collections/list.h>
#include "PoolMem.h"


typedef enum{SC, SHC, EC} t_consist;

typedef struct{
	t_consist tipo;
	t_list *memorias;
}Criterio;

Criterio sc,shc,ec;

Memoria criterioSC();
void iniciarCriterios();
void liberarCriterios();
void destroy_nodo_memoria(void * elem);

#endif /* CRITERIO_H_ */
