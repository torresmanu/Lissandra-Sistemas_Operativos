/*
 * fileSystem.c
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#include "fileSystem.h"

int existeMetadata(char* nombreTabla){
	char* metadataPath = obtenerMetadataPath(nombreTabla);
	FILE* metadataFile = fopen(metadataPath,"r");
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
	char* puntoMontura = config_get_string_value(g_config,"PUNTO_MONTAJE");
	char* metadataPath= string_new();
	string_append(&metadataPath, puntoMontura);
	string_append(&metadataPath, "/tables/");
	return metadataPath;
}

metadataTabla obtenerMetadata(char* nombreTabla){
	metadataTabla metadata;
	char* metadataPath=obtenerMetadataPath(nombreTabla);
	t_config* config = config_create(metadataPath);
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
			return regAux;
		}
	}
	return reg;
}

registro* fs_select_partition(char* nombreTabla, int key, int partition){
	char* partitionPath= obtenerTablePath();
	string_append(&partitionPath, nombreTabla);
	string_append(&partitionPath, "/");
	string_append(&partitionPath, string_itoa(partition));
	string_append(&partitionPath, ".bin");
	FILE* partitionFile = fopen(partitionPath,"r");
	//Si el file es null significa que no encuentro la particion
	if(partitionFile == NULL){
		return NULL;
	}
	registro* reg = obtenerRegistroDeArchivo(partitionFile,key);
	fclose(partitionFile);
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
		return NULL;
	}
	while((tablesde=readdir(tabledir))!= NULL){
		if(!string_contains(tablesde->d_name,".bin") && !string_contains(tablesde->d_name,"metadata")){
			char* temporalPath = string_duplicate(tablesPath);
			string_append(&temporalPath, "/");
			string_append(&temporalPath, tablesde->d_name);
			FILE* temporalFile = fopen(temporalPath,"r");
			//Si el file es null significa que no encuentro el archivo
			if(temporalFile != NULL){
				regAux = obtenerRegistroDeArchivo(temporalFile,key);
			}
			fclose(temporalFile);
			if(reg == NULL && regAux != NULL){
				reg = malloc(sizeof(registro));
				memcpy(reg,regAux,sizeof(registro));
			}
			else if(regAux != NULL && regAux->timestamp > reg->timestamp){
				memcpy(reg,regAux,sizeof(registro));
			}
		}
	}
	return reg;
}

registro* obtenerRegistroDeArchivo(FILE* file, int key){
	char linea[1024];
	registro* reg = NULL;
	while(fgets(linea,1024,(FILE*)file)){
		registro* auxReg = malloc (sizeof(registro));
		parseRegistro(linea,auxReg,config_get_int_value(g_config,"TAMAÑO_VALUE"));
		//Primero pregunto si el registro tiene el mismo value que yo quiero tomar
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

int crear_tabla(char* tabla,char* t_cons,int cant_part,int tiempo_comp){
	//Obtengo el path de la tabla a crear
	char* tablesPath= obtenerTablePath();
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

	return 0;
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
		binaryFile = fopen(binaryPath,"w");
		if(binaryFile == NULL){
			perror("Error: ");
			return -1;
		}
		fclose(binaryFile);
	}
}

int dropTableFS(char * tabla){
	//Obtengo el path de la tabla a dripear
	char* tablesPath= obtenerTablePath();
	string_append(&tablesPath,tabla);
	//Borro los archivos que estan adento
	DIR* tablesdir = opendir(tablesPath);
	struct dirent* tablesde;
	while((tablesde=readdir(tablesdir))!= NULL){
		if(strcmp(tablesde->d_name,".")!= 0 && strcmp(tablesde->d_name,"..")!= 0){
			char* filePath = string_duplicate(tablesPath);
			string_append(&filePath,"/");
			string_append(&filePath,tablesde->d_name);
			int status = remove(filePath);
			if(status != 0){
				perror("Error: ");
				return status;
			}
		}
	}
	//Borro el directorio
	int status = remove(tablesPath);
	if(status != 0){
		perror("Error: ");
	}
	return status;
}
