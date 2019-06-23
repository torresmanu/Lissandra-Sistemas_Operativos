/*
 * Estados.h
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */

#ifndef ESTADOS_H_
#define ESTADOS_H_

#include <commons/collections/queue.h>
#include <commons/parser.h>
#include "stdlib.h"
#include "Request.h"

typedef enum {NEW, READY, EXEC, EXIT} nombreEstado;
typedef t_queue* Estado;
Estado new,ready,exec,exi;

// MANEJO DE ESTADOS
void iniciarEstado(Estado est);
void iniciarEstados();
void finalizarEstados();
void liberarScript(void*);
void liberarRequest(void*);

#endif /* ESTADOS_H_ */
