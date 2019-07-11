/*
 * Gossiping.c
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */
#include "Gossiping.h"

void gossiping(){
	log_info(g_logger,"Comienza el gossiping");
	list_iterate(memoriasSeeds,gossipear);

}

void gossipear(void* elem){
	int estado;
	Memoria* mem = (Memoria*)elem;

	if(estaConectada(mem)){
		estado = mandarYrecibir(mem);
		if(estado<0){
			mem->socket=-1;
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

	int estado = getaddrinfo(mem->ip, mem->puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	mem->socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int status = connect((int)mem->socket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	uint32_t codigo = 0;
	if(status!=-1)
		estado = send(mem->socket,&codigo,sizeof(uint32_t),0);
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	if((mem->socket==-1) || (status ==-1)){
		close(mem->socket);
		mem->socket=-1;
		return false;
	}
	return true;
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
	int status1 = mandarTabla(socket);


	if(status1<0)
		log_info(g_logger,"Fallo al mandar tabla");

	if(status2<0)
		log_info(g_logger,"Fallo al recibir tabla");

	return status2;
}

int mandarTabla(int socket){
	uint32_t totalSize;

	pthread_mutex_lock(&mMemoriasConocidas);
	char *tablaSerializada = serializarTabla(memoriasConocidas,&totalSize);
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

	*valor = *valor + strlen(memoria->ip)+1 + strlen(memoria->puerto)+1 + sizeof(uint32_t)*3;
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

		size_to_send = sizeof(uint32_t);
		memcpy(tablaSerializada+offset,&elem->numero,size_to_send);
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

		memNueva->socket=-1;

		agregarMemoria(memNueva);
		log_info(g_logger,"Recibi memoria numero:%d",memNueva->numero);
	}
	free(buffer);
	log_info(g_logger,"Recibi tabla");
	return status;
}

void agregarMemoria(Memoria* mem){

	bool memoriasIguales(void* elem){
		return ((Memoria*)elem)->numero == mem->numero;
	}

	if(list_any_satisfy(memoriasConocidas,memoriasIguales)){
		free(mem);
		return;
	}

	list_add(memoriasConocidas,mem);
}

void cerrarConexion(Memoria* mem){
	close(mem->socket);
}


