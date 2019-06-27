/*
 * Estados.c
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */

#include "Estados.h"

void iniciarEstados(){ 			// Son 4 colas.
	new = queue_create();
	ready = queue_create();
	exec = queue_create();
	exi = queue_create();
}

void liberarRequest(void* elem){
	resultadoParser* nodo_elem = (resultadoParser *) elem;
	free(nodo_elem);
}

void liberarScript(void* elem){
	Script* nodo_elem = (Script *) elem;
	list_destroy_and_destroy_elements(nodo_elem->instrucciones,liberarRequest);
}

void finalizarEstados(){
	queue_clean_and_destroy_elements(new,liberarScript);
	queue_clean_and_destroy_elements(ready,liberarScript);
	queue_clean_and_destroy_elements(exec,liberarScript);
	queue_clean_and_destroy_elements(exi,liberarScript);
}
