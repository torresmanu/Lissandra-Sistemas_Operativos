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
	char* puerto = config_get_string_value(g_config,"PUERTO_MEMORIA");
	mem->puerto = strdup(puerto);

	gossiping(mem);//meto en pool la lista de memorias encontradas

	// ACA TENGO QUE TIRAR UN HILO POR CONEXION POR CADA MEMORIA DEL POOL
}

// HARDCODEADO SOLO COMO PARA EJEMPLO.	////////////////////
void gossiping(Memoria *mem){
	list_add(pool,mem);

	Memoria *m1 = malloc(sizeof(Memoria));
	m1->id = 1;

	char* ip = config_get_string_value(g_config,"IP_MEMORIA");
	m1->ipMemoria = strdup(ip);

	char* puerto = config_get_string_value(g_config,"PUERTO_MEMORIA");
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

void obtenerMemoriaDescribe()
{
	MemDescribe = malloc(sizeof(Memoria));
	MemDescribe->id = 0;
	MemDescribe->ipMemoria = config_get_string_value(g_config,"IP_MEMORIA");
	MemDescribe->puerto = config_get_string_value(g_config, "PUERTO_MEMORIA");
}

Memoria *buscarMemoria(int numero){
	bool numerosIguales(void* elem){
		return ((Memoria*)elem)->id == numero;
	}
	return list_find(pool,numerosIguales);
}
