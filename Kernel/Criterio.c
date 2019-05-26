/*
 * Criterio.c
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#include "Criterio.h"

Memoria obtenerMemoria(t_config* config){
	Memoria mem;
	int id = config_get_int_value(config,"MEMORIA");
	mem.idMemoria = id;
	return mem;
}


Memoria criterioSC(){}

t_consist obtenerConsistencia();
