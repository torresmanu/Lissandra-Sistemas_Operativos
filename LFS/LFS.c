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
	res.resultado= OK;
	iniciar_programa();

	//gestionarConexion();

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

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	int err = pthread_create(&thread, &attr, esperarClienteNuevo, server_fd);
	if(err != 0) {
		printf("Hubo un problema al crear el thread esperarCliente:[%s]\n", strerror(err));
		//return err;
	}

	pthread_attr_destroy(&attr);

}

resultado parsear_mensaje(char* mensaje)
{
	resultado res;
	resultadoParser resParser = parseConsole(mensaje);
	switch(resParser.accionEjecutar){
		case SELECT:
		{
			contenidoSelect* contSel;
			contSel = (contenidoSelect*)resParser.contenido;
			res = select_acc(contSel->nombreTabla,contSel->key);
			break;
		}
		case DESCRIBE:
		{
			contenidoDescribe* contDes = resParser.contenido;
			res = describe(contDes->nombreTabla);
			break;
		}
		case INSERT:
		{
			contenidoInsert* contIns = resParser.contenido;
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
			contenidoCreate* contCreate = resParser.contenido;
			res = create(contCreate->nombreTabla,contCreate->consistencia,contCreate->cant_part,contCreate->tiempo_compresion);
			break;
		}
		case DROP:
		{
			contenidoDrop* contDrop = resParser.contenido;
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
	//Hago un select a la memtable
	registro* reg = memtable_select(tabla,key);
	if(reg == NULL){
		res.mensaje="Registro no obtenido";
	}else{
		res.mensaje= string_duplicate(reg->value);
	}

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
	res.mensaje="Registro insertado exitosamente";
	res.resultado=OK;
	return res;
}

resultado create(char* tabla,char* t_cons,int cant_part,int tiempo_comp)
{
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado describe(char* tabla)
{
	if(tabla != NULL){
		obtenerMetadata(tabla);
	}else{
		obtenerTodasMetadata();
	}
	resultado res;
	res.mensaje="Salida prueba";
	res.resultado=OK;
	return res;
}

resultado drop(char* tabla)
{
	resultado res;
	res.mensaje="Salida prueba";
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

void terminar_programa()
{
	//Destruyo el logger
	log_destroy(g_logger);

	//Destruyo las configs
	config_destroy(g_config);

	//Finalizar programa
	finalizar_memtable();

	//close(*server_fd);
}

void gestionarConexion()
{
	int estado=1;
	char buffer[PACKAGESIZE];

	//int serverLiss_fd = iniciarServidor(config_get_string_value(g_config,"PUERTO_SERVIDOR"));
	//int clienteMem_fd = esperarCliente(serverLiss_fd,"Se conecto la memoria!");

	while(estado){

//		recibir_mensaje(clienteMem_fd,buffer,"La memoria me mando el mensaje");
		//recv(clienteMem_fd, (void*) buffer, PACKAGESIZE, 0);	//Recibo mensaje de la memorias

		if(strcmp(buffer,"exit")==0)
			estado = 0;
		else{
			printf( "\n%s: %s\n","La memoria me mando el mensaje", buffer);
			printf( "Tamaño: %d\n", strlen(buffer));
		}

	}
	/*
    close(clienteMem_fd);
    close(serverLiss_fd);
    */
}

int esperarClienteNuevo(int conexion_servidor) {

	int conexion_cliente, recibiendo = 1;
	struct sockaddr_in cliente;
	socklen_t longc; //Debemos declarar una variable que contendrá la longitud de la estructura
	char buffer[100]; //Declaramos una variable que contendrá los mensajes que recibamos

	longc = sizeof(cliente);
	conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc);

	if(conexion_cliente<0) {
		printf("Error al aceptar trafico\n");
		close(conexion_servidor);
		return 1;
	}

	printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));

	while(recibiendo) {
		if(recv(conexion_cliente, buffer, 100, 0) < 0) { //Comenzamos a recibir datos del cliente
			//Si recv() recibe 0 el cliente ha cerrado la conexion. Si es menor que 0 ha habido algún error.
			printf("Error al recibir los datos\n");
			close(conexion_servidor);
			recibiendo = 0;
			//return 1;
		} else {
			printf("%s\n", buffer);
			bzero((char *)&buffer, sizeof(buffer));
			send(conexion_cliente, "Recibido\n", 13, 0);
		}
	}

	printf("Cierro la conexion normalmente\n");
	close(conexion_servidor);
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
		printf("Error al asociar el puerto a la conexion\n");
	    close(conexion_servidor);
	    return 1;
	}

	listen(conexion_servidor, 3);
	printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));

	return conexion_servidor;
}

