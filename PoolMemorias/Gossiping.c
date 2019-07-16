/*
 * Gossiping.c
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */
#include "Gossiping.h"

void gossiping(){
	log_info(g_logger,"Comienza el gossiping");
	t_list* memoriasGossiping = obtenerMemoriasGossiping();
	list_iterate(memoriasGossiping,gossipear);
//	list_iterate(memoriasSeeds,gossipear);
	log_info(g_logger,"Termino el gossiping");
}

t_list* obtenerMemoriasGossiping(){
	pthread_mutex_lock(&mMemoriasConocidas);
	t_list* memoriasGossiping = list_filter(memoriasConocidas,noSoyYo);
	pthread_mutex_unlock(&mMemoriasConocidas);

	agregarSeeds(memoriasGossiping);
	return memoriasGossiping;
}

void agregarSeeds(t_list* memoriasGossiping){

	void veoSiLaAgrego(void* elem){
		Memoria* mem = elem;

		bool mismaMemoria(void* elem2){
			return coincideIPyPuerto((Memoria*)elem2,mem);
		}

		if(list_any_satisfy(memoriasGossiping,mismaMemoria))
			return;
		else
			list_add(memoriasGossiping,mem);
	}

	list_iterate(memoriasSeeds,veoSiLaAgrego);
}

bool coincideIPyPuerto(Memoria* mem1,Memoria* mem2){
	return strcmp(mem1->ip,mem2->ip)==0 && strcmp(mem1->puerto,mem2->puerto)==0;
}

bool noSoyYo(void* elem){
	Memoria* mem=elem;
	return mem->numero != yo->numero;
}

void gossipear(void* elem){
	int estado;
	Memoria* mem = (Memoria*)elem;

	bool mismaMemoria(void* elem2){
		return coincideIPyPuerto((Memoria*)elem2,mem);
	}

	if(estaConectada(mem)){
		estado = mandarYrecibir(mem);
		if(estado<0){
			pthread_mutex_lock(&mMemoriasConocidas);
			mem->socket=-1;
			mem->estado=0;
			ponerTimestampActual(mem);
			pthread_mutex_unlock(&mMemoriasConocidas);
		}
	}
}

bool estaConectada(Memoria *mem){
	if(mem->socket>0)
		return true;
	bool estado = conectarMemoria(mem);
	return estado;
}

bool conectarMemoria(Memoria* mem){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(mem->ip, mem->puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	mem->socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int status = connect((int)mem->socket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	uint32_t codigo = 0;
	if(status!=-1){
		send(mem->socket,&codigo,sizeof(uint32_t),0);
		status = recv(mem->socket,&(mem->numero),sizeof(mem->numero),0);
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	if(status ==-1){
		close(mem->socket);
		mem->socket=-1;
		mem->estado=0;
		ponerTimestampActual(mem);
		return false;
	}

	pthread_mutex_lock(&mMemoriasConocidas);
	mem->estado=1;
	ponerTimestampActual(mem);
	agregarMemoria(mem);
	pthread_mutex_unlock(&mMemoriasConocidas);

	return true;
}

void ponerTimestampActual(Memoria* mem){
	struct timeval te;
	gettimeofday(&te, NULL);
	mem->timestamp = te.tv_sec*1000LL + te.tv_usec/1000;
}

int mandarYrecibir(Memoria* mem){
	int status1 = mandarTabla(mem->socket);
	if(status1<0){
		log_info(g_logger,"Fallo al mandar tabla");
		return status1;
	}

	int status2 = recibirTablas(mem->socket);
	if(status2<0)
		log_info(g_logger,"Fallo al recibir tabla");
	return status2;
}

int recibirYmandar(int socket){

	int status2 = recibirTablas(socket);
	if(status2<0){
		log_info(g_logger,"Fallo al recibir tabla");
		return status2;
	}

	int status1 = mandarTabla(socket);
	if(status1<0)
		log_info(g_logger,"Fallo al mandar tabla");
	return status1;
}

int mandarTabla(int socket){
	uint32_t totalSize;

	ponerTimestampActual(yo);

	pthread_mutex_lock(&mMemoriasConocidas);
	char *tablaSerializada = serializarTabla(&totalSize);
	pthread_mutex_unlock(&mMemoriasConocidas);

	int status = send(socket,tablaSerializada,totalSize,0);

	free(tablaSerializada);

	if(status != totalSize) status = -2;

	log_info(g_logger,"Mande tabla,tamaño del mensaje:%d",totalSize);

	return status;
}

//suma los tamaños de ip,puerto,numero y los dos int con los tamaños de ip y puerto
void* sumarTamanios(void* seed,void* elem){
	uint32_t* valor = (uint32_t*) seed;
	Memoria* memoria = (Memoria*) elem;

	*valor = *valor + strlen(memoria->ip)+1 + strlen(memoria->puerto)+1 + sizeof(uint32_t)*3
			+ sizeof(memoria->estado) + sizeof(memoria->timestamp);
	return valor;
}

char* serializarTabla(uint32_t *totalSize){
	uint32_t size_to_send, offset=0;
	uint32_t seed=0;
	uint32_t cantElementos = memoriasConocidas->elements_count;

	*totalSize = *(uint32_t*)list_fold(memoriasConocidas,&seed,sumarTamanios) + sizeof(uint32_t);

	char* tablaSerializada = malloc(*totalSize);

	size_to_send = sizeof(cantElementos);
	memcpy(tablaSerializada,&cantElementos,size_to_send);
	offset += size_to_send;

	for(uint32_t i=0;i<cantElementos;i++){
		Memoria* elem = list_get(memoriasConocidas,i);

		uint32_t tamIp = strlen(elem->ip)+1;
		uint32_t tamPuerto = strlen(elem->puerto)+1;

		size_to_send = sizeof(uint32_t);
		memcpy(tablaSerializada+offset,&tamIp,size_to_send);
		offset += size_to_send;

		size_to_send = tamIp;
		memcpy(tablaSerializada+offset,elem->ip,size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(uint32_t);
		memcpy(tablaSerializada+offset,&tamPuerto,size_to_send);
		offset += size_to_send;

		size_to_send = tamPuerto;
		memcpy(tablaSerializada+offset,elem->puerto,size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(elem->numero);
		memcpy(tablaSerializada+offset,&elem->numero,size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(elem->estado);
		memcpy(tablaSerializada+offset,&elem->estado,size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(elem->timestamp);
		memcpy(tablaSerializada+offset,&elem->timestamp,size_to_send);
		offset += size_to_send;
	}

	return tablaSerializada;
}

int recibirTablas(int socket){
	int status=0;
	uint32_t tamLeer;

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

		memNueva->ip = malloc(tamLeer);
		status = recv(socket,memNueva->ip,tamLeer,0);
		if(status != tamLeer) return -2;

		status = recv(socket,buffer,sizeof(uint32_t),0);
		memcpy(&tamLeer,buffer,sizeof(uint32_t));
		if(status != sizeof(uint32_t)) return -2;

		memNueva->puerto = malloc(tamLeer);
		status = recv(socket,memNueva->puerto,tamLeer,0);
		if(status != tamLeer) return -2;

		status = recv(socket,&memNueva->numero,sizeof(uint32_t),0);
		if(status != sizeof(uint32_t)) return -2;

		status = recv(socket,&memNueva->estado,sizeof(memNueva->estado),0);
		if(status != sizeof(memNueva->estado)) return -2;

		status = recv(socket,&memNueva->timestamp,sizeof(memNueva->timestamp),0);
		if(status != sizeof(memNueva->timestamp)) return -2;

		memNueva->socket=-1;

		pthread_mutex_lock(&mMemoriasConocidas);
		bool laTenia = agregarMemoria(memNueva);
		pthread_mutex_unlock(&mMemoriasConocidas);

		log_info(g_logger,"Recibi memoria numero:%d, ESTADO= %d",memNueva->numero,memNueva->estado);

		if(laTenia){
			free(memNueva->ip);
			free(memNueva->puerto);
			free(memNueva);
		}
	}

	free(buffer);
	return status;
}

void sacarMemoria(Memoria* mem){
	bool mismaMemoria(void* elem2){
		return coincideIPyPuerto((Memoria*)elem2,mem);
	}

	list_remove_and_destroy_by_condition(memoriasConocidas,mismaMemoria,destroy_nodo_memoria);
}

bool estaEnElPool(Memoria* mem, t_list* memoriasRecibidas){

	bool mismaMemoria(void* elem2){
		return coincideIPyPuerto((Memoria*)elem2,mem);
	}

	return list_any_satisfy(memoriasRecibidas,mismaMemoria);
}

bool agregarMemoria(Memoria* mem){

	for(int i=0;i<memoriasConocidas->elements_count;i++){
		Memoria* memConocida = list_get(memoriasConocidas,i);
		if(coincideIPyPuerto(mem,memConocida)){
			if(mem->timestamp > memConocida->timestamp){
				memConocida->estado=mem->estado;
				memConocida->timestamp=mem->timestamp;
			}
			return true;
		}
	}

	list_add(memoriasConocidas,mem);
	return false;
}



void cerrarConexion(Memoria* mem){
	close(mem->socket);
}

void destroy_nodo_memoria(void* elem){
	Memoria* mem = (Memoria*) elem;
	free(mem->ip);
	free(mem->puerto);
	free(mem);
	mem=NULL;
}

void destroy_nodo_memoria_conocida(void* elem){
	Memoria* mem = (Memoria*) elem;
	if(mem!=NULL)
		destroy_nodo_memoria(elem);
}
