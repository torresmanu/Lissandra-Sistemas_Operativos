/*
 * PoolMemorias.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef POOLMEMORIAS_H_
#define POOLMEMORIAS_H_


#include <commons/sockets.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>

t_log* g_logger;
t_config* g_config;

t_list* tabla_segmentos;
t_list* tabla_paginas;


#define TAM_VALUE 20
#define TAM_MEMORIA_PRINCIPAL 2048
#define NOMBRE_TABLA 7

typedef struct
{
	char value[TAM_VALUE];
	int key;
	long timestamp;
} Registro;

Registro* memoria;

typedef struct
{
	int numero_pagina;
	Registro* puntero_registro;
	int flag_modificado;

} Pagina;

typedef struct
{
	int numero_segmento;
	char *nombre_tabla;//[NOMBRE_TABLA];
	t_list* puntero_tpaginas;

} Segmento;

void iniciar_programa(void);
void terminar_programa(void);
void gestionarConexion(void);
void destroy_nodo_tabla(void *);
void iniciar_tablas();

void select_t(char *nombre_tabla,int key);
int contieneRegistro(char *nombre_tabla,int key, char *value);
bool encuentraSegmento(char *ntabla,Segmento *segmento);
bool encuentraPagina(Segmento segmento,int key, char* value);

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
