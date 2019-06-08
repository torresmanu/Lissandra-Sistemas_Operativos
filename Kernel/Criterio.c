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
	//list_add(sc.memorias,criterioSC());  Directamente la inicializo con la unica memoria que puede tener y asignada por archivo
	//									   de configuración?

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
	free(nodo_tabla_elem);
}

Memoria criterioSC(){
	/* LOGICA DE CRITERIO STRONG CONSISTENCY*/
}

t_consist obtenerConsistencia();
