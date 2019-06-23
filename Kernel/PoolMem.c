/*
 * PoolMem.c
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#include "PoolMem.h"

Memoria obtenerMemoria(t_config* config){
	Memoria mem;
	int id = config_get_string_value(config,"IP_MEMORIA");
	int puerto = config_get_string_value(config, "PUERTO_MEMORIA");
	mem.ipMemoria = id;
	mem.puerto = puerto;
	return mem;
}
