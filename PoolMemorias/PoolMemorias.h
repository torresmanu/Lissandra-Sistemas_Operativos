/*
 * PoolMemorias.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef POOLMEMORIAS_H_
#define POOLMEMORIAS_H_

#include "Globales.h"


#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))


void monitorearConfig();
bool iniciar_programa();
void actualizarRetardos();

void terminar_programa(void);
bool gestionarConexionALFS(void);
void destroy_nodo_pagina(void *);
void destroy_nodo_segmento(void *);
void destroy_nodo_pagina_global(void * elem);
void destroy_nodo_memoria(void * elem);
void iniciar_tablas();

resultado select_t(char *nombre_tabla,int key);
int contieneRegistro(char *nombre_tabla,int key, Pagina** pagina);
bool encuentraSegmento(char *ntabla,Segmento **segmento);
bool encuentraPagina(Segmento* segmento,int key, Pagina** pagina);
Registro* pedirAlLFS(char* nombre_tabla, int key);
resultado mandarALFS(resultadoParser resParser);
resultado recibir();
int espacioLibre();
bool handshake();
void almacenarRegistro(char *nombre_tabla,Registro registro, int posLibre);
Segmento *agregarSegmento(char *nombre_tabla);
void agregarPagina(Registro registro, Segmento *segmento, int posLibre, int valorFlag);
resultado iniciarReemplazo(char *nombre_tabla,Registro registro, int flagModificado);
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
void enviarInsert(void *element);
resultado insert(char *nombre_tabla,int key,char *value,long timestamp);
void actualizarRegistro(Pagina *pagina,char *value,long timestamp);
resultado parsear_mensaje(resultadoParser*);

void escucharConexiones();
void iniciarHiloKernel(struct sockaddr_in *cliente, socklen_t *longc, int* conexion_cliente);
void iniciarHiloMemoria(struct sockaddr_in *cliente, socklen_t *longc, int* conexion_cliente);

void escucharMemoria(int* conexion_cliente);
void escucharKernel();
resultadoParser recibirRequest(int conexion_cliente);
int iniciarServidor();
int conectarAlKernel(int conexion_servidor);
void avisarResultado(resultado res, int conexion_cliente);

void actualizarTablaGlobal(int nPagina);

void liberarSegmento(void* elemento);
void liberarPagina(void* elemento);
void corregirIndicesTablaSegmentos();
void corregirIndicesPaginasGlobal();
void drop(char* nombre_tabla);


#endif /* POOLMEMORIAS_H_ */

