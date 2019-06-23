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
#include <stdlib.h>

typedef struct{
	char* ipMemoria;
	char* puerto;
}Memoria;

// Pool de Memorias
t_list *pool;

// MANEJO DE MEMORIAS
Memoria obtenerMemoria(t_config* config);


#endif /* POOLMEM_H_ */
