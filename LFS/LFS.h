/*
 * LFS.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef LFS_H_
#define LFS_H_

#include <inttypes.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/sockets.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/parser.h>
#include <commons/metadata.h>
#include <commons/registro.h>
#include "memtable.h"
#include "fileSystemPropio.h"
#include "fileSystem.h"
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/registro.h>
#include "stdint.h"
#include <string.h>
#include <sys/inotify.h>

#define PACKAGESIZE 100
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

int server_fd;
char* IP_FS;
char* PUERTO_FS;
t_list* listaHilosCompactacion;
int tiempoDump;
int tamValue;
int tamanioBloque;
char* puntoMontaje;
char* magicNumber;
pthread_mutex_t semaforoMetadata;

typedef struct {
	char* nombreTabla;
	pthread_t threadId;
} estructuraHiloCompactacion;

t_log* g_logger;


resultado parsear_mensaje(resultadoParser *);
resultado select_acc(char *,uint16_t);
resultado insert(char*,uint16_t,char*,uint64_t);
resultado create(char*,char*,int,int);
resultado describe(char*);
resultado drop(char*);
resultado journal();
resultado dump();
resultado handshake();

int iniciar_programa();
void terminar_programa();
void gestionarConexion(int);
int atender_clientes(void);
int esperarClienteNuevo(int);
char* getStringConfig(char*);
int getIntConfig(char*);
int iniciarServidor(char*);

void crearHiloCompactacion(char*);
void hiloCompactacion(char*);
void crearHiloDump(void);
void hiloDump(void);

void monitorearConfig(void);
void actualizarRetardos(void);

#endif /* LFS_H_ */
