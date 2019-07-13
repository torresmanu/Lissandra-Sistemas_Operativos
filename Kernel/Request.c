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
	Script* script;
	if(arch == NULL)
	{
		log_error(g_logger, strerror(errno));
	}
	else
	{
		log_info(g_logger,"Abro el archivo: %s",path);
	}

	script = parsearScript(arch);

	fclose(arch);
	return script;
}

resultadoParser leerRequest(FILE* fd){
	char* linea=NULL;
	size_t tamanioLeido = 0;

	resultadoParser r;
	int leido = getline(&linea,&tamanioLeido,fd);//realoca linea y pone el tamaño leido

	if(linea[leido-1]=='\n')
		linea[leido-1] = '\0';

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
	log_info(g_logger,"Numero de memoria elegida:%d",mem->id);
	resultado resultado = enviarRequest(mem, request);

	// Para las metricas
	if(resultado.resultado == OK && request->accionEjecutar == SELECT){
		(mem->selectsTotales)++;
	}
	else if(resultado.resultado == OK && request->accionEjecutar == INSERT){
		(mem->insertsTotales)++;
	}

	(mem->totalOperaciones)++;

	// Si esta llena..
	if(resultado.resultado==FULL){
		enviarJournal(mem);
		resultado = enviarRequest(mem, request);
	}
//	if(resultado.resultado==EnJOURNAL){
//		log_warning(g_logger,"Vuelvo a enviar la request");
//		resultado = ejecutar(criterio, request);
//	}

	return resultado;
}

resultado recibir(int conexion){

	resultado res;
	accion acc;
	char* buffer = malloc(sizeof(acc));
	int valueResponse;

	valueResponse = recv(conexion, buffer, sizeof(acc), 0);
	memcpy(&acc, buffer, sizeof(acc));

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
			log_warning(g_logger,res.mensaje);
		else
			log_info(g_logger,"Acción ejecutada con éxito.");

	}

//	free(buffer);
	return res;
}

resultado enviarRequest(Memoria* mem, resultadoParser* request)
{
	resultado res;
	int size;

	char* msg = serializarPaquete(request,&size);

	bloquearConexion(mem);
	send(mem->socket, msg, size, 0);
	res = recibir(mem->socket);
	desbloquearConexion(mem);

	return res;
}

resultado ejecutarScript(Script *s){

	resultadoParser *r = list_get(s->instrucciones,s->pc);
	printf("Va por la request N°: %d\n", s->pc);
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
			Criterio* cons = toConsistencia(tabla->consistency);
			if(r->accionEjecutar == SELECT || r->accionEjecutar == INSERT)
			{
				tInicio = time(NULL);
			}
			estado = ejecutar(cons,r); // EJECUTO
			if(estado.resultado == OK && (r->accionEjecutar == SELECT || r->accionEjecutar == INSERT))
			{
				tFinal = time(NULL);
				tTotal = tFinal - tInicio;
				printf("TIEMPO QUE TARDO: %d", tTotal);
			}
		}
		else{
			log_error(g_logger, "No se encontró la tabla");
			estado.resultado=ERROR;
		}
	}
	else{
		switch (r->accionEjecutar)
		{
			case JOURNAL:
				estado = journal();
				break;
			case METRICS:
				estado = metrics();
				break;
			case ADD:
			{
				contenidoAdd* contenido = (contenidoAdd *)(r->contenido);
				Memoria *mem = buscarMemoria(contenido->numMem);
				if(mem==NULL) // Validacion por si no existe
				{
					estado.resultado = ERROR;
					return estado;
				}
				printf("Criterio: %s\n",contenido->criterio);//OJO CRITERIO
				Criterio* cons = toConsistencia(contenido->criterio);
				add(mem,cons);
				estado.resultado = OK;
				estado.mensaje = string_duplicate("Memoria agregado al criterio exitosamente");
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
				contenidoDescribe* cont = r->contenido;
				estado = describe(cont->nombreTabla);
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

void reemplazarMetadata(metadataTabla* tablaNueva){

	bool coincideNombre(void* element)					//Subfunción de busqueda
	{
		bool e = strcmp(tablaNueva->nombreTabla,((metadataTabla*)element)->nombreTabla) == 0;
		return e;
	}

	metadataTabla* vieja = list_remove_by_condition(tablas,coincideNombre);
	list_add(tablas,tablaNueva);

	if(vieja!=NULL)
		log_info(g_logger,"Metadata de %s reemplazada con exito",tablaNueva->nombreTabla);
	else
		log_info(g_logger,"Metadata de %s agregada con exito",tablaNueva->nombreTabla);
}

void enviarJournal(void* element){

	Memoria* mem = (Memoria*)element;
	resultadoParser resParser;
	resParser.accionEjecutar=JOURNAL;
	resParser.contenido=NULL;
	int size_to_send;
	char* pi = serializarPaquete(&resParser, &size_to_send);

	bloquearConexion(mem);
	send(mem->socket, pi, size_to_send, 0);
	resultado res = recibir(mem->socket);
	desbloquearConexion(mem);

	free(res.mensaje);
	if(res.contenido!=NULL)
		free(res.contenido);

//	free(pi);


}
resultado journal(){
	list_iterate(sc.memorias,enviarJournal);
	log_info(g_logger,"Se hizo JOURNAL en las memorias SC.");

	list_iterate(shc.memorias,enviarJournal);
	log_info(g_logger,"Se hizo JOURNAL en las memorias SHC.");

	list_iterate(ec.memorias,enviarJournal);
	log_info(g_logger,"Se hizo JOURNAL en las memorias EC.");

	resultado res;
	res.resultado=OK;
	res.mensaje="Se realiza JOURNAL en los 3 criterios";
	return res;
}
