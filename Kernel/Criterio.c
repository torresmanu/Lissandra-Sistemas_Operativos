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
}

void liberarCriterios(){
	list_destroy_and_destroy_elements(sc.memorias,destroy_nodo_memoria);
	list_destroy_and_destroy_elements(shc.memorias,destroy_nodo_memoria);
	list_destroy_and_destroy_elements(ec.memorias,destroy_nodo_memoria);
}

void destroy_nodo_memoria(void * elem){
	Memoria* nodo_tabla_elem = (Memoria *) elem;
	free(nodo_tabla_elem->ipMemoria);
	free(nodo_tabla_elem->puerto);
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

Memoria* masApropiada(Criterio* c){
	Memoria* mem;
	switch(c->tipo)
	{
		case SC:
			mem = (Memoria*)list_get(sc.memorias,0);		// LOGICA DE CRITERIO STRONG CONSISTENCY
			break;
		case SHC:											// LOGICA DE CRITERIO STRONG HASH CONSISTENCY
			int indice;
			break;
		case EC:
		{
			int cantidadMemorias = rand()%list_size(ec.memorias)+1;	// Que sea aleatoria
			mem = (Memoria*)list_get(ec.memorias,cantidadMemorias); // LOGICA DE CRITERIO EVENTUAL CONSISTENCY
			break;
		}
		default:
			break;
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
