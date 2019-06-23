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
#include <commons/serializacion.h>
#include <commons/log.h>
#include <commons/sockets.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "Criterio.h"
#include <semaphore.h>
#include <pthread.h>

//TODO para sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
int memoriaSocket;

#define MAX_BUFFER 100

t_log* g_logger;
t_config* g_config;
char* IP;
char* PUERTO;


int quantum;
// Cantidad de Scripts en el estado EXEC
int nivelMultiprocesamiento;
int nivelActual;

// SCRIPT (Con tratamiento de procesos, por eso usaremos colas como estados)
// Estados



typedef enum {NEW, READY, EXEC, EXIT} nombreEstado;
typedef enum {REQUEST_OK, REQUEST_ERROR} status;

typedef t_queue* Estado;
Estado new,ready,exec,exi;

typedef struct{
	t_list *instrucciones;
	int pc;
}Script;

typedef struct{
	Criterio* criterio;
	char* nombre;
}Tabla;

t_list* tablas;
Tabla* buscarTabla(char*);
Tabla* obtenerTabla(resultadoParser*);
bool usaTabla(resultadoParser*);

void iniciarEstado(Estado est);
void iniciarEstados();
void finalizarEstados();

//////////// SOCKETS ////////////
void iniciar_programa(void);
void terminar_programa(void);
int enviar_mensaje(int socket_cliente);
int iniciarCliente();
void gestionarConexionAMemoria();


//////////// ARCHIVOS LQL ////////////

void leerConsola();
Script* run(char*);
resultadoParser leerLineaSQL(char*);
resultadoParser leerRequest(FILE*);
Script* parsearScript(FILE*);


void planificadorLargoPlazo();
Script* crearScript(resultadoParser*);

void ejecutador();
bool terminoScript(Script *s);
void mandarAready(Script *s);
void mandarAexit(Script *s);
bool deboSalir(Script *s);
status ejecutarRequest(resultadoParser*);
status ejecutarScript(Script*);
status ejecutar(Criterio*, resultadoParser*);
status enviarRequest(Memoria*,resultadoParser*);

Criterio toConsistencia(char*);
Memoria* masApropiada(Criterio*);

//Journal

// Add
void add(Memoria*, Criterio);

#endif /* KERNEL_H_ */
