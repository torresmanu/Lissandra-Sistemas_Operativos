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

	char* buffer = malloc(sizeof(uint32_t));
	status = recv(socket,buffer,sizeof(uint32_t),0);

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

		status = recv(socket,&memNueva->estado,sizeof(memNueva->estado),0);
		if(status != sizeof(memNueva->estado)) return -2;

		status = recv(socket,&memNueva->timestamp,sizeof(memNueva->timestamp),0);
		if(status != sizeof(memNueva->timestamp)) return -2;


		log_info(g_logger,"Recibi memoria:%d, ESTADO: %d",memNueva->id,memNueva->estado);

		bool laTenia = agregarMemoria(memNueva);

		if(laTenia){
			free(memNueva->ipMemoria);
			free(memNueva->puerto);
			free(memNueva);
		}
	}

	free(buffer);
	return status;
}

bool agregarMemoria(Memoria* mem){

	for(int i=0;i<pool->elements_count;i++){
		Memoria* memConocida = list_get(pool,i);
		if(coincideIPyPuerto(mem,memConocida)){
			if(mem->timestamp > memConocida->timestamp){
				memConocida->estado=mem->estado;
				memConocida->timestamp=mem->timestamp;
			}

			return true;
		}
	}

	inicializarMemoria(mem);

	list_add(pool,mem);
	return false;
}

bool coincideIPyPuerto(Memoria* mem1,Memoria* mem2){
	return strcmp(mem1->ipMemoria,mem2->ipMemoria)==0 && strcmp(mem1->puerto,mem2->puerto)==0;
}

void inicializarMemoria(Memoria* memNueva){
	memNueva->insertsTotales = 0;
	memNueva->selectsTotales = 0;
	memNueva->totalOperaciones = 0;
	pthread_mutex_init(&(memNueva->mutex),NULL);
	memNueva->conexiones = dictionary_create();

	int centinela=-1;
	int* conexion = malloc(sizeof(centinela));
	memcpy(conexion,&centinela,sizeof(centinela));
	dictionary_put(memNueva->conexiones,idGossiping,conexion);

	inicializarConexiones(memNueva);

	log_info(g_logger,"Agregue memoria nueva numero:%d",memNueva->id);
}

void inicializarConexiones(Memoria* mem){

	void setearConexiones(void* elem){
		char* id = elem;
		int centinela=-1;
		int* conexion = malloc(sizeof(centinela));
		memcpy(conexion,&centinela,sizeof(centinela));
		dictionary_put(mem->conexiones,id,conexion);
	}


	list_iterate(idsEjecutadores,setearConexiones);

	setearConexiones(idGossiping);
}

bool sigueConectada(Memoria* memVieja, t_list* memoriasRecibidas){
	bool coincideId(void* elem){
		Memoria* mem = elem;
		return mem->id == memVieja->id;
	}

	return list_any_satisfy(memoriasRecibidas,coincideId);
}

bool tengoMemoria(Memoria* memNueva){
	bool coincideId(void* elem){
		Memoria* mem = elem;
		return mem->id == memNueva->id;
	}

	return list_any_satisfy(pool,coincideId);
}

bool estoyConectado(int* conexion){
	return  *conexion!= -1;
}

void obtenerMemoriaDescribe()
{

	MemDescribe = malloc(sizeof(Memoria));
	MemDescribe->id = 22;
	MemDescribe->ipMemoria = config_get_string_value(g_config,"IP_MEMORIA");
	MemDescribe->puerto = config_get_string_value(g_config, "PUERTO_MEMORIA");
	MemDescribe->insertsTotales = 0;
	MemDescribe->selectsTotales = 0;
	MemDescribe->totalOperaciones = 0;
	pthread_mutex_init(&(MemDescribe->mutex),NULL);
	MemDescribe->conexiones = dictionary_create();
	MemDescribe->estado=1;
	ponerTimestampActual(MemDescribe);
	inicializarConexiones(MemDescribe);

	list_add(pool,MemDescribe);

	conectarEjecutadores();
	conectarGossiping();

}

Memoria *buscarMemoria(int numero){
	bool numerosIguales(void* elem){
		return ((Memoria*)elem)->id == numero;
	}
	return list_find(pool,numerosIguales);
}

void gossiping(){
	Memoria* mem = NULL;
	int status=-1;

	log_info(g_logger,"Gossiping en proceso..");

	for(int i=0;i<pool->elements_count;i++){
		mem = list_get(pool,i);
		int* conexion = dictionary_get(mem->conexiones,idGossiping);

		status = obtenerMemorias(*conexion);

		if(status>=0){
			log_info(g_logger,"Gossiping completado con memoria %d.",mem->id);

			conectarEjecutadores();
			conectarGossiping();

			break;
		}
		else
			*conexion=-1;
	}
	if(status<0)
		log_info(g_logger,"No hay memorias activas para hacer gossiping");
}

void conectarGossiping(){
	conectarMemorias(idGossiping);
}

void conectarEjecutadores(){
	list_iterate(idsEjecutadores,conectarMemorias);
}

void desconectarEjecutadores(Memoria* mem){

	dictionary_iterator(mem->conexiones,cerrarConexion);
}

void conectarMemorias(void* elem){
	char* id=elem;

	void conectarAsociadas(void* elem){
		Criterio* crit = elem;

		void conectar(void* elem){
			Memoria* mem = elem;
			if(mem->estado==1){
				int resultado = gestionarConexionAMemoria(mem,id);
				if(resultado<0)
					sacarMemoria(mem);
			}
			else
				sacarMemoria(mem);
		}

		list_iterate(crit->memorias,conectar);

	}

	list_iterate(criterios,conectarAsociadas);
}

void desconectarMemoria(Memoria* mem,char* id){
	int* conexion = dictionary_get(mem->conexiones,id);
	close(*conexion);
	free(conexion);
}

void cerrarConexion(char* key, void* value){
	int* conexion = value;

	if(*conexion != -1)
		close(*conexion);

	*conexion=-1;

}

void sacarMemoria(Memoria* mem){
	bool coincideId(void* elem){
		Memoria* memN = elem;
		return memN->id == mem->id;
	}

	mem->estado=0;
	ponerTimestampActual(mem);

	desconectarEjecutadores(mem);

	list_remove_by_condition(sc.memorias,coincideId);
	list_remove_by_condition(shc.memorias,coincideId);
	list_remove_by_condition(ec.memorias,coincideId);
}

//Memoria* obtenerMemoriaActiva(char* idG){
//	Memoria* mem;
//
//	for(int i=0;i<pool->elements_count;i++){
//		mem = list_get(pool,i);
//
//		if(estaActiva(mem))
//			break;
//		else
//			mem=NULL;
//	}
//
//	return mem;
//}
//
//bool estaActiva(Memoria* mem,char* idG){
//	int* conexion = dictionary_get(mem->conexiones,idG);
//
//	send(*conexion,)
//}

void establecerConexionPool(char* id)
{
	Memoria* mem;

	for(int i = 0; i<pool->elements_count; i++)
	{
		mem = list_get(pool,i);
		int* conexion=dictionary_get(mem->conexiones,id);

		if(!estoyConectado(conexion)){
			int res = gestionarConexionAMemoria(mem,id);
			if(res<0)
				sacarMemoria(mem);
		}
	}
}



void agregarMutex(Memoria* mem){
	pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex,NULL);

	dictionary_put(mutexsConexiones,mem->puerto,mutex);
}

void liberarMutexs(){
	dictionary_destroy_and_destroy_elements(mutexsConexiones,liberarMutex);
}

void liberarMutex(void* elem)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)elem;
	pthread_mutex_destroy(mutex);
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

void ponerTimestampActual(Memoria* mem){
	struct timeval te;
	gettimeofday(&te, NULL);
	mem->timestamp = te.tv_sec*1000LL + te.tv_usec/1000;
}

