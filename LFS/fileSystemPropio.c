/*
 * fileSystemPropio.c
 *
 *  Created on: 11 jun. 2019
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileSystemPropio.h"

int inicializarFSPropio(){
	//Inicializo primero el bitmap
	char* metadataPath = string_duplicate(puntoMontaje);
	string_append(&metadataPath,"/metadata");
	char* fsBitmapPath = string_duplicate(metadataPath);
	string_append(&fsBitmapPath,"/bitmap.bin");

	FILE* f = fopen(fsBitmapPath,"r");
	//Si f no es null entonces el archivo ya existe por ende los bloques tambien
	if(f != NULL){
		fclose(f);
		return 0;
	}
	//Creo primero el directorio metadata
	int status = mkdir(metadataPath,0777);
	free(metadataPath);
	if(status != 0){
		free(fsBitmapPath);
		return status;
	}
	f = fopen(fsBitmapPath,"w");
	free(fsBitmapPath);
	if(f == NULL){
		fclose(f);
		return -1;
	}
	int blocks = getIntConfig("BLOCKS");
	int blocksChar = (blocks/8)+1;
	char* bitmapData = malloc(blocksChar*sizeof(char));
	for(int i= 0;i< blocksChar;i++){
		bitmapData[i] = 0;
	}
	fwrite(bitmapData,sizeof(char),blocksChar,f);

	free(bitmapData);
	fclose(f);

	//Creo el directorio bloque
	char* bloquesPath = string_duplicate(puntoMontaje);
	string_append(&bloquesPath,"/");
	string_append(&bloquesPath,magicNumber);
	status = mkdir(bloquesPath,0777);
	if(status != 0){
		free(bloquesPath);
		return status;
	}
	string_append(&bloquesPath,"/bloques");
	status = mkdir(bloquesPath,0777);
	if(status != 0){
		free(bloquesPath);
		return status;
	}
	for(int i=0;i<blocks;i++){
		char* bloquePath = string_duplicate(bloquesPath);
		string_append(&bloquePath,"/");
		string_append(&bloquePath, string_from_format("%i", i));
		string_append(&bloquePath,".bin");
		FILE* fBloque = fopen(bloquePath,"w");
		if(fBloque != NULL){
			fclose(fBloque);
		}
		free(bloquePath);
	}

	free(bloquesPath);
	return 0;
}

//Devuelve el siguiente bloque libre, -1 si esta lleno o -2 si hay un error
int obtenerSiguienteBloque(){
	char* fsBitmapPath = string_duplicate(puntoMontaje);
	string_append(&fsBitmapPath,"/metadata/bitmap.bin");
	FILE* f = fopen(fsBitmapPath,"r+");
	//Si f no es null entonces el archivo ya existe
	if(f == NULL){
		return -2;
	}
	//Obtengo la cantidad
	int blocks = getIntConfig("BLOCKS");
	int blocksChar = (blocks/8)+1;
	char* bitmapData = malloc(blocksChar*sizeof(char));
	//Leo los bitmaps
	fread(bitmapData, sizeof(char), blocksChar, f);
	t_bitarray* bitarray =	bitarray_create_with_mode(bitmapData, blocksChar, LSB_FIRST);
	for(int i=0;i<blocks;i++){
		//Si encuentro un bit false entonces actualizo el archivo y devuelvo el valor
		if(bitarray_test_bit(bitarray,i) == false){
			bitarray_set_bit(bitarray,i);
			rewind(f);
			fwrite(bitarray->bitarray,sizeof(char),blocksChar,f);
			fclose(f);
			free(bitmapData);
			bitarray_destroy(bitarray);
			return i;
		}
	}
	fclose(f);
	free(bitmapData);
	bitarray_destroy(bitarray);
	return -1;
}

void liberarBloque(int bloque){
	//Obtengo la cantidad
	int blocks = getIntConfig("BLOCKS");
	int blocksChar = (blocks/8)+1;
	if(bloque >= blocks){
		return;
	}
	char* fsBitmapPath = string_duplicate(puntoMontaje);
	string_append(&fsBitmapPath,"/metadata/bitmap.bin");
	FILE* f = fopen(fsBitmapPath,"r+");
	//Si f no es null entonces el archivo ya existe
	if(f == NULL){
		log_info(g_logger,"Error al querer obtenr el archivo bitmap");
		return;
	}
	//Creo el array de bitmap
	char* bitmapData = malloc(blocksChar*sizeof(char));
	//Leo los bitmaps
	fread(bitmapData, sizeof(char), blocksChar, f);
	t_bitarray* bitarray =	bitarray_create_with_mode(bitmapData, blocksChar, LSB_FIRST);
	//Seteo el bitmap en 0 para la posicion requerida
	bitarray_clean_bit(bitarray,bloque);
	//Vuelvo a guardar
	rewind(f);
	fwrite(bitarray->bitarray,sizeof(char),blocksChar,f);
	fclose(f);
	free(bitmapData);
	bitarray_destroy(bitarray);
}

fs_file* fs_fopen(char* ruta){
	sem_wait(&semaforo);
	t_config* f = config_create(ruta);
	if(f == NULL){
		sem_post(&semaforo);
		return NULL;
	}
	fs_file * fs_f= malloc(sizeof(fs_file));
	strcpy(fs_f->name,ruta);
	fs_f->size = config_get_int_value(f,"SIZE");
	fs_f->bloques = config_get_array_value(f,"BLOCKS");
	config_destroy(f);
	sem_post(&semaforo);
	return fs_f;
}

int fs_fcreate(char* ruta){
	sem_wait(&semaforo);
	//Asigno un bloque
	int bloque = obtenerSiguienteBloque();
	if(bloque == -1){
		log_info(g_logger,"No quedan bloques libres");
		sem_post(&semaforo);
		return -1;
	}
	//Creo el archivo
	FILE* f = fopen(ruta,"w");
	if(f == NULL){
		sem_post(&semaforo);
		return -1;
	}
	//Escribo en el archivo
	fprintf(f,"SIZE=0\n");
	fprintf(f,"BLOCKS=[%i]\n",bloque);
	fprintf(f,"NAME=%s\n",ruta);
	fclose(f);
	sem_post(&semaforo);
	return 0;
}

void fs_fdelete(fs_file* fs){
	sem_wait(&semaforo);
	char* puntoMontura = string_duplicate(puntoMontaje);
	//Libero los bloques y limpio el archivo de los bloques
	int i=0;
	while(fs->bloques[i] != 0){
		int bloque = atoi(fs->bloques[i]);
		//Libero el bloque
		liberarBloque(bloque);
		//Limpio el archivo del bloque
		char* bloquePath = string_duplicate(puntoMontura);
		string_append(&bloquePath,"/");
		string_append(&bloquePath,magicNumber);
		string_append(&bloquePath,"/bloques/");
		string_append(&bloquePath,fs->bloques[i]);
		string_append(&bloquePath,".bin");
		FILE* fBloque = fopen(bloquePath,"w");
		if(fBloque != NULL){
			fclose(fBloque);
		}
		i++;
	}
	//Borro el archivo
	remove(fs->name);
	sem_post(&semaforo);
}

void fs_fclose(fs_file* f){
	free(f);
}

void fs_fread(fs_file* fs,registro* resultado,int position){
	sem_wait(&semaforo);
	int size = sizeof(registro)+tamValue;
	//Si el registro que quiero que leer esta por fuera del size retorno null en resultado
	if(fs->size < (position+size)){
		resultado = NULL;
		log_error(g_logger,"[fs_fread]Devuelvo NULL");
		sem_post(&semaforo);
		return;
	}
	//Calculo en que bloque esta el registro
	int tamBloque = tamanioBloque;
	int cantPorBloque = tamBloque/size;
	int posBloque = position/cantPorBloque;
	int relPosition = position - posBloque * cantPorBloque;
	char * bloque;
	//Obtengo el bloque calculado anteriormente
	for(int i= 0;i<=posBloque;i++){
		bloque = string_duplicate(fs->bloques[i]);
	}
	//Abro el archivo bloque
	char* bloquePath = string_duplicate(puntoMontaje);
	string_append(&bloquePath,"/");
	string_append(&bloquePath,magicNumber);
	string_append(&bloquePath,"/bloques/");
	string_append(&bloquePath,bloque);
	string_append(&bloquePath,".bin");
	FILE* fBloque = fopen(bloquePath,"r");
	if(fBloque == NULL){
		resultado = NULL;
		log_error(g_logger,"[fs_fread]Devuelvo NULL");
		sem_post(&semaforo);
		return;
	}
	fseek(fBloque, relPosition*size, SEEK_SET );
	fread(resultado,sizeof(registro),1,fBloque);
	resultado->value=malloc(tamValue);
	fread(resultado->value,tamValue,1,fBloque);
	fclose(fBloque);
	sem_post(&semaforo);
}

int fs_fprint(fs_file* fs, registro* obj){
	sem_wait(&semaforo);
	int size = sizeof(registro) + tamValue;
	//Veo cuantos bloques hay y me quedo con el ultimo
	int cantBloques=0;
	int ultimoBloque=0;
	while(fs->bloques[cantBloques] != 0){
		ultimoBloque = atoi(fs->bloques[cantBloques]);
		cantBloques++;
	}
	//Me fijo si el ultimo bloque esta ocupado, si no lo esta escribo sobre el ultimo y si lo esta pido otro
	int tamBloque = tamanioBloque;
	int cantObjPorBloque = tamBloque / size;
	int tamanoDisponible = cantObjPorBloque * cantBloques * size;
	if(tamanoDisponible < (fs->size + size)){
		//Pido un nuevo bloque
		ultimoBloque = obtenerSiguienteBloque();
		//Si el ultimo bloque es menor a cero entonces hay algun error
		if(ultimoBloque < 0){
			sem_post(&semaforo);
			return ultimoBloque;
		}
		//Actualizo el archivo con el nuevo bloque
		t_config* f = config_create(fs->name);
		if(f == NULL){
			sem_post(&semaforo);
			return -2;
		}
		char** bloques = config_get_array_value(f,"BLOCKS");
		char* strBloques = string_duplicate("[");
		int i = 0;
		while(bloques[i] != 0){
			string_append(&strBloques,bloques[i]);
			string_append(&strBloques,",");
			i++;
		}
		char strUltimoBloque[20];
		sprintf(strUltimoBloque, "%i", ultimoBloque);
		string_append(&strBloques,strUltimoBloque);
		string_append(&strBloques,"]");
		config_set_value(f,"BLOCKS",strBloques);
		fs->bloques = config_get_array_value(f,"BLOCKS");
		config_save(f);
		config_destroy(f);
	}
	//Abro el ultimo bloque
	char* bloquePath = string_duplicate(puntoMontaje);
	string_append(&bloquePath,"/");
	string_append(&bloquePath,magicNumber);
	string_append(&bloquePath,"/bloques/");
	char strUltimoBloque[20];
	sprintf(strUltimoBloque, "%i", ultimoBloque);
	string_append(&bloquePath,strUltimoBloque);
	string_append(&bloquePath,".bin");
	FILE* fBloque = fopen(bloquePath,"a");
	if(fBloque == NULL){
		sem_post(&semaforo);
		return -2;
	}
	char * strValue = string_duplicate(obj->value);
	obj->value = NULL;
	fwrite(obj,sizeof(registro),1,fBloque);
	//printf("Valor insertado: %s\n",obj->value);
	fwrite(strValue,tamValue,1,fBloque);
	fclose(fBloque);
	//Actualizo el size
	fs->size = fs->size + size;
	t_config* f = config_create(fs->name);
	if(f == NULL){
		sem_post(&semaforo);
		return -2;
	}
	char strSize[20];
	sprintf(strSize, "%i", fs->size);
	config_set_value(f,"SIZE",strSize);
	config_save(f);
	config_destroy(f);
	sem_post(&semaforo);
	return 0;
}
