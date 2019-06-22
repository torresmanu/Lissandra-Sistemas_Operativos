/*
 * PoolMemorias.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef POOLMEMORIAS_H_
#define POOLMEMORIAS_H_


#include<commons/sockets.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/parser.h>
#include<commons/serializacion.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

t_log* g_logger;
t_config* g_config;

t_list* tabla_segmentos;
t_list* tabla_paginas_global;

int posLibres;


#define TAM_VALUE 20
#define NOMBRE_TABLA 7
#define PACKAGESIZE 1024

//typedef struct
//{
//	char value[TAM_VALUE];
//	int key;
//	long timestamp;
//} Registro;

typedef struct
{
	char* value;
	int key;
	long timestamp;
} Registro;

char* memoria;
//Registro* memoria;
int* bitmap;
int cantidadFrames;
int serverSocket;
int tamValue;
int TAM_MEMORIA_PRINCIPAL;
int retardoJournaling;
int retardoGossiping;
int retardoMemoria;
int offset;


typedef struct
{
	int numero_pagina;
	int indice_registro;
	int flag_modificado;

} Pagina;

typedef struct
{
	int numero_segmento;
	char *nombre_tabla;//[NOMBRE_TABLA];
	t_list* puntero_tpaginas;

} Segmento;



typedef struct
{
	Pagina* pagina;
	Segmento* segmento;
}NodoTablaPaginas;

void iniciar_programa(char*);

void terminar_programa(void);
void gestionarConexionALFS(void);
void destroy_nodo_pagina(void *);
void destroy_nodo_segmento(void *);
void destroy_nodo_pagina_global(void * elem);
void iniciar_tablas();

resultado select_t(char *nombre_tabla,int key);
int contieneRegistro(char *nombre_tabla,int key, Pagina** pagina);
bool encuentraSegmento(char *ntabla,Segmento **segmento);
bool encuentraPagina(Segmento* segmento,int key, Pagina** pagina);
Registro pedirAlLFS(char* nombre_tabla, int key);
char *mandarALFS(char* accion, char* nombre_tabla, int key);
int espacioLibre();
void almacenarRegistro(char *nombre_tabla,Registro registro, int posLibre);
Segmento *agregarSegmento(char *nombre_tabla);
void agregarPagina(Registro registro, Segmento *segmento, int posLibre);
void iniciarReemplazo(char *nombre_tabla,Registro registro);
void guardarEnMemoria(Registro registro, int posLibre);
NodoTablaPaginas* paginaMenosUsada();
void removerPagina(NodoTablaPaginas *nodo);
bool memoriaFull();
bool estaModificada(void *element);
bool noEstaModificada(void *element);
void journal();
void journalConRetardo();
void gossipingConRetardo();
void consola();
resultado insert(char *nombre_tabla,int key,char *value);
void actualizarRegistro(Pagina *pagina,char *value);
resultado parsear_mensaje(char *);

void actualizarTablaGlobal(int nPagina);



char* PUERTO_M;
char* PUERTO;
char* IP_M;

/*
void gestionarConexionEntrante();
int iniciarServidor();
int esperarCliente(int listenningSocket);
void* recibir_mensaje(int socketCliente);
int iniciarCliente();
int gestionarConexionSaliente();
int enviar_mensaje(int socket_cliente, char *buffer);
*/

#endif /* POOLMEMORIAS_H_ */
