/*
 ============================================================================
 Name        : PoolMemorias.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 * parte principal
 * requests
 * tad bitmap(hay espacio() etc)
 */






#include "PoolMemorias.h"

int main(void) {
	resultado res;
	char* mensaje;
	res.resultado= OK;
	iniciar_programa();

	while(res.resultado != SALIR)
	{
		mensaje = readline(">");
		res = parsear_mensaje(mensaje);
		if(res.resultado == OK)
		{
			log_info(g_logger,res.mensaje);
		}
		else if(res.resultado == ERROR)
		{
			log_info(g_logger,"Ocurrio un error al ejecutar la acción");
		}
		else if(res.resultado == MENSAJE_MAL_FORMATEADO)
		{
			log_info(g_logger,"Mensaje incorrecto");
		}
		//atender_clientes();
	}


	terminar_programa();


}

void iniciar_programa(void)
{
	//esta en la tabla Tabla1 para probar el select
	Registro reg1;
	reg1.key= 10;
	strcpy(reg1.value,"creativOS");
	reg1.timestamp = 500;


	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "LFS", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Pool Memorias");

	//Inicio las configs
	g_config = config_create("PoolMemorias.config");
	log_info(g_logger,"Configuraciones inicializadas");


	//Me conecto al LFS
	gestionarConexionALFS();
//	char* message="hola";
//	char* value;
//	send(serverSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
//	recv(serverSocket, value, 100, 0);						// LFS me manda el value en el buffer
//	printf("%s\n", value);


	//hacer handshake con LFS y obtener tamaño de mem ppl y value

	memoria=malloc(TAM_MEMORIA_PRINCIPAL);


	cantidadFrames = TAM_MEMORIA_PRINCIPAL / sizeof(Registro);


	bitmap=calloc(cantidadFrames, sizeof(int));


	memoria[0] = reg1;
	bitmap[0]=1;

	posLibres= cantidadFrames;

	iniciar_tablas();

	Segmento* seg_prueba = malloc(sizeof(Segmento));
	seg_prueba->numero_segmento	= 0;
	seg_prueba->nombre_tabla="Tabla1";
	seg_prueba->puntero_tpaginas = list_create();

	list_add(tabla_segmentos,seg_prueba);


	//esto esta bien?? nose porqué esta
	for(int i=0;i<cantidadFrames;i++){

		Pagina* nodo=malloc(sizeof(Pagina));
		nodo->numero_pagina=i;
		nodo->indice_registro=i;
		nodo->flag_modificado=0;
		list_add(seg_prueba->puntero_tpaginas,nodo);

	}

//	printf("\nINSERT TABLA1 10\n");
//	insert("Tabla1",10,"hola");
//
//	printf("\nSELECT TABLA1 10\n");
//	select_t("Tabla1",10);




}

//void select_t(char *nombre_tabla,int key){  primera edicion ahora lo adapto al parcer de chaco
resultado select_t(char *nombre_tabla, int key){
	Pagina* pagina=malloc(sizeof(Pagina));

	resultado res;
//	Registro *registro = malloc(sizeof(Registro));	//Pensaba hacer un registro para agrupar los datos o que el select reciba un registro
	if(contieneRegistro(nombre_tabla,key,pagina)){
		//printf("Resultado select: %s\n",value);
		char* value=memoria[pagina->indice_registro].value;
		res.mensaje= string_duplicate(value);
		res.resultado=OK;
	}
	else{
		printf("Algo salio mal, ya vengo, voy a hablar con el LFS \n");	//Tengo que pedirselo al LFS y agregarlo en la pagina
		fflush(stdout);
		Registro registro = pedirAlLFS(nombre_tabla,key);	//mejor pasar un Segmento

		int posLibre= espacioLibre();
		if(posLibre>=0){
			almacenarRegistro(nombre_tabla,registro,posLibre);
		}
		else
			iniciarReemplazo(nombre_tabla,registro); //se cambia el value del registro cuando existe el segmento

		printf("Resultado select: %s\n",registro.value);
		res.mensaje= string_duplicate((&registro)->value);
		res.resultado=OK;

	}

	free (pagina);
	return res;
}

Registro pedirAlLFS(char* nombre_tabla, int key){

	Registro registro;
	char* valuenuevo=mandarALFS(SELECT,nombre_tabla,key);
	printf("A\n");
	//strcpy(registro.value, valuenuevo);
	printf("valueNuevo: %s\n", valuenuevo);
	char* valueAux = string_duplicate(valuenuevo);
	free(valuenuevo);
	strcpy(registro.value, valueAux);
	printf("B\n");
	registro.key=key;
	registro.timestamp=time(NULL);

	return registro;

}


char* mandarALFS(char* accion, char* nombre_tabla, int key){

	char* value=malloc(sizeof(char)*20);
	char* message=nombre_tabla;
	//message = accion nombre tabla key
	send(serverSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
	recv(serverSocket, value, 20, 0);						// LFS me manda el value en el buffer
	printf("%s\n", value);

	//free(message);
	return value;
}


int espacioLibre(){

	for(int posicion=0;posicion<cantidadFrames;posicion++){
		if(bitmap[posicion]==0){
			return posicion;
		}
	}

	return -1;

}

void almacenarRegistro(char *nombre_tabla,Registro registro, int posLibre){
	Segmento *segmento;
	if(!encuentraSegmento(nombre_tabla,segmento))
		segmento = agregarSegmento(nombre_tabla);
	agregarPagina(registro, segmento, posLibre);
}

Segmento *agregarSegmento(char *nombre_tabla){
	//creo segmento con el ntabla
	Segmento* segmento=(Segmento *)malloc(sizeof(Segmento));
	segmento->nombre_tabla = malloc(10);

//	strcpy(segmento->nombre_tabla, nombre_tabla);
	segmento->numero_segmento=tabla_segmentos->elements_count;
	segmento->puntero_tpaginas=list_create();
	//strcpy(segmento->nombre_tabla, nombre_tabla);

	segmento->nombre_tabla=nombre_tabla;

	/* Aca cuando "no hay espacio"  el segmento->nombre tabla se inicia en 0x0 entonces no puedo strcpy.
	 * Lo raro es que en el caso de que si hay memoria se inicializa bien y se hace ok
	 * (Todo esto sin el malloc(10)
	 */

	list_add(tabla_segmentos,segmento);
	return segmento;
}

void agregarPagina(Registro registro, Segmento *segmento, int posLibre){
	Pagina* pagina=malloc(sizeof(Pagina));
	guardarEnMemoria(registro, posLibre);

	pagina->indice_registro=posLibre;
	pagina->numero_pagina=segmento->puntero_tpaginas->elements_count;
	pagina->flag_modificado=0;

	list_add(segmento->puntero_tpaginas, pagina);
}

void iniciarReemplazo(char *nombre_tabla,Registro registro){
	//completar cuando veamos memoria en teoria
	Segmento *segmentoAnterior;
	double a=10;	//BOOM con un int no muestra nada y con un double anda bien magicamente (la variable no se usa en ningun lado)
	Pagina* direccionPagina = paginaMenosUsada(&segmentoAnterior);

	if(direccionPagina==NULL){
		journal();
	}
	else{
		Segmento *segmento;
		if(!encuentraSegmento(nombre_tabla,segmento))
			segmento = agregarSegmento(nombre_tabla);

		list_remove(segmentoAnterior->puntero_tpaginas,direccionPagina->numero_pagina);
		cambiarNumerosPaginas(segmentoAnterior->puntero_tpaginas);

		list_add(segmento->puntero_tpaginas,direccionPagina);
		cambiarNumerosPaginas(segmento->puntero_tpaginas);
		//int indice = direccionPagina->puntero_registro - memoria;//memoria+indice
		//int indice = (direccionPagina->indice_registro)*sizeof(Registro);
		int indice = direccionPagina->indice_registro;
		memoria[indice]=registro;

	}
}

Pagina* paginaMenosUsada(Segmento** segmento){
	//por ahora porque solo tenemos un segmento y una pagina(hito2)
	if(memoriaFull()){
		return NULL;
	}
	else{
		Segmento* s =list_get(tabla_segmentos, 0);
		memcpy(segmento,&s,sizeof(Segmento *));
		return list_get((*(segmento))->puntero_tpaginas, 0);
	}
}

bool memoriaFull(){

	bool segmentoEstaModificado(void *elemento){

		bool estaModificada(void *element){
			return (((Pagina *)element)->flag_modificado)==1;
		}

		return list_all_satisfy(((Segmento *)elemento)->puntero_tpaginas,estaModificada);
	}

	return list_all_satisfy(tabla_segmentos,segmentoEstaModificado);
}

void journal(){
	printf("Journaling\n");
}

void cambiarNumerosPaginas(t_list* listaPaginas){
	for(int i=0;i<listaPaginas->elements_count;i++){
		//queremos que los numeros de pagina sean consistentes(0,1,2,..) por ejemplo cuando sacmos una pagina y nos queda 1,2,4,5..
		//lo ibamos a hacer con list_get pero creemos que nos da una copia de la pagina y necesitamos la pagina
		Pagina *aux = list_get(listaPaginas,i);
		aux->numero_pagina = i;

		list_replace(listaPaginas,i,aux);
	}
}



void guardarEnMemoria(Registro registro, int posLibre){
	memoria[posLibre]=registro;
	bitmap[posLibre]=1;
}

int contieneRegistro(char *nombre_tabla,int key, Pagina* pagina){
	Segmento* segmento=malloc(sizeof(Segmento));


	if(encuentraSegmento(nombre_tabla,segmento)){

		if(encuentraPagina(segmento,key,pagina))
			free(segmento);
			return true;
	}
	free(segmento);
	return false;
}

bool encuentraSegmento(char *ntabla,Segmento *segmento){ 	//Me dice si ya existe un segmento de esa tabla y lo mete en la variable segmento, si no NULL

	bool tieneTabla(void *elemento){
		return strcmp(((Segmento *)elemento)->nombre_tabla, ntabla)==0;
	}

	Segmento* s=malloc(sizeof(Segmento));
	s=list_find(tabla_segmentos,tieneTabla);

	if(s==NULL){
		free(s);
		return false;
	}
	else{
		memcpy(segmento,s,sizeof(Segmento));
		free(s);							//COMPARACION VALGRIND V1.1
		return true;

		//me parece que esto no hace falta porque si llego hasta acá quiere decir que encontré el segmento
		//ademas nose porque no anda, ntabla queda con basura
		//return strcmp(segmento->nombre_tabla,ntabla)==0;
	}
}

bool encuentraPagina(Segmento* segmento,int key, Pagina* pagina){

	bool tieneKey(void *elemento){

		//int i=(((Pagina *)elemento)->puntero_registro)->key;
		int i=memoria[(((Pagina *)elemento)->indice_registro)].key;

		return i==key;
	}

	Pagina* paginaAux = malloc(sizeof(Pagina));       //COMPARACION DE VALGRIND V1.1
	paginaAux=list_find(segmento->puntero_tpaginas,tieneKey);
//	memcpy(paginaAux,,sizeof(Pagina));

	if(paginaAux==NULL){
		free(paginaAux);
		return false;
	}

	//strcpy(value,paginaAux->puntero_registro->value);
	//strcpy(value, memoria[(paginaAux->indice_registro)*sizeof(Registro)].value);
	//strcpy(value, memoria[paginaAux->indice_registro].value);
	memcpy(pagina,paginaAux,sizeof(Pagina));

	free(paginaAux);									//COMPARACION VALGRIND V1.1
	return true;
}

//void insert(char *nombre_tabla,int key,char *value){    primera version
resultado insert(char *nombre_tabla,int key,char *value){
	Segmento* segmento=malloc(sizeof(Segmento));

	Pagina* pagina=malloc(sizeof(Pagina));

	Registro registro;
	registro.timestamp=time(NULL);
	registro.key=key;
	strcpy(registro.value,value);
	int posLibre=espacioLibre();

	if(encuentraSegmento(nombre_tabla,segmento)){

		if(encuentraPagina(segmento,key,pagina)){	//en vez de basura(char *) pasarle una pagina
			actualizarRegistro(pagina,value);
		}
		else
			agregarPagina(registro,segmento, posLibre);
	}
	else{

		if(posLibre>=0){
			almacenarRegistro(nombre_tabla,registro, posLibre);
		}
		else
			iniciarReemplazo(nombre_tabla,registro);
		}

	//Devuelvo el resultado
	resultado res;
	res.mensaje="Registro insertado exitosamente";
	res.resultado=OK;

	free(pagina);
	free(segmento);

	return res;
}

//esta esta mal creo
resultado drop(char* nombre_tabla){

	resultado res;
//	Segmento* segmento;
//	int indice; // deberia ser el indice de la tabla de segmentos del segmento que encuentra
//
//	if(encuentraSegmento(nombre_tabla,segmento)){
//		Pagina* pagina = segmento->nombre_tabla;
//		list_remove_and_destroy_element(tabla_segmentos, indice, destroy_nodo_segmento);
//		bitmap[pagina->indice_registro]=0;
//		res.mensaje="Registro eliminado exitosamente";
//		res.resultado=OK;
//	}
//	else{
//
//		res.mensaje="Tabla no encontrada";
//		res.resultado=ERROR;
//
//	}
	return res;
}


void actualizarRegistro(Pagina *pagina,char *value){
	//Registro *registro=(Registro*)pagina->indice_registro;
	//strcpy(registro->value,value);
	strcpy(memoria[pagina->indice_registro].value,value);
	memoria[pagina->indice_registro].timestamp=time(NULL);

//	int indice=registro-memoria;
//
//	memoria[indice]=*registro;

	pagina->flag_modificado=1;
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

	//Liberar bitmap
	free(bitmap);

	//cierro el servidor
	close(serverSocket);


}

void gestionarConexionALFS()
{
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(config_get_string_value(g_config, "IP_LFS"), config_get_string_value(g_config, "PUERTO_LFS"), &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);	// No lo necesitamos mas



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



resultado parsear_mensaje(char* mensaje)
{
	resultado res;
	resultadoParser resParser = parseConsole(mensaje);
	int size_to_send;
	switch(resParser.accionEjecutar){
		case SELECT:
		{
			contenidoSelect* contSel;
			contSel = (contenidoSelect*)resParser.contenido;
			res = select_t(contSel->nombreTabla,contSel->key);
			break;
		}
		case DESCRIBE:
		{
			contenidoDescribe* contDes = resParser.contenido;

			//send al lfs el describe para obtener la metadata de las tablas
			mandarALFS(DESCRIBE, contDes->nombreTabla, 0);

			break;
		}
		case INSERT:
		{
			/*contenidoInsert* contenido = resParser.contenido;
			res = insert(contenido->nombreTabla,contenido->key,contenido->value);*/

			contenidoInsert* ci = (contenidoInsert*) (resParser.contenido);
			char* pi = serializarPaquete(&resParser, &size_to_send);
			send(serverSocket, pi, size_to_send, 0);

			break;

		}
		case JOURNAL:
		{
			journal();
			break;
		}
		case CREATE:
		{
			contenidoCreate* contCreate = resParser.contenido;
			res.mensaje="Se envió al LFS";
			res.resultado=OK;

			mandarALFS(CREATE, contCreate->nombreTabla, contCreate->cant_part);

			//send al lfs para que haga el create

			break;
		}
		case DROP:
		{
			contenidoDrop* contDrop = resParser.contenido;
			res = drop(contDrop->nombreTabla);

			mandarALFS(DROP, contDrop->nombreTabla,0);
			//send al lfs para que realice la opercacion necesaria

			break;
		}
		case DUMP:
		{
			mandarALFS(DUMP,0,0);
			break;
		}
		case ERROR_PARSER:
		{
			res.resultado = MENSAJE_MAL_FORMATEADO;
			res.mensaje = "";
			break;
		}
		case SALIR_CONSOLA:
		{
			res.resultado = SALIR;
			res.mensaje = "";
			terminar_programa();
			break;
		}
		default:
		{
			res.resultado = SALIR;
			res.mensaje = "";
			break;
		}
	}
	return res;

}


