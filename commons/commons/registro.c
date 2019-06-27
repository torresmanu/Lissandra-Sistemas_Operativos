/*
 * registro.c
 *
 *  Created on: 27 abr. 2019
 *      Author: utnso
 */

#include "registro.h"

void parseRegistro(char* linea,registro* regAux, int tamValue){
	regAux->timestamp = atol (strsep(&linea,";"));
	regAux->key = atoi(strsep(&linea,";"));
	//regAux->value = malloc(tamValue);
	strcpy(regAux->value, strsep(&linea,"\n"));

}
