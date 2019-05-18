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

t_list* tabla_paginas;


#define TAM_VALUE 4
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
	void* puntero_pagina;
	int flag_modificado;

} Pagina;

typedef struct
{
	int numero_segmento;
	char nombre_tabla[NOMBRE_TABLA];
	void* puntero_tpaginas;

} Segmento;

void iniciar_programa(void);
void terminar_programa(void);
void gestionarConexion(void);
void destroy_nodo_tabla(void *);
void iniciar_tabla_paginas();

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
