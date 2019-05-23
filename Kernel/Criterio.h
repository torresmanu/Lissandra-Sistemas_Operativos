/*
 * Criterio.h
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#ifndef CRITERIO_H_
#define CRITERIO_H_

#include <commons/config.h>

typedef enum{SC, SHC, EC} t_consist;

typedef struct{
	int idMemoria;
}Memoria;

// Pool de Memorias
typedef t_list* t_memoria;

Memoria criterioSC();

#endif /* CRITERIO_H_ */
