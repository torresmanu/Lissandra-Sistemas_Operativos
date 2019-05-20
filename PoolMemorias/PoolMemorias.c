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
//	gestionarConexion();
	terminar_programa();


}

void iniciar_programa(void)
{
	Registro reg1;
	reg1.key	= 10;
	strcpy(reg1.value,"creativOS");
	reg1.timestamp = 500;

	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "LFS", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Pool Memorias");

	//Inicio las configs
	g_config = config_create("PoolMemorias.config");
	log_info(g_logger,"Configuraciones inicializadas");

	//hacer handshake con LFS y obtener tamaño de mem ppl y value

	memoria=malloc(TAM_MEMORIA_PRINCIPAL);

	memoria[0] = reg1;

	int cantidad_paginas = TAM_MEMORIA_PRINCIPAL / sizeof(Registro);

	cantidad_paginas = 1; //Solo por el hito 2

	posLibres= cantidad_paginas;

	iniciar_tablas();

	Segmento* seg_prueba = malloc(sizeof(Segmento));
	seg_prueba->numero_segmento	= 1;
	seg_prueba->nombre_tabla="Tabla1";
	seg_prueba->puntero_tpaginas = list_create();

	list_add(tabla_segmentos,seg_prueba);

	for(int i=0;i<cantidad_paginas;i++){

		Pagina* nodo=malloc(sizeof(Pagina));
		nodo->numero_pagina=i;
		nodo->puntero_registro=&(memoria[i]);
		nodo->flag_modificado=0;
		list_add(seg_prueba->puntero_tpaginas,nodo);

	}

	printf("\nSELECT TABLA1 10\n");
	select_t("Tabla1",10);

	free(seg_prueba);
}

void select_t(char *nombre_tabla,int key){
	char value[TAM_VALUE];
//	Registro *registro = malloc(sizeof(Registro));	//Pensaba hacer un registro para agrupar los datos o que el select reciba un registro
	if(contieneRegistro(nombre_tabla,key,value)){
		printf("Resultado select: %s\n",value);
	}
	else{
		printf("Algo salio mal, ya vengo, voy a hablar con el LFS\n");	//Tengo que pedirselo al LFS y agregarlo en la pagina
		Registro registro = pedirAlLFS(nombre_tabla,key);	//mejor pasar un Segmento

		if(hayEspacio()){
			almacenarRegistro(nombre_tabla,registro);
		}
		else
			iniciarReemplazo(nombre_tabla,registro);
	}
	return;
}

Registro pedirAlLFS(char* nombre_tabla, int key){
//	strcpy(value,mandarLFS("SELECT",nombre_tabla,key));

	Registro registro;
	registro.key=2;
	registro.timestamp=10;
	strcpy(registro.value,"Ale puto");

	return registro;

}

bool hayEspacio(){	//una cola con el primero libre? hay que ver lo del LRU
	return posLibres>0;
}

void almacenarRegistro(char *nombre_tabla,Registro registro){
	Segmento *segmento;
	if(!encuentraSegmento(nombre_tabla,segmento))
		segmento = agregarSegmento(nombre_tabla);
	agregarPagina(registro, segmento);
}

Segmento *agregarSegmento(char *nombre_tabla){
	//creo segmento con el ntabla
	Segmento* segmento=malloc(sizeof(Segmento));
	strcpy(segmento->nombre_tabla, nombre_tabla);
	segmento->numero_segmento=tabla_segmentos->elements_count;
	segmento->puntero_tpaginas=NULL;

	list_add(tabla_segmentos,segmento);
	return segmento;
}

void agregarPagina(Registro registro, Segmento *segmento){
	Pagina* pagina=malloc(sizeof(Pagina));
	Registro* direccion = guardarEnMemoria(registro);
	pagina->puntero_registro=direccion;
	pagina->numero_pagina=segmento->puntero_tpaginas->elements_count;
	pagina->flag_modificado=0;

	list_add(segmento->puntero_tpaginas, pagina);
}

void iniciarReemplazo(char *nombre_tabla,Registro registro){

}

Registro *guardarEnMemoria(Registro registro){
	//guardar el registro que nos mandó el lfs
	Registro *r;
	int posLibre=0; //nos falta saber como tratar posiciones libres en memoria
	memoria[posLibre]=registro;
	return memoria+posLibre;
}

int contieneRegistro(char *nombre_tabla,int key, char *value){
	Segmento segmento;

	if(encuentraSegmento(nombre_tabla,&segmento)){

		if(encuentraPagina(segmento,key,value))
			return true;
	}

	return false;
}

bool encuentraSegmento(char *ntabla,Segmento *segmento){

	bool tieneTabla(void *elemento){
		return strcmp(((Segmento *)elemento)->nombre_tabla, ntabla)==0;
	}

	memcpy(segmento,list_find(tabla_segmentos,tieneTabla),sizeof(Segmento));

	return strcmp(segmento->nombre_tabla,ntabla)==0;
}

bool encuentraPagina(Segmento segmento,int key, char* value){

	bool tieneKey(void *elemento){

		int i=(((Pagina *)elemento)->puntero_registro)->key;

		return i==key;
	}

	Pagina *paginaAux = malloc(sizeof(Pagina));
	memcpy(paginaAux,list_find(segmento.puntero_tpaginas,tieneKey),sizeof(Pagina));

	strcpy(value,paginaAux->puntero_registro->value);

	free(paginaAux);
	return paginaAux!=NULL;
}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Destruyo la tabla de segmentos
	list_destroy_and_destroy_elements(tabla_segmentos, destroy_nodo_segmento);

	//Destruyo la tabla de paginas
	list_destroy_and_destroy_elements(tabla_paginas, destroy_nodo_pagina);

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

void iniciar_tablas(){
	tabla_segmentos = list_create();
	tabla_paginas = list_create();
}

void destroy_nodo_pagina(void * elem){
	Pagina* nodo_tabla_elem = (Pagina *) elem;
	free(nodo_tabla_elem);
}


void destroy_nodo_segmento(void * elem){
	Segmento* nodo_tabla_elem = (Segmento *) elem;
	free(nodo_tabla_elem);
}
