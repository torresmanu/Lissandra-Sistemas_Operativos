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
	resultadoParser* gossip = malloc(sizeof(resultadoParser));
	resultado res;

	//gossip->accionEjecutar = GOSSIPING;			// NO ESTA CONTEMPLADO EN EL PARSER
	char* msg = serializarPaquete(gossip,&size);
	send(memoriaSocket, msg, size, 0);

	int status = recibirYDeserializarRespuesta(memoriaSocket,&res);
	if(status<0)
	{
		log_info(g_logger, "Gossiping fallando");
	}
	else
	{
		printf("Cantidad de memorias en el Pool: ", pool->elements_count);
		memorias = (t_list*)res.contenido;										// CONTENIDO TIENE QUE MODIFICARSE EN CASO GOSSIP
		list_add_all(pool,memorias);
		log_info(g_logger,"Gossiping realizado con éxito y pool actualizado\n");
		log_info(g_logger,"Cantidad de memorias en el Pool: %d\n", pool->elements_count);
	}
	free(msg);
	free(gossip);
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
