/*
 * PoolMem.c
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#include "PoolMem.h"

Memoria obtenerMemoria(t_config* config){
	Memoria mem;
	int id = config_get_int_value(config,"MEMORIA");
	mem.idMemoria = id;
	return mem;
}
