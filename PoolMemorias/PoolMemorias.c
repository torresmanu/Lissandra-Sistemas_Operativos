#include "PoolMemorias.h"

int main(int argc, char* argv[]) {

	iniciar_programa();

//	pthread_t journalAutomatico;
//	pthread_create(&journalAutomatico,NULL,(void*) journalConRetardo,NULL);

//	pthread_t gossipingAutomatico;
//	pthread_create(&gossipingAutomatico,NULL,(void*) gossipingConRetardo,NULL);


//	pthread_t conexionKernel;
//	pthread_create(&conexionKernel,NULL,(void*) escucharKernel,NULL);

	consola();

	//ver si esta bien que este aca
	//van a hacer falta mutex para la memoria
//	pthread_join(journalAutomatico,NULL);
	//pthread_join(gossipingAutomatico,NULL);
	//pthread_join(conexionKernel,NULL);

	terminar_programa();


}

void escucharKernel(){
	int conexion_servidor = iniciarServidor();
	int conexion_cliente = conectarAlKernel(conexion_servidor);
	resultado res;
	res.resultado = OK;

	while(res.resultado!=SALIR){
		resultadoParser resParser = recibirRequest(conexion_cliente);
		res = parsear_mensaje(&resParser);

		if(res.resultado == OK)
		{
			log_info(g_logger,res.mensaje);
		}
		else if(res.resultado == ERROR)
		{
			log_info(g_logger,"Ocurrio un error al ejecutar la acción");
			log_info(g_logger,res.mensaje);
		}
		else if(res.resultado == MENSAJE_MAL_FORMATEADO)
		{
			log_info(g_logger,"Mensaje incorrecto");
		}
		//atender_clientes();

		avisarResultado(res,conexion_cliente);

		if(res.mensaje!=NULL)
			free(res.mensaje);
	}

}

void avisarResultado(resultado res, int conexion_cliente){
	int size_to_send;
	char* paqueteRespuesta = serializarRespuesta(&res, &size_to_send);
	send(conexion_cliente, paqueteRespuesta, size_to_send, 0);
	free(paqueteRespuesta);
}

resultadoParser recibirRequest(int conexion_cliente){
	char* buffer2 = malloc(sizeof(int));
	accion acc;
	resultadoParser rp;
	int status;


	int valueResponse = recv(conexion_cliente, buffer2, sizeof(int), 0);
	memcpy(&acc, buffer2, sizeof(int));

	if(valueResponse < 0) { //Comenzamos a recibir datos del cliente
		//Si recv() recibe 0 el cliente ha cerrado la conexion. Si es menor que 0 ha habido algún error.
		printf("Error al recibir los datos\n");
		rp.accionEjecutar = SALIR_CONSOLA;
	} else if(valueResponse == 0) {
		printf("El cliente se desconectó\n");
		rp.accionEjecutar = SALIR_CONSOLA;
	} else {
		rp.accionEjecutar = acc;
		status = recibirYDeserializarPaquete(conexion_cliente, &rp);
		if(status<0)
			rp.accionEjecutar = SALIR_CONSOLA;
	}
	free(buffer2);
	return rp;
}

int iniciarServidor() {

	int conexion_servidor, puerto;
	struct sockaddr_in servidor;

	puerto = config_get_int_value(g_config,"PUERTO");
	conexion_servidor = socket(AF_INET, SOCK_STREAM, 0);

	bzero((char *)&servidor, sizeof(servidor));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(puerto);
	servidor.sin_addr.s_addr = INADDR_ANY;

	if(bind(conexion_servidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
		printf("Error al asociar el puerto a la conexion. Posiblemente el puerto se encuentre ocupado\n");
	    close(conexion_servidor);
	    return -1;
	}

	listen(conexion_servidor, 3);
	printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));

	return conexion_servidor;
}


int conectarAlKernel(int conexion_servidor){
	int conexion_cliente;
	struct sockaddr_in cliente;
	socklen_t longc; //Debemos declarar una variable que contendrá la longitud de la estructura

	conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc);
	if(conexion_cliente<0)
		printf("Error al aceptar trafico\n");
	else{
		longc = sizeof(cliente);
		printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	}
	return conexion_cliente;
}

void journalConRetardo(){
	while(1){
		sleep(retardoJournaling/1000);
		journal();
	}
}

void gossipingConRetardo(){
	while(1){
		sleep(retardoGossiping/1000);
		//gossiping();
	}
}

void iniciar_programa()
{


	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "MEM", 1, LOG_LEVEL_INFO);
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
	handshake();
//	tamValue=20;

	retardoJournaling = config_get_int_value(g_config,"RETARDO_JOURNAL");
	retardoGossiping = config_get_int_value(g_config,"RETARDO_GOSSIPING");
	retardoMemoria = config_get_int_value(g_config,"RETARDO_MEM");

	TAM_MEMORIA_PRINCIPAL = config_get_int_value(g_config,"TAM_MEM");

	memoria=malloc(TAM_MEMORIA_PRINCIPAL);
	memset(memoria,'0',TAM_MEMORIA_PRINCIPAL);

	offset = sizeof(int)+sizeof(long)+tamValue;

	cantidadFrames = TAM_MEMORIA_PRINCIPAL / offset;


	bitmap=calloc(cantidadFrames, sizeof(int));


	posLibres= cantidadFrames;

	iniciar_tablas();



}

void handshake(){

	resultadoParser resParser;
	resParser.accionEjecutar = HANDSHAKE;


	int size_to_send;

	char* pi = serializarPaquete(&resParser, &size_to_send);
	send(serverSocket, pi, size_to_send, 0);

	accion acc;
	char* buffer = malloc(sizeof(int));
	int valueResponse = recv(serverSocket, buffer, sizeof(int), 0);
	memcpy(&acc, buffer, sizeof(int));
	if(valueResponse < 0) {
		log_info(g_logger,"Error al recibir los datos del handshake");
	} else {
		resultado res;
		res.accionEjecutar = acc;
		int status = recibirYDeserializarRespuesta(serverSocket, &res);
		if(status<0) {
			log_info(g_logger,"Error");
		} else {
			log_info(g_logger,"Recibi la respuesta del HANDSHAKE");
			log_info(g_logger,"El tamaño del value es: %i", ((resultadoHandshake*)(res.contenido))->tamanioValue);
			tamValue=((resultadoHandshake*)(res.contenido))->tamanioValue;

		}
	}
	free(buffer);

}


void actualizarTablaGlobal(int nPagina){
	bool mismoNumero(void* elem){
		return ((NodoTablaPaginas* )elem)->pagina->numero_pagina == nPagina;
	}

	NodoTablaPaginas* nodo = list_remove_by_condition(tabla_paginas_global,mismoNumero);
	list_add(tabla_paginas_global,nodo);
}

Registro* pedirAlLFS(char* nombre_tabla, int key){

	Registro* registro;

	resultadoParser resParser;
	resParser.accionEjecutar=SELECT;
	contenidoSelect* cont = malloc(sizeof(contenidoSelect));
	cont->nombreTabla = strdup(nombre_tabla);
	cont->key = key;
	resParser.contenido=cont;

	resultado res = mandarALFS(resParser);
	registro=(Registro*)res.contenido;

	free(cont->nombreTabla);
	free(cont);
	return registro;

}


resultado mandarALFS(resultadoParser resParser){

	int size_to_send;

	char* pi = serializarPaquete(&resParser, &size_to_send);
	send(serverSocket, pi, size_to_send, 0);

	return recibir();
}

resultado recibir(){

	resultado res;
	accion acc;
	char* buffer = malloc(sizeof(int));
	int valueResponse = recv(serverSocket, buffer, sizeof(int), 0);
	memcpy(&acc, buffer, sizeof(int));
	if(valueResponse < 0) {
		res.resultado=ERROR;
		log_info(g_logger,"Error al recibir los datos");
	} else {

		res.accionEjecutar = acc;
		int status = recibirYDeserializarRespuesta(serverSocket, &res);
		if(status<0) {
			log_info(g_logger,"Error");
		} else if(res.resultado != OK) {
			log_info(g_logger,res.mensaje);
		}
	}
	free(buffer);
	return res;
}

resultado select_t(char *nombre_tabla, int key){
	Pagina* pagina;

	resultado res;
	if(contieneRegistro(nombre_tabla,key,&pagina)){

		int posicion=(pagina->indice_registro)*offset;

		sleep(retardoMemoria/1000);
		res.mensaje= strdup(&memoria[posicion]);
		res.resultado=OK;

		actualizarTablaGlobal(pagina->numero_pagina);
	}
	else{
		log_info(g_logger,"Algo salio mal, voy a hablar con el LFS");	//Tengo que pedirselo al LFS y agregarlo en la pagina

		Registro* registro = pedirAlLFS(nombre_tabla,key);	//mejor pasar un Segmento

		int posLibre= espacioLibre();
		if(posLibre>=0){
			almacenarRegistro(nombre_tabla,*registro,posLibre);
			res.resultado=OK;
		}
		else
			res = iniciarReemplazo(nombre_tabla,*registro,0);

		if(res.resultado==OK)
			res.mensaje= string_duplicate((registro)->value);

	}

	return res;
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
	if(!encuentraSegmento(nombre_tabla,&segmento))
		segmento = agregarSegmento(nombre_tabla);
	agregarPagina(registro, segmento, posLibre, 0); //le paso cero como valorFlag porque solo la usamos en el select esta funcion
}

Segmento *agregarSegmento(char *nombre_tabla){
	//creo segmento con el ntabla
	Segmento* segmento=(Segmento *)malloc(sizeof(Segmento));
	segmento->nombre_tabla = malloc(strlen(nombre_tabla)+1);

	segmento->numero_segmento=tabla_segmentos->elements_count;
	segmento->puntero_tpaginas=list_create();

//	segmento->nombre_tabla=nombre_tabla;
	strcpy(segmento->nombre_tabla,nombre_tabla);


	list_add(tabla_segmentos,segmento);
	return segmento;
}

//void agregarPagina(Registro registro, Segmento *segmento, int posLibre){
void agregarPagina(Registro registro, Segmento *segmento, int posLibre, int valorFlag){
	Pagina* pagina=malloc(sizeof(Pagina));
	guardarEnMemoria(registro, posLibre);

	pagina->indice_registro=posLibre;
	pagina->numero_pagina=tabla_paginas_global->elements_count;
	pagina->flag_modificado=valorFlag;

	list_add(segmento->puntero_tpaginas, pagina);

	NodoTablaPaginas* nodo=malloc(sizeof(NodoTablaPaginas));
	nodo->pagina=pagina;
	nodo->segmento=segmento;
	list_add(tabla_paginas_global,nodo);
}

resultado iniciarReemplazo(char *nombre_tabla,Registro registro, int flagModificado){
	NodoTablaPaginas* nodoPagina = paginaMenosUsada();
	log_info(g_logger,"Inicio reemplazo");

	resultado res;
	if(nodoPagina==NULL){
		res.resultado = ERROR;
		char *aux = "Memoria full, hace JOURNAL";
		res.mensaje=strdup(aux);
	}

	else{
		Segmento *segmento;
		if(!encuentraSegmento(nombre_tabla,&segmento))
			segmento = agregarSegmento(nombre_tabla);

		removerPagina(nodoPagina);

		list_add(segmento->puntero_tpaginas,nodoPagina->pagina);
		nodoPagina->segmento=segmento;
		list_add(tabla_paginas_global,nodoPagina);

		nodoPagina->pagina->flag_modificado=flagModificado;
		int indice = nodoPagina->pagina->indice_registro;
		guardarEnMemoria(registro,indice);
		res.resultado=OK;
	}
	return res;
}

void consola(){
	char* mensaje;
	resultado res;
	res.resultado= OK;

	while(res.resultado != SALIR)
	{
		mensaje = readline(">");

		if(mensaje)
			add_history(mensaje);

		resultadoParser resParser = parseConsole(mensaje);
		res = parsear_mensaje(&resParser);

		if(res.resultado == OK)
		{
			log_info(g_logger,res.mensaje);
		}
		else if(res.resultado == ERROR)
		{
			log_info(g_logger,"Ocurrio un error al ejecutar la acción");
			log_info(g_logger,res.mensaje);
		}
		else if(res.resultado == MENSAJE_MAL_FORMATEADO)
		{
			log_info(g_logger,"Mensaje incorrecto");
		}
		//atender_clientes();
		if(res.mensaje!=NULL)
			free(res.mensaje);
		free(mensaje);
	}

}

void removerPagina(NodoTablaPaginas *nodo){
	bool numeroPagIgual(void* element){
		return nodo->pagina->numero_pagina==((Pagina*)element)->numero_pagina;
	}
	list_remove_by_condition(nodo->segmento->puntero_tpaginas,numeroPagIgual);
}

NodoTablaPaginas* paginaMenosUsada(){

	return list_remove_by_condition(tabla_paginas_global,noEstaModificada);
}

bool noEstaModificada(void *element){
	return (((NodoTablaPaginas *)element)->pagina->flag_modificado)==0;
}

bool estaModificada(void *element){
	return (((NodoTablaPaginas *)element)->pagina->flag_modificado)==1;
}

bool memoriaFull(){
	return list_any_satisfy(tabla_paginas_global,noEstaModificada);
}

void journal(){
	log_info(g_logger,"Journaling");

	list_iterate(tabla_paginas_global,enviarInsert);

}

void enviarInsert(void *element){ //ver los casos de error


	if(estaModificada(element)){
		int indice=((NodoTablaPaginas*)element)->pagina->indice_registro;
		int size_to_send;

		resultadoParser resParser;
		resParser.accionEjecutar=INSERT;

		contenidoInsert* cont = malloc(sizeof(contenidoInsert));
		cont->nombreTabla = malloc(strlen(((NodoTablaPaginas*)element)->segmento->nombre_tabla)+1);
		strcpy(cont->nombreTabla,((NodoTablaPaginas*)element)->segmento->nombre_tabla);
		sleep(retardoMemoria/1000);
		memcpy(&cont->key,&(memoria[(indice*offset)+tamValue]),sizeof(int));
		cont->value = strdup(&memoria[indice*offset]);
		memcpy(&cont->timestamp,&(memoria[(indice*offset)+tamValue+sizeof(int)]),sizeof(long));
		resParser.contenido=cont;

		char* pi = serializarPaquete(&resParser, &size_to_send);
		send(serverSocket, pi, size_to_send, 0);
		free(cont->nombreTabla);
		free(cont->value);
		free(cont);

		((NodoTablaPaginas*)element)->pagina->flag_modificado = 0;

		resultado res = recibir();

	}
	else{
		return;
	}
}

void cambiarNumerosPaginas(t_list* listaPaginas){
	for(int i=0;i<listaPaginas->elements_count;i++){

		Pagina *aux = list_get(listaPaginas,i);
		aux->numero_pagina = i;

		list_replace(listaPaginas,i,aux);
	}
}



void guardarEnMemoria(Registro registro, int posLibre){

	sleep(retardoMemoria/1000);
	memcpy(&memoria[(posLibre*offset)],registro.value,tamValue);
	memcpy(&memoria[(posLibre*offset)+tamValue],&(registro.key),sizeof(int));
	memcpy(&memoria[(posLibre*offset)+tamValue+sizeof(int)],&(registro.timestamp),sizeof(long));
	bitmap[posLibre]=1;
}

int contieneRegistro(char *nombre_tabla,int key, Pagina** pagina){
	Segmento* segmento;

	if(encuentraSegmento(nombre_tabla,&segmento)){
		if(encuentraPagina(segmento,key,pagina))
			return true;
	}
	return false;
}

bool encuentraSegmento(char *ntabla,Segmento **segmento){ 	//Me dice si ya existe un segmento de esa tabla y lo mete en la variable segmento, si no NULL
	bool tieneTabla(void *elemento){
		return strcmp(((Segmento *)elemento)->nombre_tabla, ntabla)==0;
	}

	Segmento* s;
	if (list_is_empty(tabla_segmentos)){
	//free(s);
	return false;
	}
	else{
		s=list_find(tabla_segmentos,tieneTabla);

		if(s==NULL){
			//free(s);
			return false;
		}
		else{
			memcpy(segmento,&s,sizeof(Segmento*));
			return true;

		}
	}

}

bool encuentraPagina(Segmento* segmento,int key, Pagina** pagina){

	bool tieneKey(void *elemento){

		int posicion=(((Pagina *)elemento)->indice_registro)*offset;
		int i=0;
		sleep(retardoMemoria/1000);
		memcpy(&i,&(memoria[posicion+tamValue]),sizeof(int));
		return i==key;
	}

	Pagina* paginaAux = list_find(segmento->puntero_tpaginas,tieneKey);
//	memcpy(paginaAux,,sizeof(Pagina));

	if(paginaAux==NULL){
		return false;
	}

	memcpy(pagina,&paginaAux,sizeof(Pagina*));

	return true;
}

resultado insert(char *nombre_tabla,int key,char *value){
	Segmento* segmento;

	Pagina* pagina;

	Registro registro;
	registro.timestamp=time(NULL);
	registro.key=key;
	registro.value=value;

	int posLibre=espacioLibre();

	resultado res;

	if(encuentraSegmento(nombre_tabla,&segmento)){

		if(encuentraPagina(segmento,key,&pagina)){	//en vez de basura(char *) pasarle una pagina
			actualizarRegistro(pagina,value);
			actualizarTablaGlobal(pagina->numero_pagina);
			res.resultado=OK;
		}
		else{
			if(posLibre>=0){
				agregarPagina(registro,segmento,posLibre,1);
				res.resultado=OK;
			}
			else
				res = iniciarReemplazo(nombre_tabla, registro, 1);
			}
	}
	else{

		if(posLibre>=0){
			segmento=agregarSegmento(nombre_tabla);
			agregarPagina(registro,segmento,posLibre,1);
			res.resultado=OK;
		}
		else
			res = iniciarReemplazo(nombre_tabla, registro, 1);
		}

	if(res.resultado==OK){
		char *aux = "Registro insertado exitosamente";
		res.mensaje=strdup(aux);
	}


	return res;
}

void liberarSegmento(void* elemento){

	Segmento* segmento = (Segmento*) elemento;

	list_destroy_and_destroy_elements(segmento->puntero_tpaginas,liberarPagina);

	free(segmento);
}

void liberarPagina(void* elemento){
	Pagina* pagina = (Pagina*) elemento;

	bool mismaPagina(void* elem){
		return ((NodoTablaPaginas* )elem)->pagina == pagina;
	}

	list_remove_and_destroy_by_condition(tabla_paginas_global,mismaPagina,destroy_nodo_pagina_global); //remuevo de la global

	bitmap[pagina->indice_registro]=0;

	free(pagina);
}

void corregirIndicesTablaSegmentos(){
	for(int i=0;i<tabla_segmentos->elements_count;i++){

		Segmento *aux = list_get(tabla_segmentos,i);
		aux->numero_segmento = i;

	}
}

void corregirIndicesPaginasGlobal(){
	for(int i=0;i<tabla_paginas_global->elements_count;i++){

		NodoTablaPaginas *aux = list_get(tabla_paginas_global,i);
		aux->pagina->numero_pagina = i;

	}
}

resultado drop(char* nombre_tabla){

	resultado res;

	Segmento* segmento;

	if(encuentraSegmento(nombre_tabla,&segmento)){

		list_remove_and_destroy_element(tabla_segmentos,segmento->numero_segmento,liberarSegmento);
		corregirIndicesTablaSegmentos();
		corregirIndicesPaginasGlobal();


		char *aux = "Registro eliminado exitosamente";
		res.mensaje=strdup(aux);
		res.resultado=OK;

		//informar al LFS
	}
	else{

		char *aux = "Tabla no encontrada";
		res.mensaje=strdup(aux);
		res.resultado=ERROR;

	}
	return res;
}


void actualizarRegistro(Pagina *pagina,char *value){

	long timestamp=time(NULL);
	int posicion=(pagina->indice_registro)*offset;
	sleep(retardoMemoria/1000);
	memcpy(&(memoria[posicion]),value,tamValue);
	memcpy(&(memoria[posicion+tamValue+sizeof(int)]),&timestamp,sizeof(long));
	//memoria[posicion+tamValue+sizeof(int)]=time(NULL);


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

	//Liberar memoria
	//FILE *archivo = fopen ("archivoBinario.dat", "wb");
	//fwrite (memoria, 1, TAM_MEMORIA_PRINCIPAL, archivo);
	//fclose(archivo);

	free(memoria);

	//Liberar bitmap
	free(bitmap);

	//cierro el servidor
	close(serverSocket);

	//Destruyo tabla de paginas global
	list_destroy_and_destroy_elements(tabla_paginas_global,destroy_nodo_pagina_global);


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
	tabla_paginas_global = list_create();
}

void destroy_nodo_pagina(void * elem){
	Pagina* nodo_tabla_elem = (Pagina *) elem;
	free(nodo_tabla_elem);
}


void destroy_nodo_segmento(void * elem){
	Segmento* nodo_tabla_elem = (Segmento *) elem;
	list_destroy_and_destroy_elements(nodo_tabla_elem->puntero_tpaginas,destroy_nodo_pagina);
	free(nodo_tabla_elem);
}

void destroy_nodo_pagina_global(void * elem){
	NodoTablaPaginas* nodo = (NodoTablaPaginas *) elem;
	free(nodo);
}

resultado parsear_mensaje(resultadoParser* resParser)
{
	resultado res;
	int size_to_send;
	switch(resParser->accionEjecutar){
		case SELECT:
		{
			contenidoSelect* contSel;
			contSel = (contenidoSelect*)resParser->contenido;
			res = select_t(contSel->nombreTabla,contSel->key);
			break;
		}
		case DESCRIBE:
		{
			//send al lfs el describe para obtener la metadata de las tablas
			mandarALFS(*resParser);
			char *aux = "Se envió al LFS";

			res.mensaje=strdup(aux);
			res.resultado=OK;

			break;
		}
		case INSERT:
		{
			contenidoInsert* contenido = resParser->contenido;
			res = insert(contenido->nombreTabla,contenido->key,contenido->value);


			break;

		}
		case JOURNAL:
		{
			journal();
			res.mensaje = NULL;
			break;
		}
		case CREATE:
		{
			char *aux = "Se envió al LFS";

			res.mensaje=strdup(aux);
			res.resultado=OK;

			mandarALFS(*resParser);

			//send al lfs para que haga el create

			break;
		}
		case DROP:
		{
			contenidoDrop* contDrop = resParser->contenido;
			res = drop(contDrop->nombreTabla);

			//mandarALFS(DROP, contDrop->nombreTabla,0);
			//send al lfs para que realice la opercacion necesaria

			break;
		}
		case DUMP:
		{
			mandarALFS(*resParser);

			char *aux = "Se envió al LFS";

			res.mensaje=strdup(aux);
			res.resultado=OK;
			break;
		}
		case ERROR_PARSER:
		{
			res.resultado = MENSAJE_MAL_FORMATEADO;
			res.mensaje = NULL;
			break;
		}
		case SALIR_CONSOLA:
		{
			resParser->contenido = malloc(0);
			res.resultado = SALIR;
			res.mensaje = NULL;
			break;
		}
//		case HANDSHAKE:
//		{
//			char* pi = serializarPaquete(&resParser, &size_to_send);
//			send(serverSocket, pi, size_to_send, 0);
//
//			accion acc;
//			char* buffer = malloc(sizeof(int));
//			int valueResponse = recv(serverSocket, buffer, sizeof(int), 0);
//			memcpy(&acc, buffer, sizeof(int));
//			if(valueResponse < 0) {
//				printf("Error al recibir los datos\n");
//			} else {
//				resultado res;
//				res.accionEjecutar = acc;
//				int status = recibirYDeserializarRespuesta(serverSocket, &res);
//				if(status<0) {
//					printf("Error\n");
//				} else {
//					printf("Recibi la respuesta del HANDSHAKE\n");
//					printf("El tamaño del value es: %i\n", ((resultadoHandshake*)(res.contenido))->tamanioValue);
//				}
//			}
//			free(buffer);
//			break;
//		}
		default:
		{
			res.resultado = SALIR;
			res.mensaje = malloc(0);
			break;
		}
	}
	free(resParser->contenido);
	return res;

}
