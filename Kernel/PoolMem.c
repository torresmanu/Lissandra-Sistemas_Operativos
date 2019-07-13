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

		log_info(g_logger,"Recibi memoria:%d",memNueva->id);

		if(!tengoMemoria(memNueva)){
			memNueva->socket=-1;

			list_add(pool,memNueva);
			log_info(g_logger,"Agregue memoria nueva numero:%d",memNueva->id);
		}
	}

	free(buffer);
	log_info(g_logger,"Recibi tabla completa");
	return status;
}

bool tengoMemoria(Memoria* memNueva){
	bool coincideId(void* elem){
		Memoria* mem = elem;
		return mem->id == memNueva->id;
	}

	return list_any_satisfy(pool,coincideId);
}

bool estoyConectado(Memoria* mem){
	return mem->socket != -1;
}

void obtenerMemoriaDescribe()
{
	MemDescribe = malloc(sizeof(Memoria));
	MemDescribe->id = 22;
	MemDescribe->ipMemoria = config_get_string_value(g_config,"IP_MEMORIA");
	MemDescribe->puerto = config_get_string_value(g_config, "PUERTO_MEMORIA");
	MemDescribe->socket = -1;
	list_add(pool,MemDescribe);
}

Memoria *buscarMemoria(int numero){
	bool numerosIguales(void* elem){
		return ((Memoria*)elem)->id == numero;
	}
	return list_find(pool,numerosIguales);
}

void gossiping(){
	establecerConexionPool();

	bloquearConexion(MemDescribe);
	int status = obtenerMemorias(MemDescribe->socket);
	desbloquearConexion(MemDescribe);

}

void establecerConexionPool()
{
	Memoria* mem;

	for(int i = 0; i<pool->elements_count; i++)
	{
		mem = list_get(pool,i);
		if(!estoyConectado(mem))
			gestionarConexionAMemoria(mem);
	}
}



void agregarMutex(Memoria* mem){
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);

	dictionary_put(mutexsConexiones,mem->puerto,mutex);
}

void liberarMutexs(){
	dictionary_destroy_and_destroy_elements(mutexsConexiones,pthread_mutex_destroy);
}

pthread_mutex_t* obtenerMutex(Memoria* mem){
	return dictionary_get(mutexsConexiones,mem->puerto);
}

void bloquearConexion(Memoria* mem){
	pthread_mutex_t* mConexion = obtenerMutex(mem);

	pthread_mutex_lock(mConexion);
}

void desbloquearConexion(Memoria* mem){
	pthread_mutex_t* mConexion = obtenerMutex(mem);

	pthread_mutex_unlock(mConexion);
}

