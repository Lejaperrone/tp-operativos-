/*
 * Comandos.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#include "Comandos.h"
#include "FuncionesFS.h"

#define mb 1048576


char* devolverRuta(char* comando, int numeroParametro){
	char* copiaComando = malloc(strlen(comando)+1);
	memset(copiaComando,0, strlen(comando)+1);
	memcpy(copiaComando, comando,strlen(comando)+1);
	char* ruta = strtok(copiaComando, " ");
	int i;

	for (i = 0; i < numeroParametro; ++i){
		ruta = strtok(NULL, " ");
	}
	free(copiaComando);
	return ruta;
}

int copiarArchivo(char* comando){
	int indice = 0, indiceNom = 0;
	char* tipo = malloc(5); //.bin o .txt
	memset(tipo,0,5);
	char* rutaNormal = devolverRuta(comando, 1);
	char* rutaFS = devolverRuta(comando, 2);
	char* nombre = malloc(strlen(comando)-4); //El peor caso seria que el parametro sea el nombre sin ruta, tomo ese valor
	memset(nombre,0,strlen(comando)-4);
	char* rutaInvertida = string_reverse(rutaNormal);
	char* slash = "/";
	char* dot = ".";
	char* caracterActual = string_substring(rutaInvertida, indice, 1);

	while(strcmp(caracterActual,dot)){
		memcpy(tipo + indice, caracterActual, 1);
		++indice;
		caracterActual = string_substring(rutaInvertida, indice, 1);
	}

	memcpy(tipo + indice, caracterActual, 1);
	++indice;
	caracterActual = string_substring(rutaInvertida, indice, 1);

	tipo = string_reverse(tipo);

	while(strcmp(caracterActual,slash)){
		memcpy(nombre + indiceNom, caracterActual, 1);
		++indice;
		++indiceNom;
		caracterActual = string_substring(rutaInvertida, indice, 1);
	}

	nombre = string_reverse(nombre);

	printf("ruta normal %s\n", rutaNormal);

	struct stat fileStat;
	if(stat(rutaNormal,&fileStat) < 0){
		printf("no se pudo abrir\n");
		exit(1);
	}

	int fd = open(rutaNormal,O_RDWR);
	int size = fileStat.st_size;

	string* mapeoArchivo;

	mapeoArchivo = malloc(sizeof(string));
	mapeoArchivo->cadena = mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	mapeoArchivo->longitud = size;

	guardarEnNodos(rutaFS, nombre, tipo, mapeoArchivo);

	if (munmap(mapeoArchivo->cadena, mapeoArchivo->longitud) == -1)
	{
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}

	actualizarBitmapNodos();

	free(tipo);
	free(nombre);

	return 1;
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

int eliminarArchivo(char* comando){
	int success = 1;
	char* path = devolverRuta(comando, 1);
	if (!validarArchivo(path))
		return success;
	success = system(comando);
	return success;
}

int eliminarDirectorio(char* comando){
	int success = 1;
	char* path = devolverRuta(comando, 2);
	if (!validarDirectorio(path))
		return success;
	success = system(comando);
	return success;
}

int listarArchivos(char* comando){
	/*int success = 1;
	char* path = devolverRuta(comando, 1);

	if (!validarDirectorio(path))
		return success;

	success = system(comando);*/
	informacionNodo info = *(informacionNodo*)list_get(nodosConectados,0);
	int a = 1;
	char* b;
	b = leerArchivo("hola/chau/hola3.txt");

	return 0;
}

int crearDirectorio(char* comando){
	int success = 1;
	char* path = devolverRuta(comando, 1);

	if (validarDirectorio(path))
		return success;

	success = system(comando);

	return success;
}

int mostrarArchivo(char* comando){

	char* path = devolverRuta(comando, 1);
	int success = 1;
	if (!validarArchivo(path))
			return success;
	success = system(comando);
	printf("\n");

	return success;
}

int cambiarNombre(char* comando){

	char* rutaNombreViejo = devolverRuta(comando, 1);
	char* nombreNuevo = devolverRuta(comando, 2);

	char* rutaNombreViejoReverse = string_reverse(rutaNombreViejo);
	int posicion = 0;
	int longitudNombreOriginal = 0;

	char* caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	char* slash = "/";

	while(strcmp(caracterActual,slash)){

		++longitudNombreOriginal;
		++posicion;
		caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	}

	rutaNombreViejoReverse = string_substring_from(rutaNombreViejoReverse, longitudNombreOriginal + 1 );
	rutaNombreViejoReverse = string_reverse(rutaNombreViejoReverse);
	int tamanioRutaNueva = sizeof(rutaNombreViejoReverse) + sizeof(slash) + sizeof(nombreNuevo);
	char* rutaNuevaDefinitiva = malloc(tamanioRutaNueva + 1);
	memcpy(rutaNuevaDefinitiva, rutaNombreViejoReverse, strlen(rutaNombreViejoReverse));
	memcpy(rutaNuevaDefinitiva + strlen(rutaNombreViejoReverse), slash, strlen(slash));
	memcpy(rutaNuevaDefinitiva + strlen(rutaNombreViejoReverse) + strlen(slash), nombreNuevo, strlen(nombreNuevo) + 1);

	if (rename(rutaNombreViejo,rutaNuevaDefinitiva) == 0){
		free(rutaNuevaDefinitiva);
		return 0;
	}else{
		free(rutaNuevaDefinitiva);
		return 1;
	}
}

int mover(char* comando){

	char* rutaArchivoVieja = devolverRuta(comando, 1);
	int success = 1;

	if (!validarArchivo(rutaArchivoVieja))
		return success;

	success = system(comando);

	return success;
}

int generarArchivoMD5(char* comando){
	int success = 1;
	char* ruta = devolverRuta(comando,1);
	char* command = malloc(8 + strlen(ruta));
	memcpy(command, "md5sum ", 7);
	memcpy(command + 7, ruta, strlen(ruta)+1);

	success = system(command);
	printf("\n");
	free(command);

	return success;
}


int informacion(char* comando){
	int success = 1;
	char* path = devolverRuta(comando, 1);

	struct stat fileStat;
		    if(stat(path,&fileStat) < 0)
		        return success;
		    else
		    	success = 0;

		    printf("Information for %s\n",path);
		    printf("---------------------------\n");
		    printf("File Size: \t\t%d bytes\n",(int) fileStat.st_size);
		    printf("Number of Links: \t%d\n",fileStat.st_nlink);
		    printf("File inode: \t\t%d\n",(int) fileStat.st_ino);
		    printf("Number of blocks: \t%d\n", (int) fileStat.st_blocks);

		    return success;
}
