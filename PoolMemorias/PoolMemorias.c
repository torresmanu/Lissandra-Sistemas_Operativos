/*
 ============================================================================
 Name        : PoolMemorias.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "PoolMemorias.h"



int main(void) {


	iniciar_programa();
	gestionarConexion();
	terminar_programa();


}

void iniciar_programa(void)
{
	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "LFS", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Pool Memorias");

	//Inicio las configs
	g_config = config_create("PoolMemorias.config");
	log_info(g_logger,"Configuraciones inicializadas");

	//hacer handshake con LFS y obtener tamaño de mem ppl y value

	memoria=malloc(TAM_MEMORIA_PRINCIPAL);
	int cantidad_paginas = TAM_MEMORIA_PRINCIPAL / sizeof(Registro);
	iniciar_tabla_paginas();

	for(int i=0;i==cantidad_paginas;i++){

		Nodo* nodo=NULL;
		nodo= malloc(sizeof(Nodo));
		nodo->numero_pagina=i;
		nodo->puntero_pagina=&memoria[i];
		nodo->flag_modificado=0;
		list_add(tabla_paginas, &nodo);

	}

}


void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Destruyo la tabla de paginas
	list_destroy_and_destroy_elements(tabla_paginas, destroy_nodo_tabla);

	//Liberar memoria
	free(memoria);

}

void gestionarConexion()
{
	int estado=1;
	char buffer[PACKAGESIZE];

	PUERTO_M = config_get_string_value(g_config,"PUERTO_FS");  // 8001
	PUERTO = config_get_string_value(g_config,"PUERTO");	   // 8000

	int socketMem_fd = iniciarServidor(PUERTO_M);										//Conecto el socket de las memorias al puerto "8001"
	int clienteKer_fd = esperarCliente(socketMem_fd,"Se conecto el Kernel!");
	int clienteMem = conectarseAlServidor("5003","Me conecte a Lissandra");             // Si harcodeo poniendo 5003 si funciona

	while(estado){

//		recibir_mensaje(clienteKer_fd,buffer,"El kernel me mando el mensaje");
		recv(clienteKer_fd, (void*) buffer, PACKAGESIZE, 0);	//Recibo mensaje del kernel

		send(clienteMem,buffer,strlen(buffer)+1,0);				//Mando a la memoria

		if(strcmp(buffer,"exit")==0)
			estado=0;
		else{
			printf( "\n%s: %s\n","El kernel me mando el mensaje", buffer);
			printf( "Tamaño: %d\n", strlen(buffer));
		}
	}

    close(clienteKer_fd);
    close(socketMem_fd);
    close(clienteMem);
}

void iniciar_tabla_paginas(){
	tabla_paginas = list_create();
}

void destroy_nodo_tabla(void * elem){
	Nodo* nodo_tabla_elem = (Nodo *) elem;
	free(nodo_tabla_elem);
}
