/*
 * serializacion.h
 *
 *  Created on: 4 jun. 2019
 *      Author: viegasm
 */

#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdio.h>
#include "parser.h"

typedef struct {
	int operacion;
	int total_size;

	int key;

	int value_size;
	char* value;

	int nombreTabla_size;
	char* nombreTabla;

	long timestamp;
} paqueteInsert;

char* serializarPaquete(resultadoParser* rp);
int recibirYDeserializarPaquete(int socketCliente, resultadoParser* rp);

#endif /* SERIALIZACION_H_ */
