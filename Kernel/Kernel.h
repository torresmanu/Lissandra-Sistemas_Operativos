/*
 * Kernel.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include<commons/log.h>
#include<commons/config.h>
#include<commons/sockets.h>

t_log* g_logger;
t_config* g_config;

char* IP;
char* PUERTO;

void iniciar_programa(void);
void terminar_programa(void);
int enviar_mensaje(int socket_cliente);
int iniciarCliente();
void gestionarConexion();

#endif /* KERNEL_H_ */
