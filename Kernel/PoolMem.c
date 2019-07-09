/*
 * PoolMem.c
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#include "PoolMem.h"

int obtenerMemorias(int socket){
	int status=0;
	uint32_t tamLeer;

	accion pedido = GOSSIPING;
	send(socket,&pedido,sizeof(int),0);
	log_info(g_logger,"Mando solicitud de gossiping");

	char* buffer = malloc(sizeof(uint32_t));
	status = recv(socket,buffer,sizeof(uint32_t),0);
	if(status==-1)
		perror("Error recv");
	if(status != sizeof(uint32_t)) return -2;
	int cantElem = *(int*)buffer;

	for(int i=0;i < cantElem;i++){
		Memoria* memNueva = malloc(sizeof(Memoria));

		status = recv(socket,buffer,sizeof(uint32_t),0);
		memcpy(&tamLeer,buffer,sizeof(uint32_t));
		if(status != sizeof(uint32_t)) return -2;

		memNueva->ipMemoria = malloc(tamLeer);
		status = recv(socket,memNueva->ipMemoria,tamLeer,0);
		if(status != tamLeer) return -2;

		status = recv(socket,buffer,sizeof(uint32_t),0);
		memcpy(&tamLeer,buffer,sizeof(uint32_t));
		if(status != sizeof(uint32_t)) return -2;

		memNueva->puerto = malloc(tamLeer);
		status = recv(socket,memNueva->puerto,tamLeer,0);
		if(status != tamLeer) return -2;

		status = recv(socket,&memNueva->id,sizeof(uint32_t),0);
		if(status != sizeof(uint32_t)) return -2;

		memNueva->socket=-1;

		list_add(pool,memNueva);
		log_info(g_logger,"Recibi memoria numero:%d",memNueva->id);
	}
	free(buffer);
	log_info(g_logger,"Recibi tabla completa");
	return status;
}

void obtenerMemoriaDescribe()
{
	MemDescribe = malloc(sizeof(Memoria));
	MemDescribe->id = 22;
	MemDescribe->ipMemoria = config_get_string_value(g_config,"IP_MEMORIA");
	MemDescribe->puerto = config_get_string_value(g_config, "PUERTO_MEMORIA");
}

Memoria *buscarMemoria(int numero){
	bool numerosIguales(void* elem){
		return ((Memoria*)elem)->id == numero;
	}
	return list_find(pool,numerosIguales);
}
