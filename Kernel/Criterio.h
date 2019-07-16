/*
 * Criterio.h
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#ifndef CRITERIO_H_
#define CRITERIO_H_

#include "PoolMem.h"

// MANEJO DE CRITERIOS
void iniciarCriterios();
void liberarCriterios();
void liberarMemorias();
Criterio* toConsistencia(char*);
char* mostrarConsistencia(t_consist);
Memoria* masApropiada(Criterio*, resultadoParser*);
void add(Memoria*,Criterio*);
void iniciarEstadisticas(Criterio);

//RANDOM (EC)
int memoriaRandom();

//HASHEO (SHC)
int hash(resultadoParser*);
int obtenerHash(resultadoParser*);

#endif /* CRITERIO_H_ */
