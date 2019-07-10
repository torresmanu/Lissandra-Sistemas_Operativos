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
	char linea[1024];
	size_t tamanioLeido;

	resultadoParser r;
	getline(&linea,&tamanioLeido,fd); //realoca linea y pone el tamaño leido
	log_info(g_logger,"Request: %s",linea);

	r = parseConsole(linea);

	//free(linea); Este tambien rompe
	return r;
}

Script* parsearScript(FILE* fd){
	Script* script = malloc(sizeof(Script));
	script->instrucciones = list_create();
	script->pc=0;

	char linea[1024];

	while(fgets(linea,1024,fd)){
		resultadoParser* req = malloc(sizeof(resultadoParser));
		resultadoParser aux;
		aux = parseConsole(linea);

		//req->accionEjecutar = aux.accionEjecutar;
		//req->contenido = aux.contenido;

		memcpy(req,&aux,sizeof(resultadoParser));
		list_add(script->instrucciones,req);
	}
	log_info(g_logger,"Script preparado. Cantidad de instrucciones: %d",list_size(script->instrucciones));
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

status ejecutar(Criterio* criterio, resultadoParser* request){
	Memoria* mem = masApropiada(criterio, request);
	log_info(g_logger,"Elegi memoria: %d",mem->id);
	status resultado = enviarRequest(mem, request); 		// Seguramente se cambie status por una estructura Resultado dependiendo lo que devuelva
	return resultado;										// la memoria. enviarRequest está sin implementar, usa sockets.
}

resultado recibir(){

	resultado res;
	accion acc;
	char* buffer = malloc(sizeof(int));
	int valueResponse;

	valueResponse = recv(memoriaSocket, buffer, sizeof(int), 0);
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
		int status = recibirYDeserializarRespuesta(memoriaSocket, &res);

		if(status<0)
			log_error(g_logger,"No hubo respuesta de la memoria.");
		else if(res.resultado != OK)
			log_info(g_logger,res.mensaje);
		else
			log_info(g_logger,"Acción ejecutada con éxito.");

	}

	free(buffer);
	return res;
}

status enviarRequest(Memoria* mem, resultadoParser* request)
{
	status result;
	resultado res;
	int size;

	char* msg = serializarPaquete(request,&size);
	pthread_mutex_lock(&mConexion);
	send(mem->socket, msg, size, 0);
	res = recibir();
	pthread_mutex_unlock(&mConexion);

	if(res.accionEjecutar==SELECT)
		log_info(g_logger,"Value: %s",((registro*)(res.contenido))->value);
	if(res.accionEjecutar==DROP)
		log_info(g_logger,"Tabla %s eliminada con éxito", ((contenidoDrop*)res.contenido)->nombreTabla);
	if(res.accionEjecutar==CREATE)
		log_info(g_logger,"Tabla %s creada con éxito",((contenidoCreate*)res.contenido)->nombreTabla);
	if(res.accionEjecutar==INSERT)
		log_info(g_logger,"Value %s insertado con éxito", ((contenidoInsert*)res.contenido)->value);

	if(res.resultado == ERROR || res.resultado == MENSAJE_MAL_FORMATEADO)
		result = REQUEST_ERROR;
	else
		result = REQUEST_OK;

	return result;
}

status ejecutarScript(Script *s){

	resultadoParser *r = list_get(s->instrucciones,s->pc);
	log_info(g_logger,"PC:%d",s->pc);
	log_info(g_logger,"Accion:%d",r->accionEjecutar);

	status estado = ejecutarRequest(r);

	(s->pc)++;
	return estado;
}

status ejecutarRequest(resultadoParser *r){

	if(usaTabla(r)){
		metadataTabla* tabla = obtenerTabla(r);
		log_info(g_logger,"UsoTabla %s",tabla->nombreTabla);

		if(tabla != NULL){
			log_info(g_logger,"Voy a ejecutar");
			log_info(g_logger,"Criterio: %d",toConsistencia(tabla->consistency)->tipo);
			return ejecutar(toConsistencia(tabla->consistency),r);
		}
		else
			return REQUEST_ERROR; 											// HACER UN ENUM
	}
	else{
		switch (r->accionEjecutar){
			case JOURNAL:
				//journal();
				break;
			case METRICS:
				//metrics();
				break;
			case ADD:
			{
				contenidoAdd* contenido = (contenidoAdd *)(r->contenido);

				Memoria *mem = buscarMemoria(contenido->numMem);

				if(mem==NULL)
					return REQUEST_ERROR;

				printf("Criterio: %s\n",contenido->criterio);//OJO CRITERIO
				Criterio* cons = toConsistencia(contenido->criterio);
				add(mem,cons);

				break;
			}
			case CREATE:
			{
				contenidoCreate* cont = (contenidoCreate*)(r->contenido);
				ejecutar(toConsistencia(cont->consistencia),r);
				break;
			}
			case DESCRIBE:
			{
				break;
			}
			default:
				break;
		}
	return REQUEST_OK;
	}
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
		case CREATE:
		{
			contenidoCreate* c = (contenidoCreate*)r->contenido;
			return buscarTabla(c->nombreTabla);
		}
		case DESCRIBE:
		{
			contenidoDescribe* c = (contenidoDescribe*)r->contenido;
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
