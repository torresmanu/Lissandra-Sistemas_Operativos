#include "PoolMemorias.h"

int main(int argc, char* argv[]) {

	iniciar_programa(argv[1]);

	pthread_t journalAutomatico;
	pthread_create(&journalAutomatico,NULL,(void*) journalConRetardo,NULL);

	pthread_t gossipingAutomatico;
	pthread_create(&gossipingAutomatico,NULL,(void*) gossipingConRetardo,NULL);

	consola();

	//ver si esta bien que este aca
	//van a hacer falta mutex para la memoria
	//pthread_join(journalAutomatico,NULL);
	//pthread_join(gossipingAutomatico,NULL);

	terminar_programa();


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

void iniciar_programa(char* path)
{
	//esta en la tabla Tabla1 para probar el select
//	Registro reg1;
//	reg1.key= 10;
//	strcpy(reg1.value,"creativOS");
//	reg1.timestamp = 500;


	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "MEM", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Pool Memorias");

	//Inicio las configs
	g_config = config_create(path);
	log_info(g_logger,"Configuraciones inicializadas");


	//Me conecto al LFS
	gestionarConexionALFS();
//	char* message="hola";
//	char* value;
//	send(serverSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
//	recv(serverSocket, value, 100, 0);						// LFS me manda el value en el buffer
//	printf("%s\n", value);


	//hacer handshake con LFS y obtener tamaño de mem ppl y value
	//tamValue = handshake();
	tamValue=20;

	retardoJournaling = config_get_int_value(g_config,"RETARDO_JOURNAL");
	retardoGossiping = config_get_int_value(g_config,"RETARDO_GOSSIPING");
	retardoMemoria = config_get_int_value(g_config,"RETARDO_MEM");

	TAM_MEMORIA_PRINCIPAL = config_get_int_value(g_config,"TAM_MEM");

	memoria=malloc(TAM_MEMORIA_PRINCIPAL);
	memset(memoria,'0',TAM_MEMORIA_PRINCIPAL);

	offset = sizeof(int)+sizeof(long)+tamValue;

	cantidadFrames = TAM_MEMORIA_PRINCIPAL / offset;


	bitmap=calloc(cantidadFrames, sizeof(int));


	//memoria[0] = reg1;
//	memcpy(memoria[0],reg1.value,tamValue);
//	memcpy(memoria[0+tamValue],reg1.key,sizeof(int));
//	memcpy(memoria[0+tamValue+sizeof(int)],reg1.timestamp,sizeof(long));
//	bitmap[0]=1;

	posLibres= cantidadFrames;

	iniciar_tablas();

//	Segmento* seg_prueba = malloc(sizeof(Segmento));
//	seg_prueba->numero_segmento	= 0;
//	seg_prueba->nombre_tabla="Tabla1";
//	seg_prueba->puntero_tpaginas = list_create();
//
//	list_add(tabla_segmentos,seg_prueba);
//
//	Pagina* nodo=malloc(sizeof(Pagina));
//	nodo->numero_pagina=0;
//	nodo->indice_registro=0;
//	nodo->flag_modificado=0;
//	list_add(seg_prueba->puntero_tpaginas,nodo);

	//esto podria ser nuestra tabla de paginas global
	/*
	for(int i=0;i<cantidadFrames;i++){

		Pagina* nodo=malloc(sizeof(Pagina));
		nodo->numero_pagina=i;
		nodo->indice_registro=i;
		nodo->flag_modificado=0;
		list_add(tabPagGlobal,nodo);

	}
	*/

	//gossiping();

}

void actualizarTablaGlobal(int nPagina){
	NodoTablaPaginas* nodo = list_remove(tabla_paginas_global,nPagina);
	list_add(tabla_paginas_global,nodo);
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


resultado select_t(char *nombre_tabla, int key){
	Pagina* pagina;

	resultado res;
//	Registro *registro = malloc(sizeof(Registro));	//Pensaba hacer un registro para agrupar los datos o que el select reciba un registro
	if(contieneRegistro(nombre_tabla,key,&pagina)){

		int posicion=(pagina->indice_registro)*offset;
//		char* value;
//		memcpy(value,&memoria[posicion],tamValue);
//		res.mensaje= strdup(value);
		sleep(retardoMemoria/1000);
		res.mensaje= strdup(&memoria[posicion]);
		res.resultado=OK;

		actualizarTablaGlobal(pagina->numero_pagina);
	}
	else{
		printf("Algo salio mal, voy a hablar con el LFS \n");	//Tengo que pedirselo al LFS y agregarlo en la pagina
		fflush(stdout);
		Registro registro = pedirAlLFS(nombre_tabla,key);	//mejor pasar un Segmento

		int posLibre= espacioLibre();
		if(posLibre>=0){
			almacenarRegistro(nombre_tabla,registro,posLibre);
		}
		else
			iniciarReemplazo(nombre_tabla,registro,0);

		printf("Resultado select: %s\n",registro.value);
		res.mensaje= string_duplicate((&registro)->value);
		res.resultado=OK;

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
	if(!encuentraSegmento(nombre_tabla,segmento))
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
	/* Aca cuando "no hay espacio"  el segmento->nombre tabla se inicia en 0x0 entonces no puedo strcpy.
	 * Lo raro es que en el caso de que si hay memoria se inicializa bien y se hace ok
	 * (Todo esto sin el malloc(10)
	 */

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

void iniciarReemplazo(char *nombre_tabla,Registro registro, int flagModificado){
	NodoTablaPaginas* nodoPagina = paginaMenosUsada();
	log_info(g_logger,"Inicio reemplazo");

	if(nodoPagina==NULL){
		journal();
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
	}
}

void consola(){
	char* mensaje;
	resultado res;
	res.resultado= OK;

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

//bool estaModificada(void *element){
//	return (((NodoTablaPaginas *)element)->pagina->flag_modificado)==1;
//}

bool memoriaFull(){
	return list_any_satisfy(tabla_paginas_global,noEstaModificada);
}

void journal(){
	log_info(g_logger,"Journaling");

	int size_to_send;
	resultadoParser resParser;
	resParser.accionEjecutar=INSERT;

	char* pi = serializarPaquete(&resParser, &size_to_send);
	send(serverSocket, pi, size_to_send, 0);


//	accion acc;
//	char* buffer = malloc(sizeof(int));
//	int valueResponse = recv(serverSocket, buffer, sizeof(int), 0);
//	memcpy(&acc, buffer, sizeof(int));
//	if(valueResponse < 0) {
//		printf("Error al recibir los datos\n");
//	} else {
//		resultado res;
//		res.accionEjecutar = acc;
//		int status = recibirYDeserializarRespuesta(serverSocket, &res);
//		if(status<0) {
//			printf("Error\n");
//		} else if(res.resultado == OK) {
//			printf("El INSERT se ejecutó correctamente\n");
//		} else {
//			printf("Hubo un error al ejecutar el INSERT\n");
//		}
//	}
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

	//Segmento* s=malloc(sizeof(Segmento));
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
			//free(s);							//COMPARACION VALGRIND V1.1
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

	if(encuentraSegmento(nombre_tabla,&segmento)){

		if(encuentraPagina(segmento,key,&pagina)){	//en vez de basura(char *) pasarle una pagina
			actualizarRegistro(pagina,value);
			actualizarTablaGlobal(pagina->numero_pagina);
		}
		else{
			if(posLibre>=0){
				agregarPagina(registro,segmento,posLibre,1);
			}
			else
				iniciarReemplazo(nombre_tabla, registro, 1);
			}
	}
	else{

		if(posLibre>=0){
			segmento=agregarSegmento(nombre_tabla);
			agregarPagina(registro,segmento,posLibre,1);
		}
		else
			iniciarReemplazo(nombre_tabla, registro, 1);
		}

	resultado res;
	char *aux = "Registro insertado exitosamente";

	res.mensaje=strdup(aux);
	res.resultado=OK;

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
	FILE *archivo = fopen ("archivoBinario.dat", "wb");
	fwrite (memoria, 1, TAM_MEMORIA_PRINCIPAL, archivo);
	fclose(archivo);

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
			res.mensaje = NULL;

			break;
		}
		case INSERT:
		{
			contenidoInsert* contenido = resParser.contenido;
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

			contenidoCreate* contCreate = resParser.contenido;
			res.mensaje=strdup(aux);
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
			res.mensaje = NULL;

			break;
		}
		case DUMP:
		{
			mandarALFS(DUMP,0,0);
			res.mensaje = NULL;
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
			resParser.contenido = malloc(0);
			res.resultado = SALIR;
			res.mensaje = NULL;
			break;
		}
		case HANDSHAKE:
		{
			char* pi = serializarPaquete(&resParser, &size_to_send);
			send(serverSocket, pi, size_to_send, 0);

			accion acc;
			char* buffer = malloc(sizeof(int));
			int valueResponse = recv(serverSocket, buffer, sizeof(int), 0);
			memcpy(&acc, buffer, sizeof(int));
			if(valueResponse < 0) {
				printf("Error al recibir los datos\n");
			} else {
				resultado res;
				res.accionEjecutar = acc;
				int status = recibirYDeserializarRespuesta(serverSocket, &res);
				if(status<0) {
					printf("Error\n");
				} else {
					printf("Recibi la respuesta del HANDSHAKE\n");
					printf("El tamaño del value es: %i\n", ((resultadoHandshake*)(res.contenido))->tamanioValue);
				}
			}
			break;
		}
		default:
		{
			res.resultado = SALIR;
			res.mensaje = malloc(0);
			break;
		}
	}
	free(resParser.contenido);
	return res;

}
