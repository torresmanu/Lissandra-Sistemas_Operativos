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
#include "metadata.h"
#include "registro.h"
#include "collections/list.h"

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

typedef struct {
	int operacion;
	int total_size;

	int nombreTabla_size;
	char* nombreTabla;

	int key;
} paqueteSelect;

typedef struct {
	int operacion;
	int total_size;

	int nombreTabla_size;
	char * nombreTabla;

	int consistencia_size;
	char * consistencia;

	int cant_part;
	int tiempo_compresion;
} paqueteCreate;

typedef struct {
	int operacion;
	int total_size;

	int nombreTabla_size;
	char* nombreTabla;
} paqueteDescribe;

typedef struct {
	int operacion;
	int total_size;

	int nombreTabla_size;
	char* nombreTabla;
} paqueteDrop;

typedef struct {
	int operacion;
	int total_size;

	int numMem;

	int criterio_size;
	char* criterio;
} paqueteAdd;

char* serializarPaquete(resultadoParser* rp, int* size_to_send);
int recibirYDeserializarPaquete(int socketCliente, resultadoParser* rp);
char* serializarRespuesta(resultado* res, int* size_to_send);
int recibirYDeserializarRespuesta(int socketCliente, resultado* res);

#endif /* SERIALIZACION_H_ */
