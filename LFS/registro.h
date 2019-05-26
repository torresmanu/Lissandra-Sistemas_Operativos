/*
 * registro.h
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#ifndef REGISTRO_H_
#define REGISTRO_H_


typedef struct
{
	char* value;
	int key;
	long timestamp;
} registro;


void parseRegistro(char*,registro*);

#endif /* REGISTRO_H_ */
