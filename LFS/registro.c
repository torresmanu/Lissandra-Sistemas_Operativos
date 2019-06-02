/*
 * registro.c
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#include "registro.h"

void parseRegistro(char* linea,registro* regAux){
	char * str_timestamp = strsep(&linea,";");
	char * str_key = strsep(&linea,";");
	regAux->value = strsep(&linea,"\n");
	regAux->key = atoi(str_key);
	regAux->timestamp = atol (str_timestamp);
}
