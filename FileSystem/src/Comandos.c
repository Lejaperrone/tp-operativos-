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

bool validarDirectorio(char* path){
	DIR* dir = opendir(path);
	if (dir)
	{
	    printf("Exite el directorio %s en el FileSystem\n", path);
	    return true;
	}
	else if (ENOENT == errno)
	{
	    printf("No existe el archivo %s en el FileSystem\n", path);
	    return false;
	}
	else
	{
	    return false;
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

int eliminarDirectorio(char* comando, int longitudKey){
	int success = -1;
		char* path = malloc(strlen(comando)-longitudKey);
		memcpy(path,comando + longitudKey, strlen(comando) - longitudKey);
		if (validarDirectorio(path)){
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
	directorio = opendir(path);

	while ((elemento = readdir(directorio)) != NULL){
		if ((elemento->d_name) != (".") && (elemento->d_name) != ("..")){
			printf("%s\n", elemento->d_name);
		}
	}
	free(path);
	closedir(directorio);
}

int crearDirectorio(char* comando, int longitudKey){

	char* path = malloc(strlen(comando)-longitudKey);
	memcpy(path,comando + longitudKey, strlen(comando) - longitudKey);

	if (validarDirectorio(path)){
		return 2;
	}

	struct stat st = {0};

	if (stat(path, &st) == -1) {
	    mkdir(path, 0700);
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

	fd = fopen(path, "r");

	if(fd == NULL){
		return printf("Error al tratar de leer el archivo");
	}

	while ((c=fgetc(fd)) != EOF){
		putchar(c);
	}
	free(path);
	fclose(fd);
	return 1;
}

int cambiarNombre(char* comando, int longitudKey){

	char*  paths= malloc(strlen(comando)-longitudKey);
	memcpy(paths,comando + longitudKey, strlen(comando) - longitudKey);

	char* rutaNombreViejo = strtok(paths, " ");
	char* nombreNuevo = strtok(NULL, " ");

	char* rutaNombreViejoReverse = strdup(string_reverse(rutaNombreViejo));

	int posicion = 0;
	int longitudNombreOriginal = 0;

	char* caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);

	while(caracterActual != "/"){

		++longitudNombreOriginal;
		++posicion;
		caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	}
	rutaNombreViejoReverse = string_substring_from(rutaNombreViejoReverse, longitudNombreOriginal + 1 );
	rutaNombreViejoReverse = strdup(string_reverse(rutaNombreViejoReverse));

	strcat(rutaNombreViejoReverse, nombreNuevo);
	char* rutaNombreNuevo = rutaNombreViejoReverse;

	if (rename(rutaNombreViejo,rutaNombreNuevo) == 0){
		return 1;
	}else{
		return 0;
	}

	return 0;
}
