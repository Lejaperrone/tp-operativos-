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
#include "Comandos.h"

#define cantDataNodes 10

extern t_directory tablaDeDirectorios[100];
extern char* rutaArchivos;
extern int cantBloques;
extern t_bitarray* bitmap[cantDataNodes];
extern t_log* loggerFS;

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

void* consolaFS(){

	int sizeComando = 256;

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando);
		bzero(comando, sizeComando);
		comando = readline(">");
		if (comando)
			add_history(comando);

		log_trace(loggerFS, "El usuario ingreso: %s", comando);

		if (string_starts_with(comando, "format")) {
			log_trace(loggerFS, "File system formateado");
		}
		else if (string_starts_with(comando, "rm -d")) {
			if (eliminarDirectorio(comando, 6) != -1)
				log_trace(loggerFS, "Directorio eliminado");
			else
				log_trace(loggerFS, "No se pudo eliminar el directorio");
		}
		else if (string_starts_with(comando, "rm -b")) {
			log_trace(loggerFS, "Bloque eliminado");
		}
		else if (string_starts_with(comando, "rm")) {
			if (eliminarArchivo(comando, 3) != -1)
				log_trace(loggerFS, "archivo eliminado");
			else
				log_trace(loggerFS, "No se pudo eliminar el archivo");
		}
		else if (string_starts_with(comando, "rename")) {
			if (cambiarNombre(comando, 7) == 1)
				log_trace(loggerFS, "Renombrado");
			else
				log_trace(loggerFS, "No se pudo renombrar");

		}
		else if (string_starts_with(comando, "mv")) {
			if (mover(comando,3) == 1)
				log_trace(loggerFS, "Archivo movido");
			else
				log_trace(loggerFS, "No se pudo mover el archivo");
		}
		else if (string_starts_with(comando, "cat")) {
			if (mostrarArchivo(comando, 4) == 1){
			log_trace(loggerFS, "Archivo mostrado");
			}else{
				log_trace(loggerFS, "No se pudo mostrar el archivo");
			}
		}
		else if (string_starts_with(comando, "mkdir")) {
			if (crearDirectorio(comando,6) == 1){

			log_trace(loggerFS, "Directorio creado");// avisar si ya existe
			}else{
				if (crearDirectorio(comando,6) == 2){
				log_trace(loggerFS, "El directorio ya existe");
				}else{
					log_trace(loggerFS, "No se pudo crear directorio");
				}
			}
		}
		else if (string_starts_with(comando, "cpfrom")) {
			log_trace(loggerFS, "Archivo copiado a yamafs");
		}
		else if (string_starts_with(comando, "cpto")) {
			log_trace(loggerFS, "Archivo copiado desde yamafs");
		}
		else if (string_starts_with(comando, "cpblock")) {
			log_trace(loggerFS, "Bloque copiado en el nodo");
		}
		else if (string_starts_with(comando, "md5")) {
			log_trace(loggerFS, "MD5 del archivo");
		}
		else if (string_starts_with(comando, "ls")) {
			listarArchivos(comando, 3);
			log_trace(loggerFS, "Archivos listados");
		}
		else if (string_starts_with(comando, "info")) {
			if (informacion(comando,5) == 1)
				log_trace(loggerFS, "Mostrando informacion del archivo");
			else
				log_trace(loggerFS, "No se pudo mostrar informacion del archivo");
		}
		else {
			printf("Comando invalido\n");
			log_trace(loggerFS, "Comando invalido");
		}
		free(comando);
	}
	return 0;
}

void almacenarArchivo(char* ruta, char* nombreArchivo, char tipo, char* datos);

void inicializarBitmaps(){ //TODO (y ver si aca hace falta mmap)
	/*char* path = "/Metadata/Bitmap.bin"; //cambiar
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

	bitmap[0] = bitarray_create_with_mode(mapa, cantBloques/8 , LSB_FIRST);*/
}
