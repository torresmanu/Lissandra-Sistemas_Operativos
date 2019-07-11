/*
 * Request.c
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */

#include "Request.h"

// EJECUTAR ARCHIVO LQL
Script* run(char* path){
	FILE* arch = fopen(path, "r");
	if(arch == NULL)
		perror("\nError:");
	else
		log_info(g_logger,"Abro el archivo: %s",path);

	Script* script = parsearScript(arch);

	fclose(arch);
	return script;
}

resultadoParser leerRequest(FILE* fd){
	char* linea=NULL;
	size_t tamanioLeido = 0;

	resultadoParser r;
	int leido = getline(&linea,&tamanioLeido,fd);//realoca linea y pone el tamaño leido

	r = parseConsole(linea);
	return r;
}

Script* parsearScript(FILE* fd){
	Script* script = malloc(sizeof(Script));
	script->instrucciones = list_create();
	script->pc=0;

	while(!feof(fd)){
		resultadoParser* req = malloc(sizeof(resultadoParser));
		resultadoParser aux = leerRequest(fd);

		memcpy(req,&aux,sizeof(resultadoParser));
		list_add(script->instrucciones,req);
	}
	log_info(g_logger,"Cantidad de instrucciones: %d\n",list_size(script->instrucciones));
	return script;
}

Script* crearScript(resultadoParser* r){
	Script* s;
	if(r->accionEjecutar==RUN)
	{
		char* path;
		path = string_duplicate(((contenidoRun*) r->contenido)->path);
		s = run(path);
		//free(path);
		//free(r->contenido);
	}
	else
	{
		s = malloc(sizeof(Script));
		s->instrucciones = list_create();
		s->pc = 0;
		list_add(s->instrucciones,r);
	}
	return s;
}


bool terminoScript(Script *s){
	return s->pc == list_size(s->instrucciones);
}

resultado ejecutar(Criterio* criterio, resultadoParser* request){
	Memoria* mem = masApropiada(criterio, request);
	log_info(g_logger,"Elegi memoria: %d",mem->id);
	resultado resultado = enviarRequest(mem, request);
	return resultado;
}

resultado recibir(int conexion){

	resultado res;
	accion acc;
	char* buffer = malloc(sizeof(int));
	int valueResponse;

	valueResponse = recv(conexion, buffer, sizeof(int), 0);
	memcpy(&acc, buffer, sizeof(int));

	if(valueResponse < 0)
	{
		res.resultado=ERROR;
		log_error(g_logger,"Error al recibir los datos.");
	}
	else if(valueResponse == 0)
	{
		log_error(g_logger, "Posible desconexión de memoria.");
	}
	else
	{
		res.accionEjecutar = acc;
		int status = recibirYDeserializarRespuesta(conexion, &res);

		if(status<0)
			log_error(g_logger,"No hubo respuesta de la memoria.");
		else if(res.resultado != OK)
			log_info(g_logger,res.mensaje);
		else
			log_info(g_logger,"Acción ejecutada con éxito.");

	}

	//free(buffer);
	return res;
}

resultado enviarRequest(Memoria* mem, resultadoParser* request)
{
	resultado res;
	int size;

	char* msg = serializarPaquete(request,&size);
	pthread_mutex_lock(&mConexion);
	send(mem->socket, msg, size, 0);
	res = recibir(mem->socket);
	pthread_mutex_unlock(&mConexion);

	return res;
}

resultado ejecutarScript(Script *s){

	resultadoParser *r = list_get(s->instrucciones,s->pc);
	log_info(g_logger,"PC:%d",s->pc);

	resultado estado = ejecutarRequest(r);

	(s->pc)++;
	return estado;
}

resultado ejecutarRequest(resultadoParser *r)
{
	resultado estado;

	if(usaTabla(r)){
		metadataTabla* tabla = obtenerTabla(r);

		if(tabla != NULL){
			log_info(g_logger,"Uso la tabla: %s",tabla->nombreTabla);
			log_info(g_logger,"Criterio: %d",toConsistencia(tabla->consistency)->tipo);
			Criterio* cons = toConsistencia(tabla->consistency);
			estado = ejecutar(cons,r);
		}
	}
	else{
		switch (r->accionEjecutar)
		{
			case JOURNAL:
				estado = journal();
				break;
			case METRICS:
				//metrics();
				break;
			case ADD:
			{
				contenidoAdd* contenido = (contenidoAdd *)(r->contenido);
				Memoria *mem = buscarMemoria(contenido->numMem);

				if(mem==NULL)
				{
					estado.resultado = ERROR;
					return estado;
				}

				printf("Criterio: %s\n",contenido->criterio);//OJO CRITERIO
				Criterio* cons = toConsistencia(contenido->criterio);
				add(mem,cons);
				estado.resultado = OK;
				break;
			}
			case CREATE:
			{
				contenidoCreate* cont = (contenidoCreate*)(r->contenido);
				Criterio* cons = toConsistencia(cont->consistencia);
				estado = ejecutar(cons,r);
				break;
			}
			case DESCRIBE:
			{
				estado = describe();
				break;
			}
			default:
				break;
		}
	}
	return estado;
}

void finalizarScript()	// Debe hacer un free y sacarlo de la cola
{
	free(queue_pop(exi));
}

bool usaTabla(resultadoParser* r){
	return r->accionEjecutar == SELECT || r->accionEjecutar == INSERT || r->accionEjecutar == DROP; // Describe ira en el futuro?
}
metadataTabla* obtenerTabla(resultadoParser* r){
	switch(r->accionEjecutar)
	{
		case SELECT:
		{
			contenidoSelect* c = (contenidoSelect*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case INSERT:
		{
			contenidoInsert* c = (contenidoInsert*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case DROP:
		{
			contenidoDrop* c = (contenidoDrop*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		default:
			return NULL;
	}
}

metadataTabla* buscarTabla(char* nom)
{

	bool coincideNombre(void* element)					//Subfunción de busqueda
	{
		bool e = strcmp(nom,((metadataTabla*)element)->nombreTabla) == 0;
		return e;
	}

	return list_find(tablas,coincideNombre);
}

resultado journal()
{
	resultadoParser* j = malloc(sizeof(resultadoParser));
	char* msg;
	char* buffer = malloc(sizeof(int));
	int size;
	int statusRespuesta;
	int valueResponse;
	resultado res;
	accion acc;
	j->accionEjecutar = JOURNAL;

	msg = serializarPaquete(j,&size);

	pthread_mutex_lock(&mConexion);
	for(int i = 0; i<list_size(pool);i++)
	{
		Memoria* mem = list_get(pool,i);
		send(mem->socket,msg,size,0);
		valueResponse = recv(mem->socket,buffer,sizeof(int),0);
		memcpy(&acc,buffer,sizeof(int));

		if(valueResponse < 0)
		{
			log_error(g_logger,strerror(errno));
			pthread_mutex_unlock(&mConexion);
			res.resultado = ERROR;
		}
		else if(valueResponse == 0)
		{
			log_error(g_logger,"Posiblemente la memoria se desconectó.");
			pthread_mutex_unlock(&mConexion);
			res.resultado = ERROR;
		}
		else
		{
			res.accionEjecutar = acc;
			statusRespuesta = recibirYDeserializarRespuesta(mem->socket, &res); // Recibo la lista de tablas

			pthread_mutex_unlock(&mConexion);

			if (statusRespuesta < 0)
			{
				log_error(g_logger, "Journal fallido");
				res.resultado = ERROR;
			}
			else
			{
				log_info(g_logger, "Journal realizado con exito en la memoria ID: %d", mem->id);
				res.resultado = OK;
			}
		}
	}
	return res;
}
