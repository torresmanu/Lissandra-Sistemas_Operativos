/*
 * Gossiping.c
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */
#include "Gossiping.h"

void gossiping(){

	list_iterate(memoriasSeeds,gossipear);

}

void gossipear(void* elem){
	Memoria* mem = (Memoria*)elem;

	if(estaConectada(mem)){
		intercambiarTablas(mem);
		cerrarConexion(mem);
	}
}

bool estaConectada(Memoria *mem){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(mem->ip, mem->puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	mem->socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int status = connect(mem->socket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	if((mem->socket==-1) || (status ==-1))
		return false;
	return true;
}

void intercambiarTablas(Memoria* mem){
	int status1 = mandarTabla(mem);
	int status2 = recibirTablas(mem);

	if(status1<0)
		log_info(g_logger,"Fallo al mandar tabla");

	if(status2<0)
		log_info(g_logger,"Fallo al recibir tabla");
}

int mandarTabla(Memoria* mem){
	uint32_t totalSize;

	char *tablaSerializada = serializarTabla(memoriasConocidas,&totalSize);

	int status = send(mem->socket,tablaSerializada,totalSize,0);

	free(tablaSerializada);

	if(status != totalSize) status = -2;

	return status;
}

//suma los tamaños de ip,puerto,numero y los dos int con los tamaños de ip y puerto
void* sumarTamanios(void* seed,void* elem){
	uint32_t* valor = (uint32_t*) seed;
	Memoria* memoria = (Memoria*) elem;

	*valor = *valor + strlen(memoria->ip) + strlen(memoria->puerto) + sizeof(uint32_t)*3;
	return valor;
}

char* serializarTabla(t_list* memoriasConocidas,uint32_t *totalSize){
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

		uint32_t tamIp = strlen(elem->ip);
		uint32_t tamPuerto = strlen(elem->puerto);

		size_to_send = sizeof(uint32_t);
		memcpy(tablaSerializada,&tamIp,size_to_send);
		offset += size_to_send;

		size_to_send = tamIp;
		memcpy(tablaSerializada,elem->ip,size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(uint32_t);
		memcpy(tablaSerializada,&tamPuerto,size_to_send);
		offset += size_to_send;

		size_to_send = tamPuerto;
		memcpy(tablaSerializada,elem->puerto,size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(uint32_t);
		memcpy(tablaSerializada,&elem->numero,size_to_send);
		offset += size_to_send;
	}

	return tablaSerializada;
}

int recibirTablas(Memoria* mem){
	int status=0;
	uint32_t tamLeer;

	char* buffer = malloc(sizeof(uint32_t));
	status = recv(mem->socket,buffer,sizeof(uint32_t),0);
	if(status != sizeof(uint32_t)) return -2;

	for(int i=0;i < *(int*)buffer;i++){
		Memoria* memNueva = malloc(sizeof(Memoria));

		status = recv(mem->socket,buffer,sizeof(uint32_t),0);
		memcpy(&tamLeer,buffer,sizeof(uint32_t));
		if(status != sizeof(uint32_t)) return -2;

		memNueva->ip = malloc(tamLeer);
		status = recv(mem->socket,memNueva->ip,tamLeer,0);
		if(status != tamLeer) return -2;

		status = recv(mem->socket,buffer,sizeof(uint32_t),0);
		memcpy(&tamLeer,buffer,sizeof(uint32_t));
		if(status != sizeof(uint32_t)) return -2;

		memNueva->puerto = malloc(tamLeer);
		status = recv(mem->socket,memNueva->puerto,tamLeer,0);
		if(status != tamLeer) return -2;

		status = recv(mem->socket,&memNueva->numero,sizeof(uint32_t),0);
		if(status != sizeof(uint32_t)) return -2;

		memNueva->socket=-1;

		agregarMemoria(memNueva);
	}
	free(buffer);
	return status;
}

void agregarMemoria(Memoria* mem){

	bool memoriasIguales(void* elem){
		return ((Memoria*)elem)->numero == mem->numero;
	}

	if(list_any_satisfy(memoriasConocidas,memoriasIguales))
		return;

	list_add(memoriasConocidas,mem);
}

void cerrarConexion(Memoria* mem){
	close(mem->socket);
}

