/*
 * registro.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef REGISTRO_H_
#define REGISTRO_H_

#include "stdint.h"

typedef struct
{
	char value[200];
	uint16_t key;
	long timestamp;
} registro;


void parseRegistro(char*,registro*,int);

#endif /* REGISTRO_H_ */
