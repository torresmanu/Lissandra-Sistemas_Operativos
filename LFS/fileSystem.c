/*
 * fileSystem.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "fileSystem.h"


registro* obtenerRegistroDeArchivoPropio(fs_file* file, int key){

	//Divido el size del archivo por el peso del registro
	int cant = file->size/(sizeof(registro) + getIntConfig("TAMANIO_VALUE"));
	registro* auxReg = malloc (sizeof(registro));
	registro* reg = NULL;
	//Leo los archivos
	for(int i=0;i<cant;i++){
		fs_fread(file,auxReg,i);
		if(auxReg->key == key){
			//Si el registro todavia no se habia seteado lo seteo
			if(reg == NULL){
				reg = malloc(sizeof(registro));
				memcpy(reg,auxReg,sizeof(registro));
			}
			//Si ya tengo un registro comparo los timestamp
			else if(auxReg->timestamp > reg->timestamp){
				memcpy(reg,auxReg,sizeof(registro));
			}
		}
	}

	return reg;
}


int existeMetadata(char* nombreTabla){
	char* metadataPath = obtenerMetadataPath(nombreTabla);
	FILE* metadataFile = fopen(metadataPath,"r");
	free(metadataPath);
	//Si el file es null significa que no existe el archivo metadata por lo tanto no existe la tabla
	if(metadataFile == NULL){
		return -1;
	}
	fclose(metadataFile);
	return 0;
}

char* obtenerMetadataPath(char* nombreTabla){
	char* metadataPath= obtenerTablePath();
	string_append(&metadataPath, nombreTabla);
	string_append(&metadataPath, "/metadata");
	return metadataPath;
}

char* obtenerTablePath(){
	char* puntoMontura = getStringConfig("PUNTO_MONTAJE");
	char* metadataPath= string_new();
	string_append(&metadataPath, puntoMontura);
	string_append(&metadataPath, "/tables/");
	return metadataPath;
}

metadataTabla obtenerMetadata(char* nombreTabla){
	metadataTabla metadata;
	char* metadataPath=obtenerMetadataPath(nombreTabla);
	t_config* config = config_create(metadataPath);
	free(metadataPath);
	metadata.compaction_time = config_get_int_value(config,"COMPACTION_TIME");
	metadata.partitions = config_get_int_value(config,"PARTITIONS");
	metadata.consistency = string_duplicate(config_get_string_value(config,"CONSISTENCY"));
	config_destroy(config);
	return metadata;
}

t_list* obtenerTodasMetadata(){
	t_list* metadataList = list_create();
	DIR* tablesdir = opendir(obtenerTablePath());
	struct dirent* tablesde;
	if(tablesdir == NULL){
		log_info(g_logger,"Error al obtener el path a las tablas");
		return NULL;
	}
	while((tablesde=readdir(tablesdir))!= NULL){
		if(!string_contains(tablesde->d_name,".")){
			metadataTabla meta = obtenerMetadata(tablesde->d_name);
			metadataTabla* metaIns = malloc(sizeof(metadataTabla));
			memcpy(metaIns,&meta,sizeof(metadataTabla));
			list_add(metadataList,metaIns);
		}
	}
	closedir(tablesdir);
	return metadataList;
}

registro* fs_select(char* nombreTabla, int key, int partition){
	registro* regAux = NULL;
	registro* reg =fs_select_partition(nombreTabla,key,partition);
	regAux = fs_select_temporal(nombreTabla,key);
	if(reg == NULL && regAux != NULL){
		return regAux;
	}else if(regAux != NULL && reg != NULL){
		if(regAux->timestamp > reg->timestamp){
			free(reg->value);
			free(reg);
			return regAux;
		}
	}

	if(regAux != NULL) {
		free(regAux->value);
		free(regAux);
	}

	return reg;
}

registro* fs_select_partition(char* nombreTabla, int key, int partition){
	char* partitionPath= obtenerTablePath();
	string_append(&partitionPath, nombreTabla);
	string_append(&partitionPath, "/");
	string_append(&partitionPath, string_itoa(partition));
	string_append(&partitionPath, ".bin");
	//Abro el archivo para el FS Ubuntu
	//FILE* partitionFile = fopen(partitionPath,"r");
	//Abro el archivo para el FS propio
	fs_file* partitionFile = fs_fopen(partitionPath);
  free(partitionPath);
	//Si el file es null significa que no encuentro la particion
	if(partitionFile == NULL){
		return NULL;
	}
	//Funcion obtener reg del FS Ubuntu
	//registro* reg = obtenerRegistroDeArchivo(partitionFile,key);
	//Funcion obtener reg del FS propio
	registro* reg = obtenerRegistroDeArchivoPropio(partitionFile,key);

	//Cierro el archivo para el FS Ubuntu
	//fclose(partitionFile);
	//Cierro el archivo para el FS propio
	fs_fclose(partitionFile);
	return reg;
}

registro* fs_select_temporal(char* nombreTabla, int key){
	registro* reg = NULL;
	registro* regAux = NULL;
	char* tablesPath= obtenerTablePath();
	string_append(&tablesPath, nombreTabla);
	DIR* tabledir = opendir(tablesPath);
	struct dirent* tablesde;
	if(tabledir == NULL){
		free(tablesPath);
		return NULL;
	}
	while((tablesde=readdir(tabledir))!= NULL){
		if(strcmp(tablesde->d_name,"..") != 0 &&strcmp(tablesde->d_name,".") != 0 &&!string_contains(tablesde->d_name,".bin") && !string_contains(tablesde->d_name,"metadata")){
			char* temporalPath = string_duplicate(tablesPath);
			string_append(&temporalPath, "/");
			string_append(&temporalPath, tablesde->d_name);

			//Abro el archivo del FS Ubuntu
			//FILE* temporalFile = fopen(temporalPath,"r");
			//Abro el archivo del FS propio
			fs_file* temporalFile = fs_fopen(temporalPath);
      free(temporalPath);
			//Si el file es null significa que no encuentro el archivo
			if(temporalFile != NULL){
				//Obtengo el registro para el FS Ubuntu
				//regAux = obtenerRegistroDeArchivo(temporalFile,key);
				//Obtengo el registro para el FS propio
				regAux = obtenerRegistroDeArchivoPropio(temporalFile,key);
			}
			//Cierro el archivo para el FS Ubuntu
			//fclose(temporalFile);
			//Cierro el archivo para el FS propio
			fs_fclose(temporalFile);
			if(reg == NULL && regAux != NULL){
				reg = malloc(sizeof(registro));
				memcpy(reg,regAux,sizeof(registro));
			}
			else if(regAux != NULL && regAux->timestamp > reg->timestamp){
				memcpy(reg,regAux,sizeof(registro));
			}
		}
	}
	closedir(tabledir);
	return reg;
}

registro* obtenerRegistroDeArchivo(FILE* file, int key){
	char linea[1024];
	registro* reg = NULL;

	while(fgets(linea,1024,(FILE*)file)){
		registro* auxReg = malloc (sizeof(registro));
		parseRegistro(linea,auxReg,getIntConfig("TAMAÑO_VALUE"));
		//Primero pregunto si el registro tiene el mismo value que yo quiero tomar
		if(auxReg->key == key){
			//Si el registro todavia no se habia seteado lo seteo
			if(reg == NULL){
				//reg = malloc(sizeof(registro));
				//memcpy(reg,auxReg,sizeof(registro));
				reg = auxReg;
			}
			//Si ya tengo un registro comparo los timestamp
			else if(auxReg->timestamp > reg->timestamp){
				//memcpy(reg,auxReg,sizeof(registro));
				reg = auxReg;
			}
		} else {
			free(auxReg->value);
			free(auxReg);
		}
	}
	return reg;
}


int crear_tabla(char* tabla,char* t_cons,int cant_part,int tiempo_comp){
	//Obtengo el path de la tabla a crear
	char* tablesPath= obtenerTablePath();

	//Chequeo si el directorio tables esta creado, en caso contrario lo creo
	mkdir(tablesPath,0777);

	string_append(&tablesPath,tabla);

	//Creo el directorio
	int status = mkdir(tablesPath,0777);
	if(status != 0){
		return status;
	}

	//Creo la metadata
	metadataTabla metadata;
	metadata.compaction_time=tiempo_comp;
	metadata.consistency= string_duplicate(t_cons);
	metadata.partitions=cant_part;
	status = crearArchivoMetadata(tablesPath,metadata);
	if(status != 0){
		return status;
	}

	//Creo los archivos binarios necesarios
	status = crearArchivosBinarios(tablesPath,metadata);

	return status;
}

int crearArchivoMetadata(char* tablesPath,metadataTabla metadata){
	FILE* metadataFile;
	char* metadataPath= string_duplicate(tablesPath);
	string_append(&metadataPath,"/metadata");
	metadataFile = fopen(metadataPath,"w");
	if(metadataFile == NULL){
		return -1;
	}
	fprintf(metadataFile,"CONSISTENCY=%s\n",metadata.consistency);
	fprintf(metadataFile,"PARTITIONS=%d\n",metadata.partitions);
	fprintf(metadataFile,"COMPACTION_TIME=%d\n",metadata.compaction_time);
	fclose(metadataFile);
	return 0;
}

int crearArchivosBinarios(char* tablesPath,metadataTabla metadata){
	for(int i=0;i<metadata.partitions;i++){
		FILE* binaryFile;
		char* binaryPath= string_duplicate(tablesPath);
		string_append_with_format(&binaryPath,"/%d.bin",i+1);
		//Creo el archivo para FS ubuntuo
		/*binaryFile = fopen(binaryPath,"w");
		if(binaryFile == NULL){
			perror("Error: ");
			return -1;
		}
		fclose(binaryFile);*/
		//Creo el archivo para FS propio
		int status = fs_fcreate(binaryPath);
		if(status != 0){
			return status;
		}
	}
	return 0;
}

int dropTableFS(char * tabla){
	//Obtengo el path de la tabla a dripear
	char* tablesPath= obtenerTablePath();
	string_append(&tablesPath,tabla);
	//Borro primero el archivo metadata
	char* metadataPath = string_duplicate(tablesPath);
	string_append(&metadataPath,"/metadata");
	int st = remove(metadataPath);
	if(st != 0){
		perror("Error: ");
		return st;
	}
	//Borro los archivos binarios que estan adento
	DIR* tablesdir = opendir(tablesPath);
	struct dirent* tablesde;
	while((tablesde=readdir(tablesdir))!= NULL){
		if(strcmp(tablesde->d_name,".")!= 0 && strcmp(tablesde->d_name,"..")!= 0){
			char* filePath = string_duplicate(tablesPath);
			string_append(&filePath,"/");
			string_append(&filePath,tablesde->d_name);
			//Borro el archivo para el FS Ubuntu
			/*int status = remove(filePath);
			if(status != 0){
				perror("Error: ");
				return status;
			}*/
			//Borro el archivo para el FS propio
			fs_file* file = fs_fopen(filePath);
			if(file == NULL){
				return -1;
			}
			fs_fdelete(file);
		}
	}
	//Borro el directorio
	int status = remove(tablesPath);
	if(status != 0){
		perror("Error: ");
	}
	return status;
}

int fs_create_tmp(char* tabla,t_list* regList){
	//Chequeo que exista el directorio de la tabla, si existe lo creo
	char* tablesPath= obtenerTablePath();
	string_append(&tablesPath,tabla);
	mkdir(tablesPath,0777);

	//Busco archivos tmp hasta que no encuentre
	int i=1;
	int ultimoTemp = 0;
	char strUltimoTemp[200];
	while(ultimoTemp == 0){
		sprintf(strUltimoTemp, "%s/%i.tmp",tablesPath, i);
		FILE* f = fopen(strUltimoTemp,"r");
		if(f != NULL){
			fclose(f);
		}else{
			ultimoTemp = i;
		}
		i++;
	}

	//Creo el archivo para el FS Ubuntu
	/*FILE* file = fopen(strUltimoTemp,"w");
	if(file == NULL){
		return -1;
	}*/

	//Creo el archivo para el FS propio y lo abro
	int st = fs_fcreate(strUltimoTemp);
	if(st != 0){
		return st;
	}

	fs_file* file = fs_fopen(strUltimoTemp);
	if(file == NULL){
		return -1;
	}

	//Guardo todos los registros
	for(int n = 0; n < list_size(regList); n++){
		registro* reg = ((registro*)list_get(regList,n));
		//Escribo el archivo en el FS ubuntu
		/*char strReg[200];
		sprintf(strReg, "%ld;%i;%s\n",reg->timestamp,reg->key,reg->value);
		fprintf(file, "%s",strReg);*/
		//Escribo el archivo en el FS propio
		int status = fs_fprint(file,reg);
		if(status != 0){
			fs_fclose(file);
			return -1;
		}
	}

	//Cierro el archivo para el FS Ubuntu
	//fclose(file);
	//Cierro el archivo para el FS propio
	fs_fclose(file);
	return 0;
}

void compactarTabla(char* tabla){
	char* tablesPath= obtenerTablePath();
	string_append(&tablesPath,tabla);
	DIR* tabledir = opendir(tablesPath);
	struct dirent* tablesde;
	if(tabledir == NULL){
		log_info(g_logger,"Error al compactar tabla %s al abrir directorio",tabla);
		return;
	}
	int cantidadACompactar = 0;
	//Itero entre los archivos temporales para renombrarlos
	while((tablesde=readdir(tabledir))!= NULL){
		if(string_contains(tablesde->d_name,"tmp") && !string_contains(tablesde->d_name,"tmpc")){
			char * tmpPath = string_duplicate(tablesPath);
			string_append(&tmpPath,"/");
			string_append(&tmpPath,tablesde->d_name);
			char * tmpcPath = string_duplicate(tmpPath);
			string_append(&tmpcPath,"c");
			int status = rename(tmpPath, tmpcPath);
			if(status != 0){
				log_info(g_logger,"Error al cambiar el temporal a memoria");
			}else{
				cantidadACompactar ++;
			}
		}
	}

	closedir(tabledir);

	//Si son 0 a compactar entonces vuelvo porque no tiene sentido seguir
	if(cantidadACompactar ==  0){
		log_info(g_logger,"Tabla %s compactada exitosamente",tabla);
		return;
	}

	//Creo la lista
	t_list* list = list_create();

	tabledir = opendir(tablesPath);
	if(tabledir == NULL){
		log_info(g_logger,"Error al compactar tabla %s al abrir directorio",tabla);
		return;
	}

	//Cargo los archivos de particiones en la lista en memoria
	while((tablesde=readdir(tabledir))!= NULL){
		if(string_contains(tablesde->d_name,".bin")){
			char * binPath = string_duplicate(tablesPath);
			string_append(&binPath,"/");
			string_append(&binPath,tablesde->d_name);

			//Agrego el nodo de la particion a la lista
			nodo_tabla* nodo = malloc(sizeof(nodo_tabla));
			nodo->nombre_tabla = string_duplicate(binPath);
			nodo->lista_registros = list_create();
			list_add(list,nodo);

			//Abro el archivo para FS Ubuntu
			//FILE * file = fopen(binPath,"r");
			//Abro el archivo para FS propio
			fs_file* file = fs_fopen(binPath);

			//Itero entre los registros agregandolos a la lista FS Ubuntu
			/*char linea[1024];
			while(fgets(linea,1024,(FILE*)file)){
				registro * reg_aux = malloc(sizeof(registro));
				parseRegistro(linea,reg_aux,getIntConfig("TAMANIO_VALUE"));
				list_add(nodo->lista_registros,reg_aux);
			}*/
			//Itero entre los registros agregandolos a la lsita FS propio
			int cant = file->size/(sizeof(registro)+getIntConfig("TAMANIO_VALUE"));
			for(int n = 0; n < cant; n++){
				registro* auxReg = malloc (sizeof(registro));
				fs_fread(file,auxReg,n);
				list_add(nodo->lista_registros,auxReg);
			}

			//Cierro el archivo FS Ubuntu
			//fclose(file);
			//Cierro el archivo FS propio
			fs_fclose(file);
		}
	}

	closedir(tabledir);
	tabledir = opendir(tablesPath);
	if(tabledir == NULL){
		log_info(g_logger,"Error al compactar tabla %s al abrir directorio",tabla);
		return;
	}

	//Cargo los temporales en la lista en memoria
	while((tablesde=readdir(tabledir))!= NULL){
		if(string_contains(tablesde->d_name,".tmpc")){
			char * tempcPath = string_duplicate(tablesPath);
			string_append(&tempcPath,"/");
			string_append(&tempcPath,tablesde->d_name);

			//Abro el archivo FS Ubuntu
			//FILE * file = fopen(tempcPath,"r");
			//Abro el archivo FS propio
			fs_file* file = fs_fopen(tempcPath);
			registro* auxReg = malloc (sizeof(registro));
			int cant = file->size/(sizeof(registro)+getIntConfig("TAMANIO_VALUE"));

			//Itero entre los registros agregandolos a la lista
			char linea[1024];
			//while(fgets(linea,1024,(FILE*)file)){
			for(int i = 0; i < cant;i++){
				registro * reg = malloc(sizeof(registro));
				fs_fread(file,reg,i);

				//Busco la particion correspondiente donde guardarlo
				int particion = reg->key % list_size(list);
				if(particion == 0){
					particion = list_size(list);
				}

				//Busco el nodo correspondiente a la particion
				nodo_tabla* nodo;
				for(int i=0;i<list_size(list);i++){
					nodo_tabla* nodAux = (nodo_tabla*)list_get(list,i);
					char binName[20];
					sprintf(binName,"%i.bin",particion);
					if(string_contains(nodAux->nombre_tabla,binName)){
						nodo = nodAux;
					}
				}

				//Itero entre los registros, si lo encuentro lo actualizo si el timestamp es mayor
				int crear = 1;
				for(int i = 0; i < list_size(nodo->lista_registros); i++){
					registro* reg_aux = ((registro*)list_get(nodo->lista_registros,i));
					if(reg_aux->key == reg->key){
						if(reg_aux->timestamp < reg->timestamp){
							strcpy(reg_aux->value,reg->value);
							reg_aux->timestamp = reg->timestamp;
						}
						crear = 0;
					}
				}
				//Si no encontre el registro entonces lo creo
				if(crear == 1){
					list_add(nodo->lista_registros,reg);
				}
				else{
					free(reg);
				}
			}
			//Borro archivo FS propio
			fs_fclose(file);
			//Borro archivo FS Ubuntu
			//fclose(file);
		}
	}
	closedir(tabledir);

	//Bloqueo la tabla
	//TODO

	//Borro los archivos temporales y los binarios
	tabledir = opendir(tablesPath);
	if(tabledir == NULL){
		log_info(g_logger,"Error al compactar tabla %s al abrir directorio",tabla);
		return;
	}
	while((tablesde=readdir(tabledir))!= NULL){
		if(string_contains(tablesde->d_name,".bin") || string_contains(tablesde->d_name,".tmpc")){
			char * binPath = string_duplicate(tablesPath);
			string_append(&binPath,"/");
			string_append(&binPath,tablesde->d_name);

			//Abro el archivo para el FS propio
			fs_file* file = fs_fopen(binPath);

			//Borro el archivo para el FS propio
			fs_fdelete(file);

			//Cierro el archivo para el FS propio
			fs_fclose(file);
		}
	}

	closedir(tabledir);


	//Borro los archivos de particiones y los vuelvo a crear con los nuevos valores
	for(int i = 0; i < list_size(list);i ++){
		nodo_tabla* nodo = list_get(list,i);

		//Creo el archivo FS Ubuntu
		//FILE* file = fopen(nodo->nombre_tabla,"w");
		//Creo el archivo FS Propio

		int statusCreate = fs_fcreate(nodo->nombre_tabla);
		if(statusCreate != 0){
			log_info(g_logger,"Error al compactar tabla %s al crear archivo binario",tabla);
		}
		fs_file* file = fs_fopen(nodo->nombre_tabla);
		if(file == NULL){
			log_info(g_logger,"Error al compactar tabla %s al crear archivo binario",tabla);
		}

		//Guardo todos los registros
		for(int n = 0; n < list_size(nodo->lista_registros); n++){
			registro* reg = ((registro*)list_get(nodo->lista_registros,n));
			//Guardo el registro para el FS propio
			/*char strReg[200];
			sprintf(strReg, "%ld;%i;%s\n",reg->timestamp,reg->key,reg->value);
			fprintf(file, "%s",strReg);*/
			//Guardo el registro para el FS Ubuntu
			fs_fprint(file,reg);
		}
		//Cierro el archivo FS Ubuntu
		//fclose(file);
		//Cierro el archivo FS propio
		fs_fclose(file);

	}

	//Libero la lista
	//list_destroy_and_destroy_elements(list,destroy_nodo_tabla);

	//Libero el bloqueo
	//TODO
	log_info(g_logger,"Tabla %s compactada exitosamente",tabla);
}

void compactar(){
	char* tablesPath= obtenerTablePath();
	DIR* tabledir = opendir(tablesPath);
	struct dirent* tablesde;
	if(tabledir == NULL){
		return;
	}
	//Itero entre las tablas
	while((tablesde=readdir(tabledir))!= NULL){
		if(strcmp(tablesde->d_name,".")!= 0 && strcmp(tablesde->d_name,"..")!= 0){
			//Llamo a la funcion para cada tabla
			compactarTabla(tablesde->d_name);
		}
	}
	closedir(tabledir);
}
