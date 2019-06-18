/*
 ============================================================================
 Name        : LFS.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "LFS.h"


int main(void) {
	resultado res;
	char* mensaje;
	res.resultado = OK;
	iniciar_programa();

	while(res.resultado != SALIR)
	{
		mensaje = readline(">");
		resultadoParser resParser = parseConsole(mensaje);
		res = parsear_mensaje(&resParser);
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

void iniciar_programa()
{
	pthread_attr_t attr;
	pthread_t thread;

	//Inicio el logger
	g_logger = log_create("LFS.log", "LFS", 1, LOG_LEVEL_INFO);
	log_info(g_logger,"Inicio Aplicacion LFS");

	//Inicio las configs
	g_config = config_create("LFS.config");
	log_info(g_logger,"Configuraciones inicializadas");

	//Inicio la memtable
	iniciar_memtable();

	server_fd = iniciarServidor(config_get_string_value(g_config,"PUERTO_SERVIDOR"));
	if(server_fd < 0) {
		printf("[iniciar_programa] Ocurrió un error al intentar iniciar el servidor\n");
	} else {
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		int err = pthread_create(&thread, &attr, esperarClienteNuevo, server_fd);
		if(err != 0) {
			printf("[iniciar_programa] Hubo un problema al crear el thread esperarClienteNuevo:[%s]\n", strerror(err));
		}
		pthread_attr_destroy(&attr);
	}
}

resultado parsear_mensaje(resultadoParser* resParser)
{
	resultado res;
	switch(resParser->accionEjecutar){
		case SELECT:
		{
			contenidoSelect* contSel;
			contSel = (contenidoSelect*)resParser->contenido;
			res = select_acc(contSel->nombreTabla,contSel->key);
			break;
		}
		case DESCRIBE:
		{
			contenidoDescribe* contDes = resParser->contenido;
			res = describe(contDes->nombreTabla);
			break;
		}
		case INSERT:
		{
			contenidoInsert* contIns = resParser->contenido;
			res = insert(contIns->nombreTabla,contIns->key,contIns->value,contIns->timestamp);
			break;
		}
		case JOURNAL:
		{
			journal();
			break;
		}
		case CREATE:
		{
			contenidoCreate* contCreate = resParser->contenido;
			res = create(contCreate->nombreTabla,contCreate->consistencia,contCreate->cant_part,contCreate->tiempo_compresion);
			break;
		}
		case DROP:
		{
			contenidoDrop* contDrop = resParser->contenido;
			res = drop(contDrop->nombreTabla);
			break;
		}
		case DUMP:
		{
			res = dump();
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
			break;
		}
		case HANDSHAKE:
		{
			res = handshake();
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

resultado select_acc(char* tabla,int key)
{
	resultado res;

	//Paso 1: Verificar que la tabla exista en el file system y obtengo la metadata
	metadataTabla metadata;
	if(existeMetadata(tabla) == 0){
		metadata = obtenerMetadata(tabla);
	}else{
		res.mensaje="Prueba";
		res.resultado=ERROR;
		return res;
	}

	//Paso 2: Calcular cual es la partición que contiene dicho KEY.
	int particion = key % metadata.partitions;
	if(particion == 0){
		particion = metadata.partitions;
	}

	//Paso 3: Escanear la partición objetivo, todos los archivos temporales
	//y la memoria temporal de dicha tabla (si existe) buscando la key deseada.
	registro* regMemTable = memtable_select(tabla,key);

	registro* regFs = fs_select(tabla,key,particion);

	//Paso 4: Comparo la de mayor timestamp

	if(regMemTable == NULL && regFs == NULL){
		log_info(g_logger,"No se encontro el registro");
	}else if(regMemTable == NULL && regFs != NULL){
		log_info(g_logger,regFs->value);
	}else if(regMemTable != NULL && regFs == NULL){
		log_info(g_logger,regMemTable->value);
	}else{
		if(regMemTable->timestamp > regFs->timestamp){
			log_info(g_logger,regMemTable->value);
		}else{
			log_info(g_logger,regFs->value);
		}
	}
	res.mensaje="Prueba";
	res.resultado=OK;
	return res;
}

resultado insert(char* tabla,int key,char* value,long timestamp)
{
	//Creo un registro que es con el que voy a llamar a los proyectos
	registro reg;
	reg.key=key;
	reg.value = string_duplicate(value);
	reg.timestamp = timestamp;

	//Llamo al insert
	memtable_insert(tabla,reg);

	//Devuelvo el resultado
	resultado res;
	res.resultado=OK;
	res.accionEjecutar=INSERT;
	res.contenido=NULL;
	return res;
}

resultado create(char* tabla,char* t_cons,int cant_part,int tiempo_comp)
{
	resultado res;
	//Valido que exista la tabla
	if(existeMetadata(tabla)){
		//Creo la tabla con su directorio, metadata y archivos binarios
		int status = crear_tabla(tabla,t_cons,cant_part,tiempo_comp);
		if(status != 0){
			res.mensaje="Error al crear la tabla";
			res.resultado=ERROR;
		}
		res.mensaje="Tabla creada exitosamente";
		res.resultado=OK;
	}else{
		res.mensaje="Ya existe la tabla";
		res.resultado=OK;
	}

	return res;
}

resultado describe(char* tabla)
{
	if(tabla != NULL){
		if(existeMetadata(tabla) == 0){
			metadataTabla metadata = obtenerMetadata(tabla);
			log_info(g_logger,metadata.consistency);
		}else{
			log_info(g_logger,"NO LA ENCONTRE");
		}
	}else{
		t_list* listaMetadata = obtenerTodasMetadata();
		if(listaMetadata != NULL){
			for(int i=0;i<list_size(listaMetadata);i++){
				metadataTabla* metadata = (metadataTabla*) list_get(listaMetadata,i);
				log_info(g_logger,metadata->consistency);
			}
		}
	}
	resultado res;
	res.mensaje="Resultado prueba";
	res.resultado=OK;
	return res;
}

resultado drop(char* tabla)
{
	resultado res;
	if(existeMetadata(tabla) == 0){
		int status = dropTableFS(tabla);
		if(status == 0){
			log_info(g_logger,"Tabla dropeada exitosamente");
		}else{
			log_info(g_logger,"Error al dropear la tabla");
		}
	}else{
		log_info(g_logger,"No existe la tabla a dropear");
	}
	res.mensaje="Mensaje de prueba";
	res.resultado=OK;
	return res;
}

resultado journal()
{
	if(existeMetadata("colores") == 0){
		metadataTabla masd = obtenerMetadata("colores");
		log_info(g_logger,masd.consistency);
	}
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado dump(){
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado handshake() {
	resultado res;
	resultadoHandshake* rh = malloc(sizeof(resultadoHandshake));
	rh->tamanioValue = config_get_int_value(g_config, "TAMANIO_VALUE");

	res.accionEjecutar = HANDSHAKE;
	res.resultado = OK;
	res.contenido = rh;
	return res;
}

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Finalizar programa
	finalizar_memtable();

	close(server_fd);
}

void gestionarConexion(int conexion_cliente) {
	int recibiendo = 1;
	int status;
	resultadoParser rp;
	char buffer[100];
	int size_to_send;

	char* buffer2 = malloc(sizeof(int));

	while(recibiendo) {
		accion acc;
		int valueResponse = recv(conexion_cliente, buffer2, sizeof(int), 0);
		memcpy(&acc, buffer2, sizeof(int));

		if(valueResponse < 0) { //Comenzamos a recibir datos del cliente
			//Si recv() recibe 0 el cliente ha cerrado la conexion. Si es menor que 0 ha habido algún error.
			printf("Error al recibir los datos\n");
			recibiendo = 0;
		} else if(valueResponse == 0) {
			printf("El cliente se desconectó\n");
			recibiendo = 0;
		} else {
			rp.accionEjecutar = acc;
			status = recibirYDeserializarPaquete(conexion_cliente, &rp);
			if(status<0) {
				recibiendo = 0;
			} else {
				/*printf("Recibi el paquete\n");
				printf("[gestionarConexion] key recibida = %i\n", ((contenidoInsert*)(rp.contenido))->key);
				printf("[gestionarConexion] value recibido = %s\n", ((contenidoInsert*)(rp.contenido))->value);
				printf("[gestionarConexion] nombreTabla recibido = %s\n", ((contenidoInsert*)(rp.contenido))->nombreTabla);
				printf("[gestionarConexion] Timestamp recibido = %ld\n", ((contenidoInsert*)(rp.contenido))->timestamp);*/
				resultado res = parsear_mensaje(&rp);
				char* paqueteRespuesta = serializarRespuesta(&res, &size_to_send);
				send(conexion_cliente, paqueteRespuesta, size_to_send, 0);
			}
		}
	}

	printf("Cierro la conexion normalmente\n");
}

int esperarClienteNuevo(int conexion_servidor) {

	int conexion_cliente;
	struct sockaddr_in cliente;
	socklen_t longc; //Debemos declarar una variable que contendrá la longitud de la estructura

	while(1) {
		conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc);
		if(conexion_cliente<0) {
			printf("Error al aceptar trafico\n");
			return 1;
		} else {
			pthread_attr_t attr;
			pthread_t thread;

			longc = sizeof(cliente);
			printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			int err = pthread_create(&thread, &attr, gestionarConexion, conexion_cliente);
			if(err != 0) {
				printf("[esperarClienteNuevo] Hubo un problema al crear el thread gestionarConexion:[%s]\n", strerror(err));
			}
			pthread_attr_destroy(&attr);
		}
	}

	return 0;
}

int iniciarServidor(char* configPuerto) {

	int conexion_servidor, puerto;
	struct sockaddr_in servidor;

	puerto = atoi(configPuerto);
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
