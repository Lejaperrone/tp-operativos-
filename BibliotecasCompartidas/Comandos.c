/*
 * Comandos.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#include "Comandos.h"

bool validarArchivo(char* path) {
	if (access(path, R_OK) == -1) {
		printf("No existe el archivo %s en el FileSystem\n", path);
		return false;
	} else {
		printf("Existe el archivo %s en el FileSystem\n", path);
		return true;
	}
}

int eliminarArchivo(char* comando, int longitudKey){
	int success = -1;
	char* path = malloc(strlen(comando)-longitudKey);
	memcpy(path,comando + longitudKey, strlen(comando) - longitudKey);
	if (validarArchivo(path)){
		success = remove(path);
		if (success == -1)
			printf("No se pudo eliminar el archivo");
		else
			printf("El archivo fue eliminado correctamente");
	}
	free(path);
	return success;
}

void listarArchivos(char* comando, int longitudKey){

	char* path = malloc(strlen(comando)-longitudKey);
	memcpy(path,comando + longitudKey, strlen(comando) - longitudKey);

	DIR * directorio;
	struct dirent * elemento;
	char elem;
	directorio = opendir(path);

	while ((elemento = readdir(directorio)) != EOF){
		elem = elemento->d_name;
		if (elem != "." && elem != ("..")){
			printf(elem);
		}
	}
	free(path);
	closedir(directorio);
}

int crearDirectorio(char* comando, int longitudKey){

	char* path = malloc(strlen(comando)-longitudKey);
	memcpy(path,comando + longitudKey, strlen(comando) - longitudKey);

	if (mkdir(path) == 0){

		free(path);
		return 1;

	}else{

		free(path);
		return 0;
	}
}

int mostrarArchivo(char* comando, int longitudKey){

	char* path = malloc(strlen(comando)-longitudKey);
	memcpy(path,comando + longitudKey, strlen(comando) - longitudKey);

	FILE *fd;
	int c;

	fd = fopen(path, "rt");

	if(fd == NULL){
		return printf("Error al tratar de leer el archivo");
	}

	while ((c=fgetc(fd)) != EOF){
		if (c=='\n'){
			printf('\n');
		}else{
			putchar(c);
		}
	}
	free(path);
	fclose(fd);
	return 1;
}
