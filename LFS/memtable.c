/*
 * memtable.c
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#include "memtable.h"

void iniciar_memtable(){
	memtable_list = list_create();
}
void finalizar_memtable(){
	list_destroy_and_destroy_elements(memtable_list,destroy_nodo_tabla);
}

void destroy_nodo_tabla(void * elem){
	nodo_tabla* nodo_tabla_elem = (nodo_tabla *) elem;
	list_destroy_and_destroy_elements(nodo_tabla_elem->lista_registros,destroy_nodo_registro);
	free(nodo_tabla_elem);
}

void destroy_nodo_registro(void * elem){
	registro* reg = (registro*) elem;
	free(reg);
}

registro* memtable_select(char* nombreTabla, int key){
	//Itero para buscar la tabla
	for(int i = 0; i < list_size(memtable_list); i++){
		nodo_tabla* nodo = ((nodo_tabla*)list_get(memtable_list,i));
		if(strcmp(nodo->nombre_tabla,nombreTabla) == 0){
			//Dentro de la tabla itero por los registros buscando el correcto
			for(int n = 0; n < list_size(nodo->lista_registros); n++){
				registro* reg = ((registro*)list_get(nodo->lista_registros,n));
				if(reg->key == key){
					log_info(g_logger,"Registro obtenido exitosamente tabla %s key %d value %s",nombreTabla,reg->key,reg->value);
					return reg;
				}
			}
		}
	}
	log_info(g_logger,"Registro no obtenido tabla %s key %d",nombreTabla,key);
	return NULL;
}

void memtable_insert(char* nombre_tabla, registro reg){
	//Me fijo si hay un nodo con esa tabla, si hay lo obtengo
	nodo_tabla * nodo = NULL;
	for(int i = 0; i < list_size(memtable_list); i++){
		nodo_tabla* nodo_aux = ((nodo_tabla*)list_get(memtable_list,i));
		if(strcmp(nodo_aux->nombre_tabla,nombre_tabla) == 0){
			nodo = nodo_aux;
		}
	}
	//Si no lo encontre lo agrego
	if(nodo == NULL){
		nodo = malloc(sizeof(nodo_tabla));
		nodo->nombre_tabla = string_duplicate(nombre_tabla);
		nodo->lista_registros = list_create();
		list_add(memtable_list,nodo);

	}
	//Una vez que tengo el nodo itero a ver si encuentro un nodo con ese valor, si lo encuentro actualizo
	for(int i = 0; i < list_size(nodo->lista_registros); i++){
		registro* reg_aux = ((registro*)list_get(nodo->lista_registros,i));
		if(reg_aux->key == reg.key){
			if(reg_aux->timestamp < reg.timestamp){
				reg_aux->value = reg.value;
				reg_aux->timestamp = reg.timestamp;
				log_info(g_logger,"Valor memtable actualizado tabla %s key %d value %s",nombre_tabla,reg_aux->key,reg_aux->value);
				return;
			}else{
				log_info(g_logger,"Valor memtable no actualizado hay un registro más nuevo");
				return;
			}
		}
	}
	//Si no encontre el registro entonces lo creo
	registro * reg_aux = malloc(sizeof(registro));
	memcpy(reg_aux,&reg,sizeof(registro));
	list_add(nodo->lista_registros,reg_aux);
	log_info(g_logger,"Valor memtable insertado tabla %s key %d value %s",nombre_tabla,reg_aux->key,reg_aux->value);
}
