/*
 * PoolMem.h
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#ifndef POOLMEM_H_
#define POOLMEM_H_

#include <commons/collections/list.h>
#include <commons/config.h>


typedef struct{
	int idMemoria;
}Memoria;

// Pool de Memorias
t_list *pool;

Memoria obtenerMemoria(t_config* config);

#endif /* POOLMEM_H_ */
