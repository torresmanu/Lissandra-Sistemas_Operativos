#include "PoolMemorias.h"

int main(int argc, char* argv[]) {

	pathConfig = argv[1];

	bool estado = iniciar_programa();
	if(!estado)
		return 0;

	/*pthread_t journalAutomatico;
	pthread_create(&journalAutomatico,NULL,(void*) journalConRetardo,NULL);

	pthread_t gossipingAutomatico;
	pthread_create(&gossipingAutomatico,NULL,(void*) gossipingConRetardo,NULL);*/

	pthread_attr_t attr;
	pthread_t threadJournal;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int errThreadJournal = pthread_create(&threadJournal, &attr, journalConRetardo, NULL);
	if(errThreadJournal != 0) {
		log_info(g_logger,"Hubo un problema al crear el thread journalConRetardo:[%s]", strerror(errThreadJournal));
	}
	//pthread_attr_destroy(&attr);

	//pthread_attr_t attr;
	pthread_t threadGossiping;

	//pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int errThreadGossiping = pthread_create(&threadGossiping, &attr, gossipingConRetardo, NULL);
	if(errThreadGossiping != 0) {
		log_info(g_logger,"Hubo un problema al crear el thread gossipingConRetardo:[%s]", strerror(errThreadGossiping));
	}
	pthread_attr_destroy(&attr);


	pthread_t conexionesEntrantes;
	pthread_create(&conexionesEntrantes,NULL,(void*) escucharConexiones,NULL);

	pthread_t monitoreador;
	pthread_create(&monitoreador,NULL,(void*) monitorearConfig,NULL);

	gossiping();

	consola();

	//ver si esta bien que este aca
	//van a hacer falta mutex para la memoria
//	pthread_join(journalAutomatico,NULL);
	//pthread_join(gossipingAutomatico,NULL);
//	pthread_join(conexionKernel,NULL);

//	pthread_join(monitoreador,NULL); //NO VA CON JOIN CREO XQ SI NO SE QUEDA ESPERANDO UN CAMBIO EN ARCHIVO


	terminar_programa();


}

void escucharConexiones(){
	int conexion_servidor = iniciarServidor();

	struct sockaddr_in cliente;
	socklen_t longc = sizeof(cliente);

	t_list* conexiones = list_create();

	while(ejecutando){
		int *conexion_cliente=malloc(sizeof(int));

		*conexion_cliente= accept(conexion_servidor, (struct sockaddr *) &cliente, &longc);
		log_info(g_logger,"Conectado con Socket:%d ",*conexion_cliente);

		if(*conexion_cliente<0) {
			log_info(g_logger,"Error al aceptar tráfico");
			free(conexion_cliente);
			break;
		} else {
			list_add(conexiones,conexion_cliente);

			int status = 0;
			uint32_t* tipoCliente = malloc(sizeof(uint32_t));
			status = recv(*conexion_cliente,tipoCliente,sizeof(uint32_t),0);

			if(status != sizeof(uint32_t))
				log_info(g_logger, "Error al recibir info del cliente");


			if(*tipoCliente==1){
				log_info(g_logger, "Nueva conexion del kernel");
				int memoryNumber = config_get_int_value(g_config, "MEMORY_NUMBER");
				send(*conexion_cliente,&memoryNumber,sizeof(memoryNumber),0);
				iniciarHiloKernel(&cliente,&longc,conexion_cliente);
			}
			else if(*tipoCliente==0){
				log_info(g_logger, "Nueva conexion de una memoria");
				iniciarHiloMemoria(&cliente,&longc,conexion_cliente);
			}
			else{
				log_info(g_logger,"Error al reconocer cliente");
			}
			free(tipoCliente);
		}

	}
	list_destroy_and_destroy_elements(conexiones,free);
}

void iniciarHiloMemoria(struct sockaddr_in *cliente, socklen_t *longc, int* conexion_cliente){
	pthread_attr_t attr;
	pthread_t thread;

	*longc = sizeof(cliente);
	log_info(g_logger,"Conectado con %s:%d", inet_ntoa(cliente->sin_addr),htons(cliente->sin_port));

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int err = pthread_create(&thread, &attr, escucharMemoria, conexion_cliente);
	if(err != 0) {
		log_info(g_logger,"[esperarClienteNuevo] Hubo un problema al crear el thread para escuchar la memoria:[%s]", strerror(err));
	}
	pthread_attr_destroy(&attr);
}

void escucharMemoria(int *conexion_cliente){
	log_info(g_logger,"Conexion cliente: %d", *conexion_cliente);
	int estado;
	do{
		estado = recibirYmandar(*conexion_cliente);
	}while(estado>0);
}

void iniciarHiloKernel(struct sockaddr_in *cliente, socklen_t *longc, int* conexion_cliente){
	pthread_attr_t attr;
	pthread_t thread;

	*longc = sizeof(cliente);
	log_info(g_logger,"Conectado con %s:%d", inet_ntoa(cliente->sin_addr),htons(cliente->sin_port));

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int err = pthread_create(&thread, &attr, escucharKernel, conexion_cliente);
	if(err != 0) {
		log_info(g_logger,"[esperarClienteNuevo] Hubo un problema al crear el thread para escuhcar al kernel:[%s]", strerror(err));
	}
	pthread_attr_destroy(&attr);
}

void escucharKernel(int* conexion_cliente){
//	int conexion_servidor = iniciarServidor();
//	int conexion_cliente = conectarAlKernel(conexion_servidor);
	resultado res;
	res.resultado = OK;

	while(res.resultado!=SALIR){
		resultadoParser resParser = recibirRequest(*conexion_cliente);
		if(estaHaciendoJournal){
			res.resultado=EnJOURNAL;
			res.mensaje = NULL;
		}
		else
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
		else if(res.resultado == EnJOURNAL)
		{
			log_info(g_logger,"Se esta haciendo Journaling, ingrese la request mas tarde");
		}
		else if(res.resultado == SALIR)
		{
			log_info(g_logger,"El kernel se desconecto");
		}

		avisarResultado(res,*conexion_cliente);

		if(res.mensaje!=NULL)
			free(res.mensaje);
	}

}

void avisarResultado(resultado res, int conexion_cliente){
	if(res.resultado==ENVIADO || res.resultado==SALIR)
		return;
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
		log_info(g_logger,"Error al recibir los datos");
		rp.accionEjecutar = SALIR_CONSOLA;
	} else if(valueResponse == 0) {
		rp.accionEjecutar = SALIR_CONSOLA;
	} else {
		rp.accionEjecutar = acc;

		if(rp.accionEjecutar == GOSSIPING){
			int socket = conexion_cliente;
			rp.contenido = malloc(sizeof(int));
			memcpy(rp.contenido,&socket,sizeof(int));

			free(buffer2);
			return rp;
		}

		status = recibirYDeserializarPaquete(conexion_cliente, &rp);
		log_info(g_logger,"La accion es:%d", rp.accionEjecutar);
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
		log_info(g_logger,"Error al asociar el puerto a la conexion. Posiblemente el puerto se encuentre ocupado");
	    close(conexion_servidor);
	    return -1;
	}

	listen(conexion_servidor, 3);
	log_info(g_logger,"A la escucha en el puerto %d", ntohs(servidor.sin_port));

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
	while(ejecutando){
		sleep(retardoJournaling/1000);
		journal();
	}
}

void gossipingConRetardo(){
	while(ejecutando){
		sleep(retardoGossiping/1000);

		gossiping();
	}
}


void actualizarRetardos(){

	config_destroy(g_config);

	g_config = config_create(pathConfig);

	retardoJournaling = config_get_int_value(g_config,"RETARDO_JOURNAL");

	retardoGossiping = config_get_int_value(g_config,"RETARDO_GOSSIPING");

	retardoMemoria = config_get_int_value(g_config,"RETARDO_MEM");
}

void monitorearConfig() {
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, pathDirectorio,IN_MODIFY);

    while(ejecutando){
    	i=0;
        length = read(fd, buffer, BUF_LEN);

        if (length < 0) {
            perror("read");
        }
        if(length == 0){
        	printf("no lei nada\n");
        }

        while (i < length) {
            struct inotify_event *event =
                (struct inotify_event *) &buffer[i];
            if (event->len) {
                if (event->mask & IN_MODIFY) {
                	if(strcmp(pathConfig,event->name)==0){
                		log_info(g_logger,"El archivo %s fue modificado.", event->name);
                		actualizarRetardos();
                	}
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);

}

bool iniciar_programa()
{

	pthread_mutex_init(&mMemPrincipal,NULL);
	pthread_mutex_init(&mTabSeg,NULL);
	pthread_mutex_init(&mTabPagGlobal,NULL);
	pthread_mutex_init(&mMemoriasConocidas,NULL);
	pthread_mutex_init(&mBitmap,NULL);



	estaHaciendoJournal = false;
	ejecutando = true;

	getcwd(pathDirectorio, sizeof(pathDirectorio));

	//Inicio el logger
	g_logger = log_create("PoolMemorias.log", "MEM", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion Pool Memorias");

	//Inicio las configs
	g_config = config_create(pathConfig);
	log_info(g_logger,"Configuraciones inicializadas");

	//Me conecto al LFS
	bool estado = gestionarConexionALFS();
	if(!estado){
		log_info(g_logger,"Error al conectarse al LFS");
		return false;
	}


	//hacer handshake con LFS y obtener tamaño de mem ppl y value
	estado = handshake();
	if(!estado)
		return false;

	retardoJournaling = config_get_int_value(g_config,"RETARDO_JOURNAL");
	retardoGossiping = config_get_int_value(g_config,"RETARDO_GOSSIPING");
	retardoMemoria = config_get_int_value(g_config,"RETARDO_MEM");
	retardoLFS = config_get_int_value(g_config,"RETARDO_FS");


	TAM_MEMORIA_PRINCIPAL = config_get_int_value(g_config,"TAM_MEM");

	memoria=malloc(TAM_MEMORIA_PRINCIPAL);

	if(memoria==NULL){
		log_info(g_logger,"Error al inicializar la memoria principal");
		return false;
	}

	offset = sizeof(uint16_t)+sizeof(long)+tamValue;

	cantidadFrames = TAM_MEMORIA_PRINCIPAL / offset;


	bitmap=calloc(cantidadFrames, sizeof(int));


	posLibres= cantidadFrames;

	memoriasConocidas = list_create();

	yo = malloc(sizeof(Memoria));

	yo->ip = config_get_string_value(g_config,"IP_PROPIA");;
	yo->puerto=config_get_string_value(g_config,"PUERTO");
	yo->numero = config_get_int_value(g_config,"MEMORY_NUMBER");
	yo->socket=-1;

	iniciarTablaSeeds();

	list_add(memoriasConocidas,yo);

	iniciar_tablas();

	return true;
}

void iniciarTablaSeeds(){
	int i=0;

	memoriasSeeds = list_create();
	char** ips = config_get_array_value(g_config,"IP_SEEDS");
	char** puertos = config_get_array_value(g_config,"PUERTO_SEEDS");

	while(ips[i]!=NULL){
		Memoria* mem = malloc(sizeof(Memoria));
		mem->ip = ips[i];
		mem->puerto = puertos[i];
		mem->numero = -1;
		mem->socket = -1;
		conectarMemoria(mem);

		list_add(memoriasSeeds,mem);

		i++;
	}
}




bool handshake(){
	bool estado;
	resultadoParser resParser;
	resParser.accionEjecutar = HANDSHAKE;


	int size_to_send;

	sleep(retardoLFS/1000);
	char* pi = serializarPaquete(&resParser, &size_to_send);
	send(serverSocket, pi, size_to_send, 0);

	accion acc;
	char* buffer = malloc(sizeof(int));
	int valueResponse = recv(serverSocket, buffer, sizeof(int), 0);
	memcpy(&acc, buffer, sizeof(int));
	if(valueResponse < 0) {
		log_info(g_logger,"Error al recibir los datos del handshake");
		estado = false;
	} else {
		resultado res;
		res.accionEjecutar = acc;
		int status = recibirYDeserializarRespuesta(serverSocket, &res);
		if(status<0) {
			log_info(g_logger,"Error");
			estado = false;
		} else {
			log_info(g_logger,"Recibi la respuesta del HANDSHAKE");
			log_info(g_logger,"El tamaño del value es: %i", ((resultadoHandshake*)(res.contenido))->tamanioValue);
			tamValue=((resultadoHandshake*)(res.contenido))->tamanioValue;
			estado = true;
		}
	}
	free(buffer);
	return estado;
}


void actualizarTablaGlobal(int nPagina){
	bool mismoNumero(void* elem){
		return ((NodoTablaPaginas* )elem)->pagina->numero_pagina == nPagina;
	}

	pthread_mutex_lock(&mTabPagGlobal);
	NodoTablaPaginas* nodo = list_remove_by_condition(tabla_paginas_global,mismoNumero);
	pthread_mutex_unlock(&mTabPagGlobal);

	list_add(tabla_paginas_global,nodo);
}

Registro* pedirAlLFS(char* nombre_tabla, uint16_t key){

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

	sleep(retardoLFS/1000);
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
	res.accionEjecutar = SELECT;

	Registro* registro;

	if(contieneRegistro(nombre_tabla,key,&pagina)){

		int posicion=(pagina->indice_registro)*offset;

		sleep(retardoMemoria/1000);

		registro = malloc(sizeof(Registro));

		pthread_mutex_lock(&mMemPrincipal);
		registro->value = strdup(&memoria[posicion]);
		memcpy(&registro->key,(&memoria[posicion+tamValue]),sizeof(uint16_t));
		memcpy(&registro->timestamp,(&memoria[posicion+tamValue+sizeof(uint16_t)]),sizeof(long));
		pthread_mutex_unlock(&mMemPrincipal);

		res.mensaje= strdup(registro->value);
		res.resultado=OK;

		actualizarTablaGlobal(pagina->numero_pagina);
	}
	else{
		log_info(g_logger,"No encontre el registro, voy a hablar con el LFS");	//Tengo que pedirselo al LFS y agregarlo en la pagina

		registro = pedirAlLFS(nombre_tabla,key);	//mejor pasar un Segmento

		if(registro==NULL){
			char* aux = "Fallo al recibir registro";
			res.resultado=ERROR;
			res.mensaje= string_duplicate(aux);
			return res;
		}

		pthread_mutex_lock(&mBitmap);
		int posLibre= espacioLibre();
		pthread_mutex_unlock(&mBitmap);

		if(posLibre>=0){
			almacenarRegistro(nombre_tabla,*registro,posLibre);
			res.resultado=OK;
		}
		else
			res = iniciarReemplazo(nombre_tabla,*registro,0);

		if(res.resultado==OK)
			res.mensaje= string_duplicate((registro)->value);


	}
	res.contenido = registro;

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

	pthread_mutex_lock(&mTabSeg);
	segmento->numero_segmento=tabla_segmentos->elements_count;
	pthread_mutex_unlock(&mTabSeg);

	segmento->puntero_tpaginas=list_create();

//	segmento->nombre_tabla=nombre_tabla;
	strcpy(segmento->nombre_tabla,nombre_tabla);

	pthread_mutex_lock(&mTabSeg);
	list_add(tabla_segmentos,segmento);
	pthread_mutex_unlock(&mTabSeg);

	return segmento;
}


void agregarPagina(Registro registro, Segmento *segmento, int posLibre, int valorFlag){
	Pagina* pagina=malloc(sizeof(Pagina));
	guardarEnMemoria(registro, posLibre);

	pagina->indice_registro=posLibre;

	pthread_mutex_lock(&mTabPagGlobal);
	pagina->numero_pagina=tabla_paginas_global->elements_count;
	pthread_mutex_unlock(&mTabPagGlobal);

	pagina->flag_modificado=valorFlag;

	list_add(segmento->puntero_tpaginas, pagina);

	NodoTablaPaginas* nodo=malloc(sizeof(NodoTablaPaginas));
	nodo->pagina=pagina;
	nodo->segmento=segmento;

	pthread_mutex_lock(&mTabPagGlobal);
	list_add(tabla_paginas_global,nodo);
	pthread_mutex_unlock(&mTabPagGlobal);

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

		pthread_mutex_lock(&mTabPagGlobal);
		list_add(tabla_paginas_global,nodoPagina);
		pthread_mutex_unlock(&mTabPagGlobal);

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
		if(estaHaciendoJournal){
			res.resultado=EnJOURNAL;
			res.mensaje = NULL;
		}
		else
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
		else if(res.resultado == EnJOURNAL)
		{
			log_info(g_logger,"Se esta haciendo Journaling, ingrese la request mas tarde");
		}
		else if(res.resultado == SALIR)
		{
			ejecutando = false;
			log_info(g_logger,"Voy a salir");
		}
		if(res.mensaje!=NULL)
			free(res.mensaje);
		free(mensaje);
	}
	ejecutando = false;
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
	log_info(g_logger,"Journaling, por favor espere");
	estaHaciendoJournal=true;
	list_iterate(tabla_paginas_global,enviarInsert);

	pthread_mutex_lock(&mTabSeg);
	list_clean_and_destroy_elements(tabla_segmentos,liberarSegmento);
	pthread_mutex_unlock(&mTabSeg);

	estaHaciendoJournal=false;
	log_info(g_logger,"Termino el journal, puede ingresar sus request");
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

		pthread_mutex_lock(&mMemPrincipal);
		memcpy(&cont->key,&(memoria[(indice*offset)+tamValue]),sizeof(uint16_t));
		cont->value = strdup(&memoria[indice*offset]);
		memcpy(&cont->timestamp,&(memoria[(indice*offset)+tamValue+sizeof(uint16_t)]),sizeof(cont->timestamp));
		pthread_mutex_unlock(&mMemPrincipal);

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

	pthread_mutex_lock(&mMemPrincipal);
	memcpy(&memoria[(posLibre*offset)],registro.value,tamValue);
	memcpy(&memoria[(posLibre*offset)+tamValue],&(registro.key),sizeof(uint16_t));
	memcpy(&memoria[(posLibre*offset)+tamValue+sizeof(uint16_t)],&(registro.timestamp),sizeof(long));
	pthread_mutex_unlock(&mMemPrincipal);

	pthread_mutex_lock(&mBitmap);
	bitmap[posLibre]=1;
	pthread_mutex_unlock(&mBitmap);
}

int contieneRegistro(char *nombre_tabla,uint16_t key, Pagina** pagina){
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

	pthread_mutex_lock(&mTabSeg);
	if (list_is_empty(tabla_segmentos)){
		pthread_mutex_unlock(&mTabSeg);
		return false;
	}

	else{
		pthread_mutex_unlock(&mTabSeg);

		pthread_mutex_lock(&mTabSeg);
		s=list_find(tabla_segmentos,tieneTabla);
		pthread_mutex_unlock(&mTabSeg);

		if(s==NULL){
			return false;
		}
		else{
			memcpy(segmento,&s,sizeof(Segmento*));
			return true;

		}
	}

}

bool encuentraPagina(Segmento* segmento,uint16_t key, Pagina** pagina){

	bool tieneKey(void *elemento){

		int posicion=(((Pagina *)elemento)->indice_registro)*offset;
		int i=0;
		sleep(retardoMemoria/1000);

		pthread_mutex_lock(&mMemPrincipal);
		memcpy(&i,&(memoria[posicion+tamValue]),sizeof(uint16_t));
		pthread_mutex_unlock(&mMemPrincipal);

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

resultado insert(char *nombre_tabla,uint16_t key,char *value,long timestamp){
	Segmento* segmento;

	Pagina* pagina;

	Registro registro;
	registro.timestamp=timestamp;
	registro.key=key;
	registro.value=value;

	pthread_mutex_lock(&mBitmap);
	int posLibre=espacioLibre();
	pthread_mutex_unlock(&mBitmap);

	resultado res;

	if(encuentraSegmento(nombre_tabla,&segmento)){

		if(encuentraPagina(segmento,key,&pagina)){	//en vez de basura(char *) pasarle una pagina
			actualizarRegistro(pagina,value,timestamp);
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

	res.accionEjecutar=INSERT;
	res.contenido=NULL;

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

	pthread_mutex_lock(&mTabPagGlobal);
	list_remove_and_destroy_by_condition(tabla_paginas_global,mismaPagina,destroy_nodo_pagina_global); //remuevo de la global
	pthread_mutex_unlock(&mTabPagGlobal);

	pthread_mutex_lock(&mBitmap);
	bitmap[pagina->indice_registro]=0;
	pthread_mutex_unlock(&mBitmap);

	free(pagina);
}

void corregirIndicesTablaSegmentos(){
	pthread_mutex_lock(&mTabSeg);
	for(int i=0;i<tabla_segmentos->elements_count;i++){

		Segmento *aux = list_get(tabla_segmentos,i);
		aux->numero_segmento = i;

	}
	pthread_mutex_unlock(&mTabSeg);

}

void corregirIndicesPaginasGlobal(){
	for(int i=0;i<tabla_paginas_global->elements_count;i++){

		NodoTablaPaginas *aux = list_get(tabla_paginas_global,i);
		aux->pagina->numero_pagina = i;

	}
}

void drop(char* nombre_tabla){

	Segmento* segmento;

	if(encuentraSegmento(nombre_tabla,&segmento)){

		pthread_mutex_lock(&mTabSeg);
		list_remove_and_destroy_element(tabla_segmentos,segmento->numero_segmento,liberarSegmento);
		pthread_mutex_unlock(&mTabSeg);

		corregirIndicesTablaSegmentos();
		corregirIndicesPaginasGlobal();
		log_info(g_logger, "Se libero el segmento de la tabla %s en memoria",nombre_tabla);
	}
	else{
		log_info(g_logger, "No se encontro el segmento de la tabla %s en memoria",nombre_tabla);
	}
}


void actualizarRegistro(Pagina *pagina,char *value,long timestamp){

	int posicion=(pagina->indice_registro)*offset;
	sleep(retardoMemoria/1000);

	long ts;

	pthread_mutex_lock(&mMemPrincipal);
	memcpy(&ts,&(memoria[posicion+tamValue+sizeof(uint16_t)]),sizeof(long));
	pthread_mutex_unlock(&mMemPrincipal);

	if(timestamp>=ts){
		pthread_mutex_lock(&mMemPrincipal);
		memcpy(&(memoria[posicion]),value,tamValue);
		memcpy(&(memoria[posicion+tamValue+sizeof(uint16_t)]),&timestamp,sizeof(long));
		pthread_mutex_unlock(&mMemPrincipal);

		pagina->flag_modificado=1;
	}
}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Destruyo la tabla de segmentos
	pthread_mutex_lock(&mTabSeg);
	list_destroy_and_destroy_elements(tabla_segmentos, destroy_nodo_segmento);
	pthread_mutex_unlock(&mTabSeg);


	//Liberar memoria
	pthread_mutex_lock(&mMemPrincipal);
	free(memoria);
	pthread_mutex_unlock(&mMemPrincipal);

	//Liberar bitmap
	pthread_mutex_lock(&mBitmap);
	free(bitmap);
	pthread_mutex_unlock(&mBitmap);

	//cierro el servidor
	close(serverSocket);

	//Destruyo tabla de paginas global
	list_destroy_and_destroy_elements(tabla_paginas_global,destroy_nodo_pagina_global);

	//Destruyo la lista de memorias seeds
	list_destroy_and_destroy_elements(memoriasSeeds, destroy_nodo_memoria);

	//Destruyo la lista de memorias conocidas
	pthread_mutex_lock(&mMemoriasConocidas);
	list_destroy_and_destroy_elements(memoriasConocidas, destroy_nodo_memoria);
	pthread_mutex_unlock(&mMemoriasConocidas);

	destruirMutexs();
}

void destruirMutexs(){
	pthread_mutex_destroy(&mMemPrincipal);
	pthread_mutex_destroy(&mTabSeg);
	pthread_mutex_destroy(&mTabPagGlobal);
	pthread_mutex_destroy(&mMemoriasConocidas);
	pthread_mutex_destroy(&mBitmap);


}

bool gestionarConexionALFS()
{
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(config_get_string_value(g_config, "IP_LFS"), config_get_string_value(g_config, "PUERTO_LFS"), &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	int status = connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	if((serverSocket==-1) || (status ==-1))
		return false;
	return true;
}

void iniciar_tablas(){
	pthread_mutex_lock(&mTabSeg);
	tabla_segmentos = list_create();
	pthread_mutex_unlock(&mTabSeg);
	tabla_paginas_global = list_create();
}

void destroy_nodo_memoria(void* elem){
	Memoria* mem = (Memoria*) elem;
//	free(mem->ip);
//	free(mem->puerto);
	free(mem);
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
			res = mandarALFS(*resParser);
			log_info(g_logger,"Se envió al LFS");
			break;
		}
		case INSERT:
		{
			contenidoInsert* contenido = resParser->contenido;
			res = insert(contenido->nombreTabla,contenido->key,contenido->value,contenido->timestamp);
			break;

		}
		case JOURNAL:
		{
			journal();
			res.accionEjecutar = JOURNAL;
			res.resultado = OK;
			res.contenido = NULL;
			char *aux = "Se realizo JOURNAL";
			res.mensaje=strdup(aux);
			break;
		}
		case CREATE:
		{
			mandarALFS(*resParser);
			log_info(g_logger,"Se envió al LFS");
			res = recibir();
			break;
		}
		case DROP:
		{
			contenidoDrop* contDrop = resParser->contenido;
			drop(contDrop->nombreTabla);
			mandarALFS(*resParser);
			res = recibir();
			break;
		}
		case DUMP:
		{
			mandarALFS(*resParser);
			log_info(g_logger,"Se envió al LFS");
			res = recibir();
			break;
		}
		case GOSSIPING:
		{
			mandarTabla(*((int*)(resParser->contenido)));
			res.resultado = ENVIADO;
			res.mensaje = NULL;
			log_info(g_logger,"Se retorno tabla gossiping");
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
		default:
		{
			res.resultado = SALIR;
			res.mensaje = NULL;
			break;
		}
	}
	free(resParser->contenido);
	return res;

}

