/*
 * LFS.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef LFS_H_
#define LFS_H_

#include<commons/log.h>
#include<commons/config.h>
#include<commons/sockets.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<commons/parser.h>
#include "memtable.h"
#include "metadata.h"
#include "fileSystem.h"
#include "registro.h"




char* IP_FS;
char* PUERTO_FS;

typedef enum
{
	OK,
	SALIR,
	MENSAJE_MAL_FORMATEADO,
	ERROR
}estado;

typedef struct
{
	estado resultado;
	char* mensaje;
} resultado;

t_log* g_logger;
t_config* g_config;


resultado parsear_mensaje(char *);
resultado select_acc(char *,int);
resultado insert(char*,int,char*,long);
resultado create(char*,char*,int,int);
resultado describe(char*);
resultado drop(char*);
resultado journal();
resultado dump();

void iniciar_programa();
void terminar_programa();
void gestionarConexion(void);
int atender_clientes(void);


#endif /* LFS_H_ */
