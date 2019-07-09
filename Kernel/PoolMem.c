/*
 * PoolMem.c
 *
 *  Created on: 29 may. 2019
 *      Author: utnso
 */

#include "PoolMem.h"

void obtenerMemorias(){
	// ACA LE PIDO GOSSIPING A LA MEMORIA DEL CONFIG

	t_list* memorias;
	int size;
	int valueResponse;
	accion acc;
	char* buffer = malloc(sizeof(char));
	resultadoParser* gossip = malloc(sizeof(resultadoParser));
	resultado res;

	//gossip->accionEjecutar = GOSSIPING;			// NO ESTA CONTEMPLADO EN EL PARSER
	char* msg = serializarPaquete(gossip,&size);
	send(memoriaSocket, msg, size, 0);
	valueResponse = recv(memoriaSocket,buffer,sizeof(int),0);
	memcpy(&acc,buffer,sizeof(int));								// Me fijo que accion para saber como deserializar

	res.accionEjecutar = acc;
	int status = recibirYDeserializarRespuesta(memoriaSocket,&res);
	if(status<0)
	{
		log_error(g_logger, "Gossiping fallando");
	}
	else
	{
		if(valueResponse < 0)
		{
			log_error(g_logger,strerror(errno));
		}
		else if(valueResponse == 0)
		{
			log_error(g_logger,"Posiblemente se desconectó la memoria");
		}
		else
		{
			printf("Cantidad de memorias en el Pool: %i\n", pool->elements_count);
			memorias = (t_list*)res.contenido;										// CONTENIDO TIENE QUE MODIFICARSE EN CASO GOSSIP
			list_add_all(pool,memorias);
			log_info(g_logger,"Gossiping realizado con éxito y pool actualizado\n");
			log_info(g_logger,"Cantidad de memorias en el Pool: %d\n", pool->elements_count);
		}
	}
	free(msg);
	free(gossip);
	free(buffer);
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
