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
	int* bloques;
}fs_file;

//Funciones del FS propio
fs_file* fs_fopen(char* ruta);
void fs_fread(fs_file* file,void* resultado,int size,int position);
int fs_fprintf(fs_file* file, void* obj, int size);
void fs_close(fs_file* fs);
void fs_delete(fs_file* fs);

//Funciones del bitmap
void crearBitmap();
void liberarBloque(int bloque);
int obtenerSiguienteBloque();


#endif /* FILESYSTEM_H_ */
