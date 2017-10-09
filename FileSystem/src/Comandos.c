/*
 * Comandos.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#include "Comandos.h"
#include "FuncionesFS.h"

#define mb 1048576

bool CalcFileMD5(char *file_name, char *md5_sum)
{
    #define MD5SUM_CMD_FMT "md5sum %." STR(PATH_LEN) "s 2>/dev/null"
    char cmd[PATH_LEN + sizeof (MD5SUM_CMD_FMT)];
    sprintf(cmd, MD5SUM_CMD_FMT, file_name);
    #undef MD5SUM_CMD_FMT

    FILE *p = popen(cmd, "r");
    if (p == NULL) return false;

    int i, ch;
    for (i = 0; i < MD5_LEN && isxdigit(ch = fgetc(p)); i++) {
        *md5_sum++ = ch;
    }

    *md5_sum = '\0';
    pclose(p);
    return i == MD5_LEN;
}

char* devolverRuta(char* comando, int numeroParametro)
{
	char* copiaComando = malloc(strlen(comando)+1);
	memcpy(copiaComando, comando,strlen(comando)+1);
	char* ruta = strtok(copiaComando, " ");
	int i;

	for (i = 0; i < numeroParametro; ++i){
		ruta = strtok(NULL, " ");
	}
	//free(copiaComando);
	return ruta;
}

int copiarArchivo(char* comando){
	int indice = 0, indiceNom = 0;;
	int mockSizeArchivo = 1024*1024*2;
	char* tipo = malloc(5); //.bin o .txt
	char* rutaNormal = devolverRuta(comando, 1);
	char* rutaFS = devolverRuta(comando, 2);
	char* nombre = malloc(strlen(comando)-4); //El peor caso seria que el parametro sea el nombre sin ruta, tomo ese valor
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
	if(stat(rutaNormal,&fileStat) < 0)
		exit(1);

	int fd = open(rutaNormal,O_RDWR);
	int size = fileStat.st_size;

	string* mapeoArchivo;

	mapeoArchivo = malloc(sizeof(string));
	mapeoArchivo->cadena = mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	mapeoArchivo->longitud = size;

	guardarEnNodos(rutaFS, nombre, tipo, mapeoArchivo);

	free(tipo);
	free(nombre);

	return 1;
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

int eliminarArchivo(char* comando){
	int success = -1;
	char* path = devolverRuta(comando, 1);
	if (validarArchivo(path)){
		success = remove(path);
		if (success == -1)
			printf("No se pudo eliminar el archivo");
		else
			printf("El archivo fue eliminado correctamente");
	}
	return success;
}

int eliminarDirectorio(char* comando){
	int success = -1;
	char* path = devolverRuta(comando, 2);
		if (validarDirectorio(path)){
			success = remove(path);
			if (success == -1)
				printf("No se pudo eliminar el archivo");
			else
				printf("El archivo fue eliminado correctamente");
		}
		return success;
}

int listarArchivos(char* comando){

	char* path = devolverRuta(comando, 1);

	if (!validarDirectorio(path))
		return 0;

	DIR * directorio;
	struct dirent * elemento;
	directorio = opendir(path);

	while ((elemento = readdir(directorio)) != NULL){
		if ((elemento->d_name) != (".") && (elemento->d_name) != ("..")){
			printf("%s\n", elemento->d_name);
		}
	}
	closedir(directorio);
	return 1;
}

int crearDirectorio(char* comando){

	char* path = devolverRuta(comando, 1);

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

int mostrarArchivo(char* comando){

	char* path = devolverRuta(comando, 1);

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

int cambiarNombre(char* comando){

	char* rutaNombreViejo = devolverRuta(comando, 1);
	char* nombreNuevo = devolverRuta(comando, 2);
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

int mover(char* comando){

	char* rutaNombreViejo = devolverRuta(comando, 1);
	char* rutaNombreNuevo = devolverRuta(comando, 2);
	printf("--%s\n",rutaNombreViejo);
	printf("--%s\n",rutaNombreNuevo);

	if (rename(rutaNombreViejo,rutaNombreNuevo) == 0){
		//free(rutaNombreViejo);
		//free(rutaNombreNuevo);
		return 1;
	}else{
		//free(rutaNombreViejo);
		//free(rutaNombreNuevo);
		return 0;
	}
}

int generarArchivoMD5(char* comando){

	char* path = devolverRuta(comando, 1);
	char md5[MD5_LEN + 1];

	    if (!CalcFileMD5(path, md5)) {
	        return 0;
	    } else {
	        printf("MD5 sum es: %s\n", md5);
	    }
	return 1;
}


int informacion(char* comando){

	char* path = devolverRuta(comando, 1);

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
