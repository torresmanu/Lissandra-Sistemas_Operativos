/*
 * memtable.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef MEMTABLE_H_
#define MEMTABLE_H_

#include<stdlib.h>
#include "LFS.h"
#include<commons/collections/list.h>
#include <commons/metadata.h>
#include <commons/registro.h>

t_list* memtable_list;
sem_t semaforoMemtable;
typedef struct
{
	char* nombre_tabla;
	t_list* lista_registros;
} nodo_tabla;

void iniciar_memtable();
void finalizar_memtable();
void destroy_nodo_tabla(void *);
void destroy_nodo_registro(void *);
registro* memtable_select(char*,int);
void memtable_insert(char*,registro);
int memtable_dump();


#endif /* MEMTABLE_H_ */
