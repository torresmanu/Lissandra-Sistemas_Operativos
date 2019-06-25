/*
 * PoolMem.c
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#include "PoolMem.h"

void obtenerMemorias(){
	Memoria *mem = malloc(sizeof(Memoria));
	mem->id = 0;
	char* ip = config_get_string_value(g_config,"IP_MEMORIA");
	mem->ipMemoria = strdup(ip);
	char* puerto = config_get_string_value(g_config,"IP_MEMORIA");
	mem->puerto = strdup(puerto);

	gossiping(mem);//meto en pool la lista de memorias encontradas

}

// HARDCODEADO SOLO COMO PARA EJEMPLO.	////////////////////
void gossiping(Memoria *mem){
	list_add(pool,mem);

	Memoria *m1 = malloc(sizeof(Memoria));
	m1->id = 1;

	char* ip = config_get_string_value(g_config,"IP_MEMORIA");
	m1->ipMemoria = strdup(ip);

	char* puerto = config_get_string_value(g_config,"IP_MEMORIA");
	m1->puerto = strdup(puerto);

	list_add(pool,m1);


	Memoria *m2 = malloc(sizeof(Memoria));
	m2->id = 2;
	m2->ipMemoria = strdup(ip);
	m2->puerto = strdup(puerto);
	list_add(pool,m2);

	Memoria *m3 = malloc(sizeof(Memoria));
	m3->id = 3;
	m3->ipMemoria = strdup(ip);
	m3->puerto = strdup(puerto);
	list_add(pool,m3);
}

Memoria obtenerMemoria(t_config* config){
	Memoria mem;
	int id = config_get_string_value(config,"IP_MEMORIA");
	int puerto = config_get_string_value(config, "PUERTO_MEMORIA");
	mem.ipMemoria = id;
	mem.puerto = puerto;
	return mem;
}

Memoria *buscarMemoria(int numero){
	bool numerosIguales(void* elem){
		return ((Memoria*)elem)->id == numero;
	}
	return list_find(pool,numerosIguales);
}
