/*
 * fileSystem.h
 *
 *  Created on: 28 abr. 2019
 *      Author: utnso
 */

#ifndef FILESYSTEMPROPIO_H_
#define FILESYSTEMPROPIO_H_

#include <commons/bitarray.h>
#include "LFS.h"

typedef struct{
	int size;
	char** bloques;
	char name[255];
}fs_file;

//Funciones del FS propio
fs_file* fs_fopen(char*);
int fs_fcreate(char* ruta);
void fs_fread(fs_file* file,void* resultado,int size,int position);
int fs_fprint(fs_file* file, void* obj, int size);
void fs_fclose(fs_file* fs);
void fs_fdelete(fs_file* fs);

//Funciones del bitmap
void liberarBloque(int bloque);
int obtenerSiguienteBloque();

//Funciones inicializacion
int inicializarFSPropio();

#endif /* FILESYSTEM_H_ */
