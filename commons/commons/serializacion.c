/*
 * serializacion.c
 *
 *  Created on: 4 jun. 2019
 *      Author: viegasm
 */

#include "serializacion.h"

char* serializarPaquete(resultadoParser* rp, int* total_size) {
	int offset = 0;
	int size_to_send;

	switch(rp->accionEjecutar) {
	case(INSERT): {
		int nombreTabla_size;
		int value_size;
		contenidoInsert* ci;
		offset = 0;
		ci = (contenidoInsert *) (rp->contenido);

		*total_size = sizeof(ci->key) + (strlen(ci->nombreTabla) + 1) * sizeof(char)
				+ sizeof(ci->timestamp) + (strlen(ci->value) + 1) * sizeof(char)
				+ sizeof(rp->accionEjecutar) + sizeof(int) + sizeof(value_size) + sizeof(nombreTabla_size);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(ci->key);
		memcpy(paqueteSerializado + offset, &(ci->key), size_to_send);
		offset += size_to_send;


		value_size = strlen(ci->value) + 1;
		size_to_send = sizeof(value_size);
		memcpy(paqueteSerializado + offset, &value_size, size_to_send);
		offset += size_to_send;


		size_to_send = value_size;
		memcpy(paqueteSerializado + offset, ci->value, size_to_send);
		offset += size_to_send;

		nombreTabla_size = strlen(ci->nombreTabla) + 1;
		size_to_send = sizeof(nombreTabla_size);
		memcpy(paqueteSerializado + offset, &nombreTabla_size, size_to_send);
		offset += size_to_send;


		size_to_send = nombreTabla_size;
		memcpy(paqueteSerializado + offset, ci->nombreTabla, size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(ci->timestamp);
		memcpy(paqueteSerializado + offset, &(ci->timestamp), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(SELECT): {
		int nombreTabla_size;
		contenidoSelect* cs;
		offset = 0;
		cs = (contenidoSelect *) (rp->contenido);

		*total_size = sizeof(rp->accionEjecutar) + sizeof(int)
				+ (strlen(cs->nombreTabla) + 1) * sizeof(char) + sizeof(nombreTabla_size)
				+ sizeof(cs->key);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;


		nombreTabla_size = strlen(cs->nombreTabla) + 1;
		size_to_send = sizeof(nombreTabla_size);
		memcpy(paqueteSerializado + offset, &nombreTabla_size, size_to_send);
		offset += size_to_send;


		size_to_send = nombreTabla_size;
		memcpy(paqueteSerializado + offset, cs->nombreTabla, size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(cs->key);
		memcpy(paqueteSerializado + offset, &(cs->key), size_to_send);
		offset += size_to_send;


		return paqueteSerializado;

		break;
	}
	case(CREATE): {
		int nombreTabla_size;
		int consistencia_size;
		contenidoCreate* cc;
		offset = 0;
		cc = (contenidoSelect *) (rp->contenido);

		*total_size = sizeof(rp->accionEjecutar) + sizeof(int)
				+ (strlen(cc->nombreTabla) + 1) * sizeof(char) + sizeof(nombreTabla_size)
				+ (strlen(cc->consistencia) + 1) * sizeof(char) + sizeof(consistencia_size)
				+ sizeof(cc->cant_part)
				+ sizeof(cc->tiempo_compresion);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;


		nombreTabla_size = strlen(cc->nombreTabla) + 1;
		size_to_send = sizeof(nombreTabla_size);
		memcpy(paqueteSerializado + offset, &nombreTabla_size, size_to_send);
		offset += size_to_send;

		size_to_send = nombreTabla_size;
		memcpy(paqueteSerializado + offset, cc->nombreTabla, size_to_send);
		offset += size_to_send;

		consistencia_size = strlen(cc->consistencia) + 1;
		size_to_send = sizeof(consistencia_size);
		memcpy(paqueteSerializado + offset, &consistencia_size, size_to_send);
		offset += size_to_send;

		size_to_send = consistencia_size;
		memcpy(paqueteSerializado + offset, cc->consistencia, size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(cc->cant_part);
		memcpy(paqueteSerializado + offset, &(cc->cant_part), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(cc->tiempo_compresion);
		memcpy(paqueteSerializado + offset, &(cc->tiempo_compresion), size_to_send);
		offset += size_to_send;

		return paqueteSerializado;

		break;
	}
	case(DESCRIBE): {
		int nombreTabla_size;
		contenidoDescribe* cd;
		offset = 0;
		cd = (contenidoDescribe *) (rp->contenido);

		*total_size = sizeof(rp->accionEjecutar) + sizeof(int)
				+ (strlen(cd->nombreTabla) + 1) * sizeof(char) + sizeof(nombreTabla_size);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;


		nombreTabla_size = strlen(cd->nombreTabla) + 1;
		size_to_send = sizeof(nombreTabla_size);
		memcpy(paqueteSerializado + offset, &nombreTabla_size, size_to_send);
		offset += size_to_send;

		size_to_send = nombreTabla_size;
		memcpy(paqueteSerializado + offset, cd->nombreTabla, size_to_send);
		offset += size_to_send;

		return paqueteSerializado;

		break;
	}
	case(DROP): {
		int nombreTabla_size;
		contenidoDrop* cd;
		offset = 0;
		cd = (contenidoDrop *) (rp->contenido);

		*total_size = sizeof(rp->accionEjecutar) + sizeof(int)
				+ (strlen(cd->nombreTabla) + 1) * sizeof(char) + sizeof(nombreTabla_size);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;


		nombreTabla_size = strlen(cd->nombreTabla) + 1;
		size_to_send = sizeof(nombreTabla_size);
		memcpy(paqueteSerializado + offset, &nombreTabla_size, size_to_send);
		offset += size_to_send;

		size_to_send = nombreTabla_size;
		memcpy(paqueteSerializado + offset, cd->nombreTabla, size_to_send);
		offset += size_to_send;

		return paqueteSerializado;

		break;
	}
	case(ADD): {
		int criterio_size;
		contenidoAdd* ca;
		offset = 0;
		ca = (contenidoAdd *) (rp->contenido);

		*total_size = sizeof(rp->accionEjecutar) + sizeof(int)
				+ sizeof(ca->numMem)
				+ (strlen(ca->criterio) + 1) * sizeof(char) + sizeof(criterio_size);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(ca->numMem);
		memcpy(paqueteSerializado + offset, &(ca->numMem), size_to_send);
		offset += size_to_send;

		criterio_size = strlen(ca->criterio) + 1;
		size_to_send = sizeof(criterio_size);
		memcpy(paqueteSerializado + offset, &criterio_size, size_to_send);
		offset += size_to_send;

		size_to_send = criterio_size;
		memcpy(paqueteSerializado + offset, ca->criterio, size_to_send);
		offset += size_to_send;

		return paqueteSerializado;

		break;
	}
	case(JOURNAL): {
		offset = 0;

		*total_size = sizeof(rp->accionEjecutar);

		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);
		offset += size_to_send;

		return paqueteSerializado;

		break;
	}
	case(HANDSHAKE): {
		offset = 0;

		*total_size = sizeof(rp->accionEjecutar);
		char* paqueteSerializado = (char*) malloc(*total_size);

		size_to_send = sizeof(rp->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(rp->accionEjecutar), size_to_send);

		return paqueteSerializado;

		break;
	}
	default:
		return NULL;
	}

}


int recibirYDeserializarPaquete(int socketCliente, resultadoParser* rp) {
	int status;
	int total_size;

	int buffer_size = sizeof(int);
	char* buffer = malloc(buffer_size);

	switch(rp->accionEjecutar) {
	case(INSERT): {
		contenidoInsert* ci = malloc(sizeof(contenidoInsert));
		char* bufferTimestamp = malloc(sizeof(long));
		int valueSize;
		int nombreTablaSize;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(ci->key), buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&valueSize, buffer, buffer_size);
		if (!status) return -2;

		ci->value = malloc(sizeof(valueSize));
		status = recv(socketCliente, ci->value, valueSize, 0);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&nombreTablaSize, buffer, buffer_size);
		if (!status) return -2;

		ci->nombreTabla = malloc(nombreTablaSize);
		status = recv(socketCliente, ci->nombreTabla, nombreTablaSize, 0);
		if (!status) return -2;

		status = recv(socketCliente, bufferTimestamp, sizeof(long), 0);
		memcpy(&(ci->timestamp), bufferTimestamp, sizeof(long));
		if (!status) return -2;

		rp->contenido = ci;

		break;
	}
	case(SELECT): {
		contenidoSelect* cs = malloc(sizeof(contenidoSelect));
		int nombreTablaSize;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&nombreTablaSize, buffer, buffer_size);
		if (!status) return -2;

		cs->nombreTabla = malloc(nombreTablaSize);
		status = recv(socketCliente, cs->nombreTabla, nombreTablaSize, 0);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(cs->key), buffer, sizeof(int));
		if (!status) return -2;

		rp->contenido = cs;

		break;
	}
	case(CREATE): {
		contenidoCreate* cc = malloc(sizeof(contenidoCreate));
		int nombreTablaSize;
		int consistencia_size;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&nombreTablaSize, buffer, buffer_size);
		if (!status) return -2;

		cc->nombreTabla = malloc(nombreTablaSize);
		status = recv(socketCliente, cc->nombreTabla, nombreTablaSize, 0);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&consistencia_size, buffer, buffer_size);
		if (!status) return -2;

		cc->consistencia = malloc(consistencia_size);
		status = recv(socketCliente, cc->consistencia, consistencia_size, 0);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(cc->cant_part), buffer, sizeof(int));
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(cc->tiempo_compresion), buffer, sizeof(int));
		if (!status) return -2;

		rp->contenido = cc;

		break;
	}
	case(DESCRIBE): {
		contenidoDescribe* cd = malloc(sizeof(contenidoDescribe));
		int nombreTablaSize;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&nombreTablaSize, buffer, buffer_size);
		if (!status) return -2;

		cd->nombreTabla = malloc(nombreTablaSize);
		status = recv(socketCliente, cd->nombreTabla, nombreTablaSize, 0);
		if (!status) return -2;

		rp->contenido = cd;

		break;
	}
	case(DROP): {
		contenidoDrop* cd = malloc(sizeof(contenidoDrop));
		int nombreTablaSize;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&nombreTablaSize, buffer, buffer_size);
		if (!status) return -2;

		cd->nombreTabla = malloc(nombreTablaSize);
		status = recv(socketCliente, cd->nombreTabla, nombreTablaSize, 0);
		if (!status) return -2;

		rp->contenido = cd;

		break;
	}
	case(ADD): {
		contenidoAdd* ca = malloc(sizeof(contenidoAdd));
		int criterio_size;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(ca->numMem), buffer, sizeof(int));
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&criterio_size, buffer, buffer_size);
		if (!status) return -2;

		ca->criterio = malloc(criterio_size);
		status = recv(socketCliente, ca->criterio, criterio_size, 0);
		if (!status) return -2;

		rp->contenido = ca;

		break;
	}
	case(HANDSHAKE): {
		resultadoHandshake* rh = malloc(sizeof(resultadoHandshake));
		rp->contenido = rh;
		break;
	}
	default:
		return -1;
	}

	return 0;

}


char* serializarRespuesta(resultado* res, int* total_size) {
	int offset = 0;
	int size_to_send;

	switch(res->accionEjecutar) {
	case(INSERT): {
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion INSERT
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total (innecesario por ahora)
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(SELECT): {
		int value_size;
		registro* reg = (registro*) (res->contenido);
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size)
				+ (strlen(reg->value) + 1) * sizeof(char) + sizeof(value_size)
				+ sizeof(reg->key)
				+ sizeof(reg->timestamp);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion SELECT
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);


		value_size = strlen(reg->value) + 1;
		size_to_send = sizeof(value_size);
		memcpy(paqueteSerializado + offset, &value_size, size_to_send);
		offset += size_to_send;


		size_to_send = value_size;
		memcpy(paqueteSerializado + offset, reg->value, size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(reg->key);
		memcpy(paqueteSerializado + offset, &(reg->key), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(reg->timestamp);
		memcpy(paqueteSerializado + offset, &(reg->timestamp), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(CREATE): {
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion CREATE
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total (innecesario por ahora)
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(DESCRIBE): {
		int consistency_size;
		metadataTabla* metadata = (metadataTabla*) (res->contenido);
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size)
				+ (strlen(metadata->consistency) + 1) * sizeof(char) + sizeof(consistency_size)
				+ sizeof(metadata->compaction_time)
				+ sizeof(metadata->partitions);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion DESCRIBE
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);


		consistency_size = strlen(metadata->consistency) + 1;
		size_to_send = sizeof(consistency_size);
		memcpy(paqueteSerializado + offset, &consistency_size, size_to_send);
		offset += size_to_send;


		size_to_send = consistency_size;
		memcpy(paqueteSerializado + offset, metadata->consistency, size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(metadata->compaction_time);
		memcpy(paqueteSerializado + offset, &(metadata->compaction_time), size_to_send);
		offset += size_to_send;


		size_to_send = sizeof(metadata->partitions);
		memcpy(paqueteSerializado + offset, &(metadata->partitions), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(DROP): {
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion DROP
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total (innecesario por ahora)
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(ADD): {
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion ADD
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total (innecesario por ahora)
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(JOURNAL): {
		offset = 0;

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion JOURNAL
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total (innecesario por ahora)
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);

		return paqueteSerializado;

		break;
	}
	case(HANDSHAKE): {
		offset = 0;
		resultadoHandshake* rh = (resultadoHandshake*) (res->contenido);

		*total_size = sizeof(res->accionEjecutar) + sizeof(res->resultado) + sizeof(*total_size)
				+ sizeof(rh->tamanioValue);
		char* paqueteSerializado = (char*) malloc(*total_size);

		//Copio la accion HANDSHAKE
		size_to_send = sizeof(res->accionEjecutar);
		memcpy(paqueteSerializado + offset, &(res->accionEjecutar), size_to_send);
		offset += size_to_send;

		//Copio el tamanio total
		size_to_send = sizeof(*total_size);
		memcpy(paqueteSerializado + offset, total_size, size_to_send);
		offset += size_to_send;

		//Copio el resultado
		size_to_send = sizeof(res->resultado);
		memcpy(paqueteSerializado + offset, &(res->resultado), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(rh->tamanioValue);
		memcpy(paqueteSerializado + offset, &(rh->tamanioValue), size_to_send);

		return paqueteSerializado;

		break;
	}
	default:
		return NULL;
	}
}


int recibirYDeserializarRespuesta(int socketCliente, resultado* res) {
	int status;
	int total_size;

	int buffer_size = sizeof(int);
	char* buffer = malloc(buffer_size);

	switch(res->accionEjecutar) {
	case(INSERT): {
		char* bufferTimestamp = malloc(sizeof(long));
		int valueSize;
		int nombreTablaSize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		res->contenido = NULL;

		break;
	}
	case(SELECT): {
		registro* reg = malloc(sizeof(registro));
		char* bufferTimestamp = malloc(sizeof(long));
		int valueSize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&valueSize, buffer, buffer_size);
		if (!status) return -2;

		reg->value = malloc(valueSize);
		status = recv(socketCliente, reg->value, valueSize, 0);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(reg->key), buffer, sizeof(int));
		if (!status) return -2;

		status = recv(socketCliente, bufferTimestamp, sizeof(long), 0);
		memcpy(&(reg->timestamp), bufferTimestamp, sizeof(long));
		if (!status) return -2;

		res->contenido = reg;

		break;
	}
	case(CREATE): {
		char* bufferTimestamp = malloc(sizeof(long));
		int valueSize;
		int nombreTablaSize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		res->contenido = NULL;

		break;
	}
	case(DESCRIBE): {
		metadataTabla* metadata = malloc(sizeof(metadataTabla));
		int consistencySize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&consistencySize, buffer, buffer_size);
		if (!status) return -2;

		metadata->consistency = malloc(consistencySize);
		status = recv(socketCliente, metadata->consistency, consistencySize, 0);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(metadata->compaction_time), buffer, sizeof(int));
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(metadata->partitions), buffer, sizeof(int));
		if (!status) return -2;

		res->contenido = metadata;

		break;
	}
	case(DROP): {
		char* bufferTimestamp = malloc(sizeof(long));
		int valueSize;
		int nombreTablaSize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		res->contenido = NULL;

		break;
	}
	case(ADD): {
		char* bufferTimestamp = malloc(sizeof(long));
		int valueSize;
		int nombreTablaSize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		res->contenido = NULL;

		break;
	}
	case(HANDSHAKE): {
		resultadoHandshake* rh = malloc(sizeof(resultadoHandshake));
		int consistencySize;

		//Recibo el tamanio total
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&total_size, buffer, buffer_size);
		if (!status) return -2;

		//Recibo el resultado
		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(res->resultado), buffer, buffer_size);
		if (!status) return -2;

		status = recv(socketCliente, buffer, sizeof(int), 0);
		memcpy(&(rh->tamanioValue), buffer, sizeof(int));
		if (!status) return -2;

		res->contenido = rh;

		break;
	}
	default:
		return -1;
	}

	return 0;
}

