/*
 * registro.c
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#include "registro.h"

void parseRegistro(char* linea,registro* regAux){

	regAux->timestamp = atol (strsep(&linea,";"));
	regAux->key = atoi(strsep(&linea,";"));
	strcpy(regAux->value, strsep(&linea,"\n"));

}
