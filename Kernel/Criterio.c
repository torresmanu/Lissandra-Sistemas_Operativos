/*
 * Criterio.c
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#include "Criterio.h"


void iniciarCriterios(){
	sc.tipo = SC;
	sc.memorias = list_create();
	//list_add(sc.memorias,criterioSC());  Directamente la inicializo con la unica memoria que puede tener y asignada por archivo de configuración?

	shc.tipo = SHC;
	shc.memorias = list_create();

	ec.tipo = EC;
	ec.memorias = list_create();
	srand(time(NULL));
}

void liberarCriterios(){
	list_destroy_and_destroy_elements(sc.memorias,destroy_nodo_memoria);
	list_destroy_and_destroy_elements(shc.memorias,destroy_nodo_memoria);
	list_destroy_and_destroy_elements(ec.memorias,destroy_nodo_memoria);
}

void liberarMemorias(){
	list_destroy(pool);
}

void destroy_nodo_memoria(void* elem){
	Memoria* nodo_tabla_elem = (Memoria*) elem;
	free(nodo_tabla_elem);
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

Memoria* masApropiada(Criterio* c, resultadoParser* r){
	Memoria* mem;
	char* aux;
	int memoriaElegida;
	switch(c->tipo)
	{
		case SC:
		{	// LOGICA DE CRITERIO STRONG CONSISTENCY
			mem = (Memoria*)list_get(sc.memorias,0);
			aux="SC";
			break;
		}
		case SHC:
		{
			// LOGICA DE CRITERIO STRONG HASH CONSISTENCY
			aux="SHC";
			if(r->accionEjecutar == SELECT || r->accionEjecutar == INSERT)
			{
				memoriaElegida = obtenerHash(r)+1;
				mem = (Memoria*)list_get(shc.memorias,memoriaElegida-1);
			}
			else
			{
				mem = (Memoria*)list_get(shc.memorias,0); // Elijo la primera y listo
			}
			break;
		}
		case EC:
		{
			// LOGICA DE CRITERIO EVENTUAL CONSISTENCY
			memoriaElegida = memoriaRandom()+1;				//(+1 porque los IDs de las memorias empiezan desde 1)
			mem = (Memoria*)list_get(ec.memorias,memoriaElegida-1);
			aux="EC";
			break;
		}
		default:
			break;
	}
	if(mem	== NULL){
		log_warning(g_logger,"No hay memorias asociadas al criterio %s",aux);
		mem = MemDescribe;
	}
	return mem;
}

////////////////////////////////////////////////////////
//Agrego la memoria en la lista de memorias del criterio
void add(Memoria *memoria,Criterio *cons)
{
	log_info(g_logger,"Entre a agregar la memoria");
	list_add(cons->memorias,memoria);
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


