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
	int size = sizeof(uint16_t)+sizeof(uint64_t)+tamValue*sizeof(char);
	char* buffer = malloc(size);
	//Si el registro que quiero que leer esta por fuera del size retorno null en resultado
	if(fs->size < (position+size)){
		resultado = NULL;
		log_error(g_logger,"[fs_fread]Devuelvo NULL");
		sem_post(&semaforo);
		return;
	}
	int cantBloques=0;
	while(fs->bloques[cantBloques] != 0){
		cantBloques++;
	}
	//Calculo en que bloque esta el registro
	int posBloqueInicial = position*size/tamanioBloque;
	int posBloqueInicialRel = position*size-posBloqueInicial*tamanioBloque;
	char * bloque;
	//Obtengo el bloque calculado anteriormente
	for(int i= 0;i<=posBloqueInicial;i++){
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
	fseek(fBloque, posBloqueInicialRel, SEEK_SET );
	if(posBloqueInicialRel+size>tamanioBloque){
		fread(buffer,tamanioBloque-posBloqueInicialRel,1,fBloque);
	}else{
		fread(buffer,size,1,fBloque);
	}
	fclose(fBloque);
	//Chequeo si me quedo algo por leer
	if(posBloqueInicialRel+size>tamanioBloque){
		for(int i= 0;i<=posBloqueInicial+1;i++){
			bloque = string_duplicate(fs->bloques[i]);
		}
		//Abro el archivo bloque
		char* bloquePathFinal = string_duplicate(puntoMontaje);
		string_append(&bloquePathFinal,"/");
		string_append(&bloquePathFinal,magicNumber);
		string_append(&bloquePathFinal,"/bloques/");
		string_append(&bloquePathFinal,bloque);
		string_append(&bloquePathFinal,".bin");
		FILE* fBloqueFinal = fopen(bloquePathFinal,"r");
		if(fBloqueFinal == NULL){
			resultado = NULL;
			log_error(g_logger,"[fs_fread]Devuelvo NULL");
			sem_post(&semaforo);
			return;
		}
		fseek(fBloqueFinal, 0, SEEK_SET );
		if(size>tamanioBloque*2-posBloqueInicialRel){
			fread(buffer+tamanioBloque-posBloqueInicialRel,tamanioBloque,1,fBloqueFinal);
		}else{
			fread(buffer+tamanioBloque-posBloqueInicialRel,size-(tamanioBloque-posBloqueInicialRel),1,fBloqueFinal);
		}
		fclose(fBloqueFinal);
	}
	if(size>tamanioBloque*2-posBloqueInicialRel){
		for(int i= 0;i<=posBloqueInicial+2;i++){
			bloque = string_duplicate(fs->bloques[i]);
		}
		//Abro el archivo bloque
		char* bloquePathFinal2 = string_duplicate(puntoMontaje);
		string_append(&bloquePathFinal2,"/");
		string_append(&bloquePathFinal2,magicNumber);
		string_append(&bloquePathFinal2,"/bloques/");
		string_append(&bloquePathFinal2,bloque);
		string_append(&bloquePathFinal2,".bin");
		FILE* fBloqueFinal2 = fopen(bloquePathFinal2,"r");
		if(fBloqueFinal2 == NULL){
			resultado = NULL;
			log_error(g_logger,"[fs_fread]Devuelvo NULL");
			sem_post(&semaforo);
			return;
		}
		fseek(fBloqueFinal2, 0, SEEK_SET );
		fread(buffer+tamanioBloque*2-posBloqueInicialRel,size-(tamanioBloque*2-posBloqueInicialRel),1,fBloqueFinal2);
		fclose(fBloqueFinal2);
	}
	deserializarRegistro(buffer,resultado);
	sem_post(&semaforo);
}

int fs_fprint(fs_file* fs, registro* obj){
	sem_wait(&semaforo);
	int size = sizeof(uint16_t) + sizeof(uint64_t) + sizeof(char) * tamValue;
	//Veo cuantos bloques hay y me quedo con el ultimo
	int cantBloques=0;
	int ultimoBloque=0;
	while(fs->bloques[cantBloques] != 0){
		ultimoBloque = atoi(fs->bloques[cantBloques]);
		cantBloques++;
	}
	//Me fijo si queda espacio disponible en el priber bloque
	int tamanoDisponible = tamanioBloque*cantBloques - fs->size;
	int tamanoRestante = size-tamanoDisponible;
	char* buffer = serializarRegistro(obj);

	if(tamanoDisponible >= 0){

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
		//Escribo lo que tengo disponible
		if(size > tamanoDisponible){
			fwrite(buffer,tamanoDisponible,1,fBloque);
		}else{
			fwrite(buffer,size,1,fBloque);
		}
		fclose(fBloque);
	}
	if(tamanoRestante > 0){
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

		//Abro el ultimo bloque
		char* bloquePathFinal = string_duplicate(puntoMontaje);
		string_append(&bloquePathFinal,"/");
		string_append(&bloquePathFinal,magicNumber);
		string_append(&bloquePathFinal,"/bloques/");
		char strUltimoBloqueFinal[20];
		sprintf(strUltimoBloqueFinal, "%i", ultimoBloque);
		string_append(&bloquePathFinal,strUltimoBloque);
		string_append(&bloquePathFinal,".bin");
		FILE* fBloqueFinal = fopen(bloquePathFinal,"a");
		if(fBloqueFinal == NULL){
			sem_post(&semaforo);
			return -2;
		}
		//Escribo lo que tengo restante fijandome si entra en un bloque
		if(tamanoRestante > tamanioBloque){
			fwrite(buffer+tamanoDisponible,tamanioBloque,1,fBloqueFinal);
		}else{
			fwrite(buffer+tamanoDisponible,tamanoRestante,1,fBloqueFinal);
		}
		fclose(fBloqueFinal);
	}
	if(tamanoRestante>tamanioBloque){
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
		char** bloques2 = config_get_array_value(f,"BLOCKS");
		char* strBloques2 = string_duplicate("[");
		int i = 0;
		while(bloques2[i] != 0){
			string_append(&strBloques2,bloques2[i]);
			string_append(&strBloques2,",");
			i++;
		}
		char strUltimoBloque[20];
		sprintf(strUltimoBloque, "%i", ultimoBloque);
		string_append(&strBloques2,strUltimoBloque);
		string_append(&strBloques2,"]");
		config_set_value(f,"BLOCKS",strBloques2);
		fs->bloques = config_get_array_value(f,"BLOCKS");
		config_save(f);
		config_destroy(f);

		//Abro el ultimo bloque
		char* bloquePathFinal2 = string_duplicate(puntoMontaje);
		string_append(&bloquePathFinal2,"/");
		string_append(&bloquePathFinal2,magicNumber);
		string_append(&bloquePathFinal2,"/bloques/");
		char strUltimoBloqueFinal2[20];
		sprintf(strUltimoBloqueFinal2, "%i", ultimoBloque);
		string_append(&bloquePathFinal2,strUltimoBloque);
		string_append(&bloquePathFinal2,".bin");
		FILE* fBloqueFinal2 = fopen(bloquePathFinal2,"a");
		if(fBloqueFinal2 == NULL){
			sem_post(&semaforo);
			return -2;
		}
		//Escribo lo que tengo restante fijandome si entra en un bloque
		fwrite(buffer+tamanoDisponible+tamanioBloque,tamanoRestante-tamanioBloque,1,fBloqueFinal2);
		fclose(fBloqueFinal2);
	}



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

char* serializarRegistro(registro* reg){
	int totalSize = tamValue * sizeof(char) + sizeof(uint16_t) + sizeof (uint64_t);
	char* paqueteSerializado = (char*) malloc(totalSize);
	int size_to_send = 0;
	int offset = 0;
	size_to_send = sizeof(uint16_t);
	memcpy(paqueteSerializado + offset, &(reg->key), size_to_send);
	offset += size_to_send;

	size_to_send = tamValue * sizeof(char);
	memcpy(paqueteSerializado + offset, reg->value, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint64_t);
	memcpy(paqueteSerializado + offset, &(reg->timestamp), size_to_send);
	return paqueteSerializado;
}

void deserializarRegistro(char* strReg,registro* reg){
	int valueSize;
	int nombreTablaSize;
	int status;
	int offset;
	int totalSize = tamValue * sizeof(char) + sizeof(uint16_t) + sizeof (uint64_t);

	memcpy(&(reg->key), strReg, sizeof(uint16_t));
	offset = sizeof(uint16_t);

	reg->value = malloc(tamValue*sizeof(char));
	memcpy(reg->value, strReg+offset, tamValue * sizeof(char));
	offset += tamValue * sizeof(char);

	memcpy(&(reg->timestamp), strReg+offset, sizeof(uint64_t));


}
