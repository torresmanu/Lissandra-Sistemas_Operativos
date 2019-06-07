/*
 * serializacion.c
 *
 *  Created on: 4 jun. 2019
 *      Author: viegasm
 */

#include "serializacion.h"

char* serializarPaquete(resultadoParser* rp) {
	int offset = 0;
	int size_to_send;
	int size_buffer;
	int value_size;
	int nombreTabla_size;
	contenidoInsert* ci;

	switch(rp->accionEjecutar) {
	case(INSERT): {
		offset = 0;
		ci = (contenidoInsert *) (rp->contenido);

		size_buffer = sizeof(ci->key) + (strlen(ci->nombreTabla) + 1) * sizeof(char)
				+ sizeof(ci->timestamp) + (strlen(ci->value) + 1) * sizeof(char)
				+ sizeof(rp->accionEjecutar) + sizeof(size_buffer) + sizeof(value_size) + sizeof(nombreTabla_size);

		//paqueteInsert* paqueteSerializado = (paqueteInsert*) malloc(size_buffer);
		char* paqueteSerializado2 = (char*) malloc(size_buffer);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado2 + offset, &(rp->accionEjecutar), size_to_send);
		//printf("[serializarPaquete] Accion serializada: %i\n", paqueteSerializado2 + offset);
		offset += size_to_send;


		size_to_send = sizeof(size_buffer);
		memcpy(paqueteSerializado2 + offset, &size_buffer, size_to_send);
		//printf("[serializarPaquete] Tamaño total serializado: %i\n", paqueteSerializado2 + offset);
		offset += size_to_send;


		size_to_send = sizeof(ci->key);
		memcpy(paqueteSerializado2 + offset, &(ci->key), size_to_send);
		//printf("[serializarPaquete] Key serializada: %i\n", paqueteSerializado2 + offset);
		offset += size_to_send;


		value_size = strlen(ci->value) + 1;
		size_to_send = sizeof(value_size);
		memcpy(paqueteSerializado2 + offset, &value_size, size_to_send);
		//printf("[serializarPaquete] Tamaño value serializado: %i\n", paqueteSerializado2 + offset);
		offset += size_to_send;


		size_to_send = value_size;
		memcpy(paqueteSerializado2 + offset, ci->value, size_to_send);
		//printf("[serializarPaquete] Value serializado: %s\n", paqueteSerializado2 + offset);
		offset += size_to_send;

		nombreTabla_size = strlen(ci->nombreTabla) + 1;
		size_to_send = sizeof(nombreTabla_size);
		memcpy(paqueteSerializado2 + offset, &nombreTabla_size, size_to_send);
		//printf("[serializarPaquete] Tamaño nombreTabla serializado: %i\n", paqueteSerializado2 + offset);
		offset += size_to_send;


		size_to_send = nombreTabla_size;
		memcpy(paqueteSerializado2 + offset, ci->nombreTabla, size_to_send);
		//printf("[serializarPaquete] Nombre tabla serializado: %s\n", paqueteSerializado2 + offset);
		offset += size_to_send;


		size_to_send = sizeof(ci->timestamp);
		memcpy(paqueteSerializado2 + offset, &(ci->timestamp), size_to_send);
		//printf("[serializarPaquete] Timestamp serializado: %ld\n", paqueteSerializado2 + offset);

		return paqueteSerializado2;

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

	int valueSize;
	int nombreTablaSize;

	switch(rp->accionEjecutar) {
	case(INSERT): {
		printf("[recibirYDeserializarPaquete] Entro en el case INSERT\n");

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(ci->key), buffer, buffer_size);
		printf("[recibirYDeserializarPaquete] key recibida = %i\n", ci->key);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&valueSize, buffer, buffer_size);
		if (!status) return -2;

		ci->value = malloc(sizeof(valueSize));
		status = recv(socketCliente, ci->value, valueSize, 0);
		printf("[recibirYDeserializarPaquete] value recibido = %s\n", ci->value);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&nombreTablaSize, buffer, buffer_size);
		if (!status) return -2;

		ci->nombreTabla = malloc(nombreTablaSize);
		status = recv(socketCliente, ci->nombreTabla, nombreTablaSize, 0);
		printf("[recibirYDeserializarPaquete] nombreTabla recibido = %s\n", ci->nombreTabla);
		if (!status) return -2;

		status = recv(socketCliente, bufferTimestamp, sizeof(long), 0);
		memcpy(&(ci->timestamp), bufferTimestamp, sizeof(long));
		printf("[recibirYDeserializarPaquete] Timestamp recibido = %ld\n", ci->timestamp);
		if (!status) return -2;

		rp->contenido = ci;

		break;
	}
	default:
		return -1;
	}

	return 0;

}


