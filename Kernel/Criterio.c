/*
 * Criterio.c
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#include "Criterio.h"
#include <inttypes.h>


void iniciarCriterios(){
	sc.tipo = SC;
	sc.memorias = list_create();
	//list_add(sc.memorias,criterioSC());  Directamente la inicializo con la unica memoria que puede tener y asignada por archivo de configuración?
	iniciarEstadisticas(sc);
	pthread_mutex_init(&(sc.mutex),NULL);

	shc.tipo = SHC;
	shc.memorias = list_create();
	iniciarEstadisticas(shc);
	pthread_mutex_init(&(shc.mutex),NULL);

	ec.tipo = EC;
	ec.memorias = list_create();
	iniciarEstadisticas(ec);
	pthread_mutex_init(&(ec.mutex),NULL);


	criterios = list_create();
	list_add(criterios,&sc);
	list_add(criterios,&shc);
	list_add(criterios,&ec);
	srand(time(NULL));
}

void iniciarEstadisticas(Criterio c)
{
	c.amountReads = 0;
	c.amountTotales = 0;
	c.amountWrites = 0;
	c.timeTotalReads = 0;
	c.timeTotalWrites = 0;
}

void liberarCriterios(){
	list_destroy_and_destroy_elements(criterios,destroy_criterio);
}

void destroy_criterio(void* elem){
	Criterio* crit = elem;

	pthread_mutex_lock(&(crit->mutex));
	list_destroy_and_destroy_elements(crit->memorias,destroy_nodo_memoria);
	pthread_mutex_unlock(&(crit->mutex));

	pthread_mutex_destroy(&(crit->mutex));
}

void liberarMemorias(){
	list_destroy(pool);
}

void destroy_nodo_diccionario(void* elem){
	int* conexion = elem;
	close(*conexion);
	free(conexion);
}

void destroy_nodo_memoria(void* elem){
	Memoria* mem = (Memoria*) elem;
	free(mem->ipMemoria);
	free(mem->puerto);

	pthread_mutex_lock(&(mem->mutexConex));
	dictionary_destroy_and_destroy_elements(mem->conexiones,destroy_nodo_diccionario);
	pthread_mutex_unlock(&(mem->mutexConex));

	pthread_mutex_destroy(&(mem->mutexConex));
	pthread_mutex_destroy(&(mem->mEstado));

	free(mem);
}

Criterio* toConsistencia(char* cadena)
{
	if(strcmp(cadena, "SC") == 0)
		return &sc;
	else if(strcmp(cadena, "SHC") == 0)
		return &shc;
	else
		return &ec;
}

char* mostrarConsistencia(t_consist consist)
{
	char* tipo;
	switch(consist)
	{
		case SC:
		{
			tipo = string_duplicate("SC");
			break;
		}
		case SHC:
		{
			tipo = string_duplicate("SHC");
			break;
		}
		case EC:
		{
			tipo = string_duplicate("EC");
			break;
		}
	}
	return tipo;
}

Memoria* masApropiada(Criterio* c, resultadoParser* r){
	Memoria* mem;
	char* aux;
	int memoriaElegida;

	switch(c->tipo)
	{
		case SC:
		{	// LOGICA DE CRITERIO STRONG CONSISTENCY
			pthread_mutex_lock(&(sc.mutex));
			mem = (Memoria*)list_get(sc.memorias,0);
			pthread_mutex_unlock(&(sc.mutex));

			aux="SC";
			break;
		}
		case SHC:
		{
			// LOGICA DE CRITERIO STRONG HASH CONSISTENCY
			aux="SHC";
			if(r->accionEjecutar == SELECT || r->accionEjecutar == INSERT)
			{
				pthread_mutex_lock(&(shc.mutex));
				memoriaElegida = obtenerHash(r);
				mem = list_get(shc.memorias,memoriaElegida);
				pthread_mutex_unlock(&(shc.mutex));
			}
			else
			{
				pthread_mutex_lock(&(ec.mutex));
				mem = (Memoria*)list_get(shc.memorias,0); // Elijo la primera y listo
				pthread_mutex_unlock(&(ec.mutex));
			}
			break;
		}
		case EC:
		{
			// LOGICA DE CRITERIO EVENTUAL CONSISTENCY
			pthread_mutex_lock(&(ec.mutex));
			memoriaElegida = memoriaRandom()+1;				//(+1 porque los IDs de las memorias empiezan desde 1)
			mem = (Memoria*)list_get(ec.memorias,memoriaElegida-1);
			pthread_mutex_unlock(&(ec.mutex));
			aux="EC";
			break;
		}
		default:
			break;
	}
	if(mem	== NULL){
		log_warning(g_logger,"No hay memorias asociadas al criterio %s",aux);
		pthread_mutex_lock(&(sc.mutex));
		mem = (Memoria*)list_get(sc.memorias,0); //tendriamos que obtener cualquiera
		pthread_mutex_unlock(&(sc.mutex));

	}
	return mem;
}

////////////////////////////////////////////////////////
//Agrego la memoria en la lista de memorias del criterio
void add(Memoria *memoria,Criterio *cons)
{
	pthread_mutex_lock(&(cons->mutex));
	list_add(cons->memorias,memoria);
	pthread_mutex_unlock(&(cons->mutex));

	log_info(g_logger,"Agrege memoria N°:%d al criterio %d",memoria->id,cons->tipo);
}

/// Funcion random (EC)
int memoriaRandom()
{
	int cantMemorias = list_size(ec.memorias);
	if(cantMemorias>0)
		return rand()%cantMemorias;
	else
		return 0;
}

/// Funcion de hasheo (SHC)
int obtenerHash(resultadoParser* r)
{
	int cantMemorias = list_size(shc.memorias);
	return hash(r)%cantMemorias; 			// 19827%3 --> 0
}

int hash(resultadoParser* r)
{
	if(r->accionEjecutar == SELECT)
	{
		contenidoSelect* s = ((contenidoSelect*)r->contenido);
		return s->key;
	}
	else
	{
		contenidoInsert* i = ((contenidoInsert*)r->contenido);
		return i->key;
	}
}
