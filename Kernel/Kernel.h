/*
 * Kernel.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/sockets.h>

t_log* g_logger;
t_config* g_config;
char* IP;
char* PUERTO;

// SCRIPT (Con tratamiento de procesos, por eso usaremos colas como estados)

t_queue* NEW;
t_queue* READY;
t_queue* EXEC;
t_queue* EXIT;
void iniciarEstados();
void finalizarEstados();



void iniciar_programa(void);
void terminar_programa(void);
int enviar_mensaje(int socket_cliente);
int iniciarCliente();
void gestionarConexion();
void iniciarEstados();




#endif /* KERNEL_H_ */
