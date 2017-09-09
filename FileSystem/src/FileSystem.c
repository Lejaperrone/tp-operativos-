/*
 ============================================================================
 Name        : FileSystem.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "Comandos.h"
#include <Configuracion.h>
#include "Sockets.c"

typedef struct{
	int index;
	char nombre[255];
	int padre;
} t_directory;


int sizeBloque = 1048576; // 1mb
int mostrarLoggerPorPantalla = 1;
t_directory tablaDeDirectorios[100];

int getIndexDirectorio(char* ruta){
	int index = 0, i = 0;
	char* slash = "/";
	char* finalPath = malloc(256);
	char* rutaInvertida = strdup(string_reverse(ruta));
	char* caracterActual = string_substring(rutaInvertida, index, 1);
	char* comparador = malloc(256);
	while(strcmp(caracterActual,slash)){
		memcpy(finalPath + index, caracterActual, 1);
		++index;
		caracterActual = string_substring(rutaInvertida, index, 1);
	}
	finalPath = string_reverse(finalPath);
	for(i = 0; i < 100; ++i){
		memcpy(comparador, tablaDeDirectorios[i].nombre, 256);
		printf("\n %s", comparador);
		printf("\n %s", tablaDeDirectorios[i].nombre);
		if (strcmp(tablaDeDirectorios[i].nombre,finalPath) == 0){
			free(rutaInvertida);
			free(finalPath);
			free(comparador);
			return tablaDeDirectorios[i].index;
		}
	}
	free(rutaInvertida);
	free(finalPath);
	free(comparador);
	return -1;
}

void almacenarArchivo(char* ruta, char* nombreArchivo, char tipo, char* datos){
	getIndexDirectorio(ruta);
}

int main(void) {

	int sizeComando = 256;
	int clienteYama = 0;
	int servidorFS = crearSocket();

	t_log* logger = log_create("logFileSystem", "FileSystem.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	//levantarServidorFS(servidorFS, clienteYama);

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando);
		bzero(comando, sizeComando);
		comando = readline(">");
		if (comando)
			add_history(comando);

		log_trace(logger, "El usuario ingreso: %s", comando);

		if (string_starts_with(comando, "format")) {
			log_trace(logger, "File system formateado");
		}
		else if (string_starts_with(comando, "rm -d")) {
			log_trace(logger, "Directorio eliminado");
		}
		else if (string_starts_with(comando, "rm -b")) {
			log_trace(logger, "Bloque eliminado");
		}
		else if (string_starts_with(comando, "rm")) {
			if (eliminarArchivo(comando, 3) != -1)
				log_trace(logger, "archivo eliminado");
			else
				log_trace(logger, "No se pudo eliminar el archivo");
		}
		else if (string_starts_with(comando, "rename")) {
			log_trace(logger, "Archivo renombrado");
		}
		else if (string_starts_with(comando, "mv")) {
			log_trace(logger, "Archivo movido");
		}
		else if (string_starts_with(comando, "cat")) {
			if (mostrarArchivo(comando, 4) == 1){
			log_trace(logger, "Archivo mostrado");
			}else{
				log_trace(logger, "No se pudo mostrar el archivo");
			}
		}
		else if (string_starts_with(comando, "mkdir")) {
			if (crearDirectorio(comando,6) == 1){

			log_trace(logger, "Directorio creado");// avisar si ya existe
			}else{
				log_trace(logger, "No se pudo crear directorio");
			}
		}
		else if (string_starts_with(comando, "cpfrom")) {
			log_trace(logger, "Archivo copiado a yamafs");
		}
		else if (string_starts_with(comando, "cpto")) {
			log_trace(logger, "Archivo copiado desde yamafs");
		}
		else if (string_starts_with(comando, "cpblock")) {
			log_trace(logger, "Bloque copiado en el nodo");
		}
		else if (string_starts_with(comando, "md5")) {
			log_trace(logger, "MD5 del archivo");
		}
		else if (string_starts_with(comando, "ls")) {
			listarArchivos(comando, 3);
			log_trace(logger, "Archivos listados");
		}
		else if (string_starts_with(comando, "info")) {
			log_trace(logger, "Mostrando informacion del archivo");
		}
		else {
			printf("Comando invalido\n");
			log_trace(logger, "Comando invalido");
		}
		free(comando);
	}
}

