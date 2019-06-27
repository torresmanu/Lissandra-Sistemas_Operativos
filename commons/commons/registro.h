/*
 * registro.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef COMMONS_REGISTRO_H_
#define COMMONS_REGISTRO_H_

#include "stdint.h"

typedef struct
{
	char value[200];
	uint16_t key;
	long timestamp;
} registro;


void parseRegistro(char*,registro*,int);

#endif /* COMMONS_REGISTRO_H_ */
