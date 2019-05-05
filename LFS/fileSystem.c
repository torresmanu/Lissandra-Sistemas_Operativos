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
}
