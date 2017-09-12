/*
 * FuncionesFS.c
 *
 *  Created on: 11/9/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <Configuracion.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "FuncionesFS.h"

#define cantDataNodes 10

extern t_directory tablaDeDirectorios[100];
extern char* rutaArchivos;
extern int cantBloques;
extern t_bitarray* bitmap[cantDataNodes];

int getIndexDirectorio(char* ruta){
	int index = 0, i = 0, j = 0, indexFinal = 0;
	char** arrayPath = string_split(ruta, "/");
	char* arrayComparador[100];
	while(arrayPath[index] != NULL){ // separo por '/' la ruta en un array
		arrayComparador[index] = malloc(strlen(arrayPath[index]));
		memcpy(arrayComparador[index],arrayPath[index],strlen(arrayPath[index]));
		++index; // guardo cuantas partes tiene el array
	}
	indexFinal = index - 1;
	int indexDirectorios[index];

	for (i = 0; i < index; ++i)
		indexDirectorios[i] = -1; // inicializo todo en -1, si al final me queda alguno en algun lado del array, significa que la ruta que pase no existe

	indexDirectorios[0] = 0;

	for(i = 0; i < index; --index){
		for(j = 0; j < 100; ++j){ // busco en la tabla que posicion coincide con el fragmento de ruta del loop
			if (strcmp(tablaDeDirectorios[j].nombre,arrayComparador[index]) == 0){ // me fijo si el padre de ese es el mismo que aparece en el fragmento anterior de la ruta
				if (index != 0 && strcmp(tablaDeDirectorios[tablaDeDirectorios[j].padre].nombre,arrayComparador[index-1]) == 0){ // si es asi lo guardo
					indexDirectorios[index] = tablaDeDirectorios[j].index;
				}
			}
		}
	}

	for (i = 0; i < indexFinal; ++i)
		if (indexDirectorios[i] == -1){
			for (i = 0; i < indexFinal; ++i){
				free(arrayComparador[i]);
			}
			return -1;
		} //me fijo si quedo algun -1, si es asi no existe la ruta

	for (i = 0; i < indexFinal; ++i){
		free(arrayComparador[i]);
	}
	return indexDirectorios[indexFinal]; // devuelvo el index de la ruta
}

char* buscarRutaArchivo(char* ruta){
	int indexDirectorio = getIndexDirectorio(ruta);
	char* numeroIndexString = string_itoa(indexDirectorio);
	char* rutaGenerada = malloc(strlen(rutaArchivos) + strlen(numeroIndexString));
	memcpy(rutaGenerada, rutaArchivos, strlen(rutaArchivos));
	memcpy(rutaGenerada + strlen(rutaArchivos), numeroIndexString, strlen(numeroIndexString));
	return rutaGenerada; //poner free despues de usar
}

void almacenarArchivo(char* ruta, char* nombreArchivo, char tipo, char* datos);

void inicializarBitmaps(){
	char* path = "/Metadata/Bitmap.bin"; //cambiar
	int fd = open(path,O_RDWR | O_TRUNC);
	if (fd == -1)
	{
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}
	if (lseek(fd, cantBloques-1, SEEK_SET) == -1)
		{
			close(fd);
			perror("Error calling lseek() to 'stretch' the file");
			exit(EXIT_FAILURE);
	}
	if (write(fd, "", 1) == -1)
	{
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}
	char* mapa = mmap(0,cantBloques/8,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);

	bitmap[0] = bitarray_create_with_mode(mapa, cantBloques/8 , LSB_FIRST);
}
