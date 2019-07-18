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

resultado ejecutar(Criterio* criterio, resultadoParser* request,char* id){

	resultado res;
	Memoria* mem = masApropiada(criterio, request);
	if(mem == NULL){
		log_info(g_logger, "No hay memorias disponibles");
		res.resultado = ERROR;

	}
	else{
		log_info(g_logger, "Elegida la memoria ID: %zu", mem->id);
		res = enviarRequest(mem, request,id);

		// Para las metricas
		if(res.resultado == OK && request->accionEjecutar == SELECT){
			(mem->selectsTotales)++;
			(criterio->amountReads)++;
		}
		else if(res.resultado == OK && request->accionEjecutar == INSERT){
			(mem->insertsTotales)++;
			(criterio->amountWrites)++;
		}
		(mem->totalOperaciones)++;

		// Si esta llena..
		if(res.resultado==FULL){
			enviarJournal(mem,id);
			res = enviarRequest(mem, request,id);
		}
		if(res.resultado == MEMORIA_CAIDA){
			enviarRequest(mem,request,id);
		}
	}
	return res;
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
	{	res.resultado = MEMORIA_CAIDA;
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

int obtenerConexion(Memoria* mem,char* id){

	int *conexion = dictionary_get(mem->conexiones,id);

	return *conexion;
}

resultado enviarRequest(Memoria* mem, resultadoParser* request,char* id)
{
	resultado res;
	int size;

	char* msg = serializarPaquete(request,&size);


	int conexion = obtenerConexion(mem,id);

	int enviado = send(conexion, msg, size, 0);
	if(enviado<=0){
		res.resultado = MEMORIA_CAIDA;
	}
	res = recibir(conexion);

	if(res.resultado == MEMORIA_CAIDA){
		pthread_mutex_lock(&(mem->mEstado));
		sacarMemoria(mem);
		pthread_mutex_unlock(&(mem->mEstado));

	}
	return res;
}

resultado ejecutarScript(Script *s,char* id){

	resultadoParser *r = list_get(s->instrucciones,s->pc);

	printf("Va por la request N°: %d\n", s->pc);
	resultado estado = ejecutarRequest(r,id);

	(s->pc)++;
	return estado;
}

void contabilizarTiempo(Criterio* c, resultadoParser* r, long tiempo)
{
	if(r->accionEjecutar == SELECT)
	{
		c->timeTotalReads += tiempo;
	}
	if(r->accionEjecutar == INSERT)
	{
		c->timeTotalWrites += tiempo;
	}
}

resultado ejecutarRequest(resultadoParser *r,char* id)
{
	resultado estado;

	if(usaTabla(r)){
		metadataTabla* tabla = obtenerTabla(r);

		if(tabla != NULL){
			log_info(g_logger,"Uso la tabla: %s",tabla->nombreTabla);
			Criterio* cons = toConsistencia(tabla->consistency);
			struct timeval te;
			if(r->accionEjecutar == SELECT || r->accionEjecutar == INSERT)
			{
				gettimeofday(&te, NULL);
				tInicio = te.tv_sec*1000LL + te.tv_usec/1000;
			}
			if(r->accionEjecutar == INSERT && ((contenidoInsert*)(r->contenido))->timestamp == 0){
				gettimeofday(&te, NULL);
				((contenidoInsert*)(r->contenido))->timestamp = te.tv_sec*1000LL + te.tv_usec/1000;
				printf("TIMESTAMP insertado %"PRIu64"\n",((contenidoInsert*)(r->contenido))->timestamp);
			}

			estado = ejecutar(cons,r,id); // EJECUTO

			if(estado.resultado == OK && (r->accionEjecutar == SELECT || r->accionEjecutar == INSERT))
			{
				gettimeofday(&te, NULL);
				tFinal = te.tv_sec*1000LL + te.tv_usec/1000;
				tTotal = tFinal - tInicio;
				log_warning(g_logger,"Tiempo requerido: %"PRIu64" ms", tTotal);
				contabilizarTiempo(cons,r, tTotal);
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
				estado = journal(id);
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

				conectarEjecutadores();

				estado.resultado = OK;
				estado.mensaje = string_duplicate("Memoria agregado al criterio exitosamente");
				break;
			}
			case CREATE:
			{
				contenidoCreate* cont = (contenidoCreate*)(r->contenido);
				Criterio* cons = toConsistencia(cont->consistencia);
				estado = ejecutar(cons,r,id);
				break;
			}
			case DESCRIBE:
			{
				contenidoDescribe* cont = r->contenido;
				estado = describe(cont->nombreTabla,id);
				if(estado.resultado == MEMORIA_CAIDA)
					ejecutarRequest(r,id);

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
			pthread_mutex_lock(&mTablas);
			metadataTabla* tabla = buscarTabla(c->nombreTabla);
			pthread_mutex_unlock(&mTablas);

			return tabla;
		}
		case INSERT:
		{
			contenidoInsert* c = (contenidoInsert*)r->contenido;

			pthread_mutex_lock(&mTablas);
			metadataTabla* tabla = buscarTabla(c->nombreTabla);
			pthread_mutex_unlock(&mTablas);

			return tabla;
		}
		case DROP:
		{
			contenidoDrop* c = (contenidoDrop*)r->contenido;
			pthread_mutex_lock(&mTablas);
			metadataTabla* tabla = buscarTabla(c->nombreTabla);
			pthread_mutex_unlock(&mTablas);

			return tabla;
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
	pthread_mutex_lock(&mTablas);
	metadataTabla* vieja = list_remove_by_condition(tablas,coincideNombre);
	list_add(tablas,tablaNueva);
	pthread_mutex_unlock(&mTablas);


	if(vieja!=NULL)
		log_info(g_logger,"Metadata de %s reemplazada con exito",tablaNueva->nombreTabla);
	else
		log_info(g_logger,"Metadata de %s agregada con exito",tablaNueva->nombreTabla);
}

void enviarJournal(void* element,char* id){

	Memoria* mem = (Memoria*)element;
	resultadoParser resParser;
	resParser.accionEjecutar=JOURNAL;
	resParser.contenido=NULL;
	int size_to_send;
	char* pi = serializarPaquete(&resParser, &size_to_send);

	pthread_mutex_lock(&(mem->mutexConex));
	int conexion = obtenerConexion(mem,id);
	pthread_mutex_unlock(&(mem->mutexConex));

	send(conexion, pi, size_to_send, 0);
	resultado res = recibir(conexion);
	res.contenido = NULL;
	free(pi);

	if(res.resultado==MEMORIA_CAIDA){
		pthread_mutex_lock(&(mem->mEstado));
		sacarMemoria(mem);
		pthread_mutex_unlock(&(mem->mEstado));
	}

	free(res.mensaje);
	if(res.contenido!=NULL)
		free(res.contenido);

}

resultado journal(char* id){

	void enviarJournal2(void* elem){
		enviarJournal(elem,id);
	}

	pthread_mutex_lock(&(sc.mutex));
	list_iterate(sc.memorias,enviarJournal2);
	pthread_mutex_unlock(&(sc.mutex));
	log_info(g_logger,"Se hizo JOURNAL en las memorias SC.");

	pthread_mutex_lock(&(shc.mutex));
	list_iterate(shc.memorias,enviarJournal2);
	pthread_mutex_unlock(&(shc.mutex));
	log_info(g_logger,"Se hizo JOURNAL en las memorias SHC.");

	pthread_mutex_lock(&(ec.mutex));
	list_iterate(ec.memorias,enviarJournal2);
	pthread_mutex_unlock(&(ec.mutex));
	log_info(g_logger,"Se hizo JOURNAL en las memorias EC.");

	resultado res;
	res.resultado=OK;
	res.mensaje="Se realiza JOURNAL en los 3 criterios";
	return res;
}
