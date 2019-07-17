/*
 * Kernel.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <unistd.h>
#include <commons/collections/queue.h>
#include <commons/parser.h>
#include <commons/serializacion.h>
#include <commons/log.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/inotify.h>
#include "Estados.h"
#include "Request.h"

// SEMAFOROS
sem_t sNuevo; // Semáforo para el estado NEW
sem_t sListo; // Semáforo para el estado READY
sem_t sDescribe;

// MUTEX
pthread_mutex_t mNew;
pthread_mutex_t mReady;
pthread_mutex_t mExit;
pthread_mutex_t mConfiguracion;

// VARIABLES DEL CONFIG
int quantum; 					// Cantidad de Scripts en el estado EXEC
int nivelMultiprocesamiento;    // Cuantos procesos (hilos) voy a ejecutar
int nivelActual;
int metadataRefresh;
int retardoGossiping;
int sleepEjecucion;

// HILOS
pthread_t plp; 					// Planificador a largo plazo
pthread_t describeGlobal; 		// Describe global
pthread_t gossipingAutomatico;	// Gossipeo
pthread_t* executer;			// Hilos de ejecucion (actuan como estado EXEC)
pthread_t monitoreador;			// Monitoreador del config
pthread_t metricas;				// Metricas cada 30 segundos

resultado finalizar; // Variable de corte

// MONITOREADOR
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))
char pathDirectorioActual[256];


//////////// INICIALIZACION DEL KERNEL  ////////////
void iniciar_programa(void);
void terminar_programa(void);

//////////// CONSOLA Y PLANIFICADORES ////////////
void leerConsola();
void planificadorLargoPlazo();
void ejecutador();
void mandarAready(Script *s);
void mandarAexit(Script *s);
bool deboSalir(Script *s);

//////////// DESCRIBE GLOBAL ////////////
void realizarDescribeGlobal();


void agregarTabla(t_list* tablasLFS);
void destruirTabla(void* elem);


//////////// GOSSIPING ////////////
void realizarGossipingAutomatico();

//////////// MONITOREADOR ////////////
void controlConfig();
void actualizarRetardos();

//////////// METRICS ////////////
void realizarMetrics();
void mostrarMetrics(Criterio*);
void mostrarMemoryLoad(void*);
void limpiarMetricasMemoria(void*);
void limpiarMetricasCriterio(Criterio*);


#endif /* KERNEL_H_ */
