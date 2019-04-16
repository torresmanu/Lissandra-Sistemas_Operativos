/*
 * PoolMemorias.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef POOLMEMORIAS_H_
#define POOLMEMORIAS_H_


#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

t_log* g_logger;
t_config* g_config;


void iniciar_programa(void);
void terminar_programa(void);
void gestionarConexionEntrante();
int iniciarServidor();
int esperarCliente(int listenningSocket);
void* recibir_mensaje(int socketCliente);
int iniciarCliente();
int gestionarConexionSaliente();
int enviar_mensaje(int socket_cliente, char *buffer);

#endif /* POOLMEMORIAS_H_ */
