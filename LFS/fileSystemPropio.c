/*
 * fileSystemPropio.c
 *
 *  Created on: 11 jun. 2019
 *      Author: utnso
 */

#include "fileSystemPropio.h"


void crearBitmap(){
	char* fsBitmapPath = getStringConfig("PUNTO_MONTAJE");
	string_append(&fsBitmapPath,"/metadata/bitmap.bin");

	FILE* f = fopen(fsBitmapPath,"r");
	//Si f no es null entonces el archivo ya existe
	if(f != NULL){
		fclose(f);
		return;
	}
	f = fopen(fsBitmapPath,"w");
	int blocks = getIntConfig("BLOCKS");
	int blocksChar = (blocks/8)+1;
	char* bitmapData = malloc(blocksChar*sizeof(char));
	for(int i= 0;i< blocksChar;i++){
		bitmapData[i] = 0;
	}
	fwrite(bitmapData,sizeof(char),blocksChar,f);
	fclose(f);
}

//Devuelve el siguiente bloque libre, -1 si esta lleno o -2 si hay un error
int obtenerSiguienteBloque(){
	char* fsBitmapPath = getStringConfig("PUNTO_MONTAJE");
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
	char* fsBitmapPath = getStringConfig("PUNTO_MONTAJE");
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
	fs_file * fs_f= malloc(sizeof(fs_file));
	t_config* f = config_create(ruta);
	if(f == NULL){
		return NULL;
	}
	fs_f->size = config_get_int_value(f,"SIZE");
	char ** strBlocks = config_get_array_value(f,"BLOCKS");
	int i=0;
	while(strBlocks[i] != '\0'){
		i++;
	}
	fs_f->bloques = malloc(sizeof(int)*i);
	i=0;
	while(strBlocks[i] != '\0'){
		fs_f->bloques[i] = atoi(strBlocks[i]);
		i++;
	}
	return fs_f;
}
