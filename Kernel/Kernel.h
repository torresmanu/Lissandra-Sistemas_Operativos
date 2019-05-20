/*
 * Kernel.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/collections/queue.h>
#include <commons/parser.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/sockets.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_BUFFER 100

t_log* g_logger;
t_config* g_config;
char* IP;
char* PUERTO;

// Cantidad procesos en el estado EXEC
int nivelMultiprocesamiento;
int nivelActual;

// SCRIPT (Con tratamiento de procesos, por eso usaremos colas como estados)
// Estados

/*
t_queue* NEW;
t_queue* READY;
t_queue* EXEC;
t_queue* EXIT;
*/


typedef enum {NEW, READY, EXEC, EXIT} nombreEstado;

typedef struct{
	t_queue* cola;
	nombreEstado nombre;
}Estado;

Estado iniciarEstado(nombreEstado nom);
void iniciarEstados();
void finalizarEstados();

void iniciar_programa(void);
void terminar_programa(void);
int enviar_mensaje(int socket_cliente);
int iniciarCliente();
void gestionarConexion();
void iniciarEstados();

void leerConsola();
void run();
resultadoParser leerScriptLQL(FILE* fd);


#endif /* KERNEL_H_ */
