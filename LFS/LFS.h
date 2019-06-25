/*
 * LFS.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef LFS_H_
#define LFS_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/sockets.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/parser.h>
#include <commons/metadata.h>
#include <commons/registro.h>
#include "memtable.h"
#include "fileSystem.h"
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PACKAGESIZE 100

int server_fd;
char* IP_FS;
char* PUERTO_FS;

t_log* g_logger;
t_config* g_config;


resultado parsear_mensaje(resultadoParser *);
resultado select_acc(char *,int);
resultado insert(char*,int,char*,long);
resultado create(char*,char*,int,int);
resultado describe(char*);
resultado drop(char*);
resultado journal();
resultado dump();
resultado handshake();

void iniciar_programa();
void terminar_programa();
void gestionarConexion(int);
int atender_clientes(void);
int esperarClienteNuevo(int);
int iniciarServidor(char*);


#endif /* LFS_H_ */
