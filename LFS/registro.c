/*
 * registro.c
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#include "registro.h"

void parseRegistro(char* linea,registro* reg){
	char * str_timestamp = strsep(&linea,";");
	char * str_key = strsep(&linea,";");
	reg->value = strsep(&linea,"\n");
	reg->key = atoi(str_key);
	reg->timestamp = atol (str_timestamp);
}
