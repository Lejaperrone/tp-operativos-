/*
 * Comandos.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#include "Comandos.h"

#define mb 1048576

char* devolverRuta(char* comando, int cantidadDeComandos)
{
	char* copiaComando = malloc(strlen(comando)+1);
	memcpy(copiaComando, comando,strlen(comando)+1);
	char* ruta = strtok(copiaComando, " ");
	int i;

	for (i = 0; i < cantidadDeComandos; ++i){
		ruta = strtok(NULL, " ");
	}
	return ruta;
}

int copiarArchivo(comando){
	printf("---%s\n",devolverRuta(comando, 1));
	printf("---%s\n",buscarRutaArchivo("hola/chau"));
	int mockSizeArchivo = 1024*1024*2;
	char* p = "hola/chau";
	char* n = "nom.bin";
	guardarEnNodos(p, n,mockSizeArchivo);
	//if (validarArchivo(pathFrom)){

	//}
	//else
		//return 0;
}


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

int eliminarArchivo(char* comando, int cantidadDeComandos){
	int success = -1;
	char* path = devolverRuta(comando, cantidadDeComandos);
	if (validarArchivo(path)){
		success = remove(path);
		if (success == -1)
			printf("No se pudo eliminar el archivo");
		else
			printf("El archivo fue eliminado correctamente");
	}
	return success;
}

int eliminarDirectorio(char* comando, int cantidadDeComandos){
	int success = -1;
	char* path = devolverRuta(comando, cantidadDeComandos);
		if (validarDirectorio(path)){
			success = remove(path);
			if (success == -1)
				printf("No se pudo eliminar el archivo");
			else
				printf("El archivo fue eliminado correctamente");
		}
		return success;
}

void listarArchivos(char* comando, int cantidadDeComandos){

	char* path = devolverRuta(comando, cantidadDeComandos);

	DIR * directorio;
	struct dirent * elemento;
	directorio = opendir(path);

	while ((elemento = readdir(directorio)) != NULL){
		if ((elemento->d_name) != (".") && (elemento->d_name) != ("..")){
			printf("%s\n", elemento->d_name);
		}
	}
	closedir(directorio);
}

int crearDirectorio(char* comando, int cantidadDeComandos){

	char* path = devolverRuta(comando, cantidadDeComandos);

	if (validarDirectorio(path)){
		return 2;
	}


	struct stat st = {0};

	if (stat(path, &st) == -1) {
	    mkdir(path, 0777);
	    return 1;
	}else{
		return 0;
	}
}

int mostrarArchivo(char* comando, int cantidadDeComandos){

	char* path = devolverRuta(comando, cantidadDeComandos);

	FILE *fd;
	int c;

	fd = fopen(path, "r");

	if(fd == NULL){
		return printf("Error al tratar de leer el archivo");
	}

	while ((c=fgetc(fd)) != EOF){
		putchar(c);
	}
	fclose(fd);
	return 1;
}

int cambiarNombre(char* comando, int cantidadDeComandos){

	char* rutaNombreViejo = devolverRuta(comando, cantidadDeComandos);
	char* nombreNuevo = devolverRuta(comando, cantidadDeComandos + 1);
	printf("--%s\n", rutaNombreViejo);
	printf("--%s\n", nombreNuevo);

	char* rutaNombreViejoReverse = strdup(string_reverse(rutaNombreViejo));
	printf("--%s\n", rutaNombreViejoReverse);
	int posicion = 0;
	int longitudNombreOriginal = 0;

	char* caracterActual = malloc(sizeof(char)*256);
	caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	printf("---%s\n", caracterActual);
	char* slash ="/";

	while(caracterActual != slash){

		++longitudNombreOriginal;
		++posicion;
		caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	}
	free(caracterActual);
	rutaNombreViejoReverse = string_substring_from(rutaNombreViejoReverse, longitudNombreOriginal + 1 );
	printf("--%s\n", rutaNombreViejoReverse);
	rutaNombreViejoReverse = strdup(string_reverse(rutaNombreViejoReverse));
	printf("--%s\n", rutaNombreViejoReverse);

	strcat(rutaNombreViejoReverse, nombreNuevo);
	char* rutaNombreNuevo = rutaNombreViejoReverse;
	printf("--%s\n", rutaNombreNuevo);

	if (rename(rutaNombreViejo,rutaNombreNuevo) == 0){
		return 1;
	}else{
		return 0;
	}

	return 0;
}

int mover(char* comando, int cantidadDeComandos){

	char* rutaNombreViejo = devolverRuta(comando, cantidadDeComandos);
	char* rutaNombreNuevo = devolverRuta(comando, (cantidadDeComandos + 1));

	if (rename(rutaNombreViejo,rutaNombreNuevo) == 0){
		free(rutaNombreViejo);
		free(rutaNombreNuevo);
		return 1;
	}else{
		free(rutaNombreViejo);
		free(rutaNombreNuevo);
		return 0;
	}
}

int informacion(char* comando, int cantidadDeComandos){

	char* path = devolverRuta(comando, cantidadDeComandos);

	struct stat fileStat;
		    if(stat(path,&fileStat) < 0)
		        return 0;

		    printf("Information for %s\n",path);
		    printf("---------------------------\n");
		    printf("File Size: \t\t%d bytes\n",(int) fileStat.st_size);
		    printf("Number of Links: \t%d\n",fileStat.st_nlink);
		    printf("File inode: \t\t%d\n",(int) fileStat.st_ino);
		    printf("Number of blocks: \t%d\n", (int) fileStat.st_blocks);

		    return 1;
}
