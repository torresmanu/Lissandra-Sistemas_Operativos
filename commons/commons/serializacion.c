/*
 * serializacion.c
 *
 *  Created on: 4 jun. 2019
 *      Author: viegasm
 */

#include "serializacion.h"

void* serializarPaquete(resultadoParser rp) {
	int offset = 0;
	int size_to_send;
	contenidoInsert* ci;
	//void* contenidoX;

	switch(rp.accionEjecutar) {
	case(INSERT): {
			//todo: ver con chaco si se le ocurre por qué no lo puedo crear acá
			//contenidoInsert* ci = (contenidoInsert *) rp.contenido;
			ci = (contenidoInsert *) rp.contenido;
			//int size_buffer = sizeof(contenidoX->key) + (strlen(contenidoX->nombreTabla) + 1) * sizeof(char) + sizeof(contenidoX->timestamp) + (strlen(contenidoX->value) + 1) * sizeof(char);
			int size_buffer = sizeof(ci->key) + (strlen(ci->nombreTabla) + 1) * sizeof(char) + sizeof(ci->timestamp) + (strlen(ci->value) + 1) * sizeof(char);
			paqueteInsert* paqueteSerializado = (paqueteInsert*) malloc(size_buffer);

			//todo: chequear si funciona esto o hay que usar directamente enteros (por el enum)
			size_to_send = sizeof(paqueteSerializado->operacion);
			memcpy(paqueteSerializado + offset, &(rp.accionEjecutar), size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(paqueteSerializado->total_size);
			memcpy(paqueteSerializado + offset, &size_buffer, size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(paqueteSerializado->key);
			memcpy(paqueteSerializado + offset, &(ci->key), size_to_send);
			offset += size_to_send;

			int value_size = strlen(ci->value) + 1;
			size_to_send = sizeof(paqueteSerializado->value_size);
			memcpy(paqueteSerializado + offset, &value_size, size_to_send);
			offset += size_to_send;

			size_to_send = value_size;
			memcpy(paqueteSerializado + offset, ci->value, size_to_send);
			offset += size_to_send;

			int nombreTabla_size = strlen(ci->nombreTabla) + 1;
			size_to_send = sizeof(paqueteSerializado->nombreTabla_size);
			memcpy(paqueteSerializado + offset, &nombreTabla_size, size_to_send);
			offset += size_to_send;

			size_to_send = nombreTabla_size;
			memcpy(paqueteSerializado + offset, ci->nombreTabla, size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(paqueteSerializado->timestamp);
			memcpy(paqueteSerializado + offset, ci->timestamp, size_to_send);

			return paqueteSerializado;

			break;
	}
	default:
			return NULL;
	}

}


int recibirYDeserializarPaquete(int socketCliente, resultadoParser* rp) {
	int status;
	contenidoInsert* ci = malloc(sizeof(contenidoInsert));
	int total_size;

	int buffer_size = sizeof(int);
	char* buffer = malloc(buffer_size);
	char* bufferTimestamp = malloc(sizeof(long));

	switch(rp->accionEjecutar) {
	case(INSERT): {
			int recvValue;
			recvValue = recv(socketCliente, buffer, sizeof(int), 0);
			memcpy(&total_size, buffer, buffer_size);
			if (!status) return -2;

			recv(socketCliente, buffer, sizeof(int), 0);
			memcpy(&(ci->key), buffer, buffer_size);
			if (!status) return -2;

			int valueSize;
			recv(socketCliente, buffer, sizeof(int), 0);
			memcpy(&valueSize, buffer, buffer_size);
			if (!status) return -2;

			ci->value = malloc(sizeof(valueSize));
			recv(socketCliente, ci->value, valueSize, 0);
			if (!status) return -2;

			int nombreTablaSize;
			recv(socketCliente, buffer, sizeof(int), 0);
			memcpy(&nombreTablaSize, buffer, buffer_size);
			if (!status) return -2;

			ci->nombreTabla = malloc(nombreTablaSize);
			recv(socketCliente, ci->nombreTabla, nombreTablaSize, 0);
			if (!status) return -2;

			recv(socketCliente, bufferTimestamp, sizeof(long), 0);
			memcpy(&(ci->timestamp), buffer, sizeof(long));
			if (!status) return -2;

			rp->contenido = ci;

			break;
	}
	default:
			return -1;
	}

	return 0;

}


