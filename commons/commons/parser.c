/*
 * parser.c
 *
 *  Created on: 29 abr. 2019
 *      Author: utnso
 */

#include "parser.h"

resultadoParser parseConsole(char* mensaje){
	resultadoParser resParser;
	char * accion = strsep(&mensaje," ");
	if(strcmp(accion,"SELECT") == 0){
		resParser.accionEjecutar=SELECT;
		contenidoSelect* cont = malloc(sizeof(contenidoSelect));
		cont->nombreTabla = strsep(&mensaje," ");
		char * str_key = strsep(&mensaje," ");
//		cont->key = atoi(str_key);
		cont->key = (uint16_t)strtoul(str_key, NULL, 10);
		resParser.contenido=cont;
	}
	else if(strcmp(accion,"INSERT") == 0)
	{
		resParser.accionEjecutar=INSERT;
		contenidoInsert* cont = malloc(sizeof(contenidoInsert));
		cont->nombreTabla = strsep(&mensaje," ");
		char * str_key = strsep(&mensaje," ");
//		cont->key = atoi(str_key);
		cont->key = (uint16_t)strtoul(str_key, NULL, 10);
		if(mensaje[0] == '\"'){
			strsep(&mensaje,"\"");
			cont->value = strsep(&mensaje,"\"");
			strsep(&mensaje," ");
		}else{
			cont->value = strsep(&mensaje," ");
		}

		char * timestamp = strsep(&mensaje," ");
		if(timestamp == '\0') {
			cont->timestamp = (long)time(NULL);
		} else {
			cont->timestamp = atol(timestamp);
		}
		/*
		resParser.accionEjecutar=INSERT;
		contenidoInsert* cont = malloc(sizeof(contenidoInsert));
		cont->nombreTabla = strsep(&mensaje," ");
		char * str_key = strsep(&mensaje," ");
		cont->key = atoi(str_key);
		if(string_contains(mensaje,"\"")){
			strsep(&mensaje,"\"");
			cont->value = strsep(&mensaje,"\"");
			strsep(&mensaje," ");
		}else{
			cont->value = strsep(&mensaje," ");
		}
		char * timestamp = strsep(&mensaje," ");
		if(timestamp == '\0') {
			cont->timestamp = (long)time(NULL);
		} else {
			cont->timestamp = atol(timestamp);
		}*/
		resParser.contenido=cont;
	}
	else if(strcmp(accion,"CREATE") == 0)
	{
		resParser.accionEjecutar=CREATE;
		contenidoCreate* cont = malloc(sizeof(contenidoCreate));
		cont->nombreTabla = strsep(&mensaje," ");
		cont->consistencia = strsep(&mensaje," ");
		char * cant_part = strsep(&mensaje," ");
		cont->cant_part = atoi(cant_part);
		char * tiempo_compr = strsep(&mensaje," ");
		cont->tiempo_compresion = atoi(tiempo_compr);
		resParser.contenido=cont;
	}
	else if(strcmp(accion,"DESCRIBE") == 0)
	{
		resParser.accionEjecutar=DESCRIBE;
		contenidoDescribe* cont = malloc(sizeof(contenidoDescribe));
		if(mensaje == NULL) {
			cont->nombreTabla = NULL;
		} else {
			cont->nombreTabla = strsep(&mensaje," ");
		}
		resParser.contenido = cont;
	}
	else if(strcmp(accion,"DROP") == 0)
	{
		resParser.accionEjecutar=DROP;
		contenidoDrop* cont = malloc(sizeof(contenidoDrop));
		cont->nombreTabla = strsep(&mensaje," ");
		resParser.contenido = cont;
	}
	else if(strcmp(accion,"JOURNAL") == 0)
	{
		resParser.accionEjecutar=JOURNAL;
		resParser.contenido=NULL;
	}
	else if(strcmp(accion,"DUMP") == 0)
	{
		resParser.accionEjecutar=DUMP;
	}
	else if(strcmp(accion,"RUN") == 0)
	{
		resParser.accionEjecutar=RUN;
		contenidoRun* cont = malloc(sizeof(contenidoRun));
		cont->path = strsep(&mensaje,"\n");
		resParser.contenido = cont;
	}
	else if(strcmp(accion,"METRICS") == 0)
	{
		resParser.accionEjecutar=METRICS;
	}
	else if(strcmp(accion,"ADD") == 0)
	{
		resParser.accionEjecutar=ADD;
		char * aux = strsep(&mensaje," ");

		if(strcmp(aux,"MEMORY") == 0){
			contenidoAdd* cont = malloc(sizeof(contenidoAdd));
			char * str_num = strsep(&mensaje," ");
			cont->numMem = atoi(str_num);

			char *conector = strsep(&mensaje," ");

			if(strcmp(conector,"TO") == 0){
				char* crit = strsep(&mensaje," ");

				if(criterioEsValido(crit))
					cont->criterio = crit;
				else
					resParser.accionEjecutar=ERROR_PARSER;
			}

			else
				resParser.accionEjecutar=ERROR_PARSER;


			resParser.contenido = cont;
		}else
			resParser.accionEjecutar=ERROR_PARSER;

	}
	else if(strcmp(accion,"SALIR") == 0)
	{
		resParser.accionEjecutar=SALIR_CONSOLA;
	}
	else if(strcmp(accion,"HANDSHAKE") == 0)
	{
		resParser.accionEjecutar=HANDSHAKE;
	}
	else
	{
		resParser.accionEjecutar=ERROR_PARSER;
	}
	return resParser;
}

bool criterioEsValido(char* consist)
{
	return strcmp(consist,"SC") == 0 || strcmp(consist,"SHC") == 0 || strcmp(consist,"EC") == 0;
}
