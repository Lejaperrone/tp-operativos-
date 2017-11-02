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
	//free(copiaComando);
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
	free(mapeoArchivo);

	return 1;
}

int copiarArchivoAFs(char* comando){
	int success = 1;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* directorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	if (string_starts_with(string_reverse(buscarRutaArchivo(directorioYamafs)), "-1"))
		return success;

	char* contenido = leerArchivo(rutaArchivoYamafs);
	printf("%s\n", contenido);
	char* nombre = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirFs = devolverRuta(comando,2);
	char* rutaFinal = malloc(strlen(rutaDirFs) + strlen(nombre) + 2);
	memset(rutaFinal, 0, strlen(rutaDirFs) + strlen(nombre) + 2);
	memcpy(rutaFinal, rutaDirFs, strlen(rutaDirFs));
	memcpy(rutaFinal + strlen(rutaDirFs), "/", 1);
	memcpy(rutaFinal + strlen(rutaDirFs) + 1, nombre, strlen(nombre));

	FILE* archivo = fopen(rutaFinal,"a");
	fwrite(contenido, sizeof(contenido), strlen(contenido), archivo);

	fclose(archivo);
	free(rutaFinal);
	success = 0;

	return success;
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

bool isDirectoryEmpty(char *dirname) {
  int n = 0;
  struct dirent *d;
  DIR *dir = opendir(dirname);
  if (dir == NULL)
    return false;
  while ((d = readdir(dir)) != NULL) {
    if(++n > 2)
      break;
  }
  closedir(dir);

  return n <= 2;
}

int eliminarArchivo(char* comando){
	int sizeArchivo, sizeAux, cantBloquesArchivo = 0, i, j, numeroNodo, bloqueNodo, success;
	char** arrayInfoBloque;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirectorioYamafs = rutaSinArchivo(rutaArchivoYamafs);

	char* rutaMetadata = buscarRutaArchivo(rutaDirectorioYamafs);
	char* rutaArchivoEnMetadata = malloc(strlen(rutaMetadata) + strlen(nombreArchivo) + 2);
	memset(rutaArchivoEnMetadata,0,strlen(rutaMetadata) + strlen(nombreArchivo) + 2);
	memcpy(rutaArchivoEnMetadata, rutaMetadata, strlen(rutaMetadata));
	memcpy(rutaArchivoEnMetadata + strlen(rutaMetadata), "/", 1);
	memcpy(rutaArchivoEnMetadata + strlen(rutaMetadata) + 1, nombreArchivo, strlen(nombreArchivo));

	if (!validarArchivo(rutaArchivoEnMetadata))
		return 1;

	t_config* infoArchivo = config_create(rutaArchivoEnMetadata);

	if (config_has_property(infoArchivo, "TAMANIO"))
		sizeArchivo = config_get_int_value(infoArchivo,"TAMANIO");

	sizeAux = sizeArchivo;

	while(sizeAux > 0){
		sizeAux -= mb;
		++cantBloquesArchivo;
	}

	for (i = 0; i < cantBloquesArchivo; ++i){
		for (j = 0; j < numeroCopiasBloque; ++j){
			if (config_has_property(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i,j))){
				arrayInfoBloque = config_get_array_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i,j));
				numeroNodo = atoi(string_substring_from(arrayInfoBloque[0], 4));
				bloqueNodo = atoi(string_substring_from(arrayInfoBloque[1], 0));
				setearBloqueLibreEnBitmap(numeroNodo, bloqueNodo);
			}
		}
	}

	actualizarBitmapNodos();

	char* command = malloc(strlen(rutaArchivoEnMetadata) + 4);
	memset(command, 0, strlen(rutaArchivoEnMetadata) + 4);
	memcpy(command, "rm ", 3);
	memcpy(command + 3, rutaArchivoEnMetadata, strlen(rutaArchivoEnMetadata));

	success = system(command);

	return success;
}

int eliminarDirectorio(char* comando){
	char* rutaDirectorioYamfs = devolverRuta(comando,2);
	char* rutaDirectorioMetadata = buscarRutaArchivo(rutaDirectorioYamfs);
	int numeroTablaDirectorio = atoi(ultimaParteDeRuta(rutaDirectorioMetadata));

	if (numeroTablaDirectorio == -1)
		return 2;

	if (isDirectoryEmpty(rutaDirectorioMetadata)){
		tablaDeDirectorios[numeroTablaDirectorio].index = -1;
		tablaDeDirectorios[numeroTablaDirectorio].padre = -1;
		memcpy(tablaDeDirectorios[numeroTablaDirectorio].nombre," ",1);

		return 0;
	}else{
		printf("El directorio no esta vacio");
		return 1;
	}
}

int listarArchivos(char* comando){
	//printf("%s",leerArchivo("/hola/hola3.txt"));
	FILE* asd = fopen("/home/utnso/test.txt", "r+");
	char * a = "a";
	fwrite(a,1,mb,asd);
	fwrite("b",1,mb,asd);
	fwrite("c",1,mb,asd);
	fwrite("d",1,2,asd);
	fclose(asd);
	int success = 1;
	/*char* rutaYamafs = devolverRuta(comando, 1);
	char* rutaFsLocal = buscarRutaArchivo(rutaYamafs);
	if (rutaFsLocal == string_itoa(-1))
		return success;

	char* command = malloc(strlen(rutaFsLocal) + 4);
	memset(command, 0,strlen(rutaFsLocal) + 4);
	memcpy(command, "ls ", 3);
	memcpy(command + 3, rutaFsLocal, strlen(rutaFsLocal));

	success = system(command);*/

	return success;
}

int crearDirectorio(char* comando){
	int success = 1, i = 0;
	char* pathComando = devolverRuta(comando, 1);
	char* path;
	char* rutaPadre;
	int indexPadre = 0;
	char* nombre;

	if (validarArchivoYamaFS(pathComando) == 0){
		printf("no se creo el directorio, ruta invalida\n");
		return 0;
	}

	path = rutaSinPrefijoYama(pathComando);
	if (strcmp("/", path) == 0){
		printf("no se creo el directorio, el directorio no puede ser root\n");
		return 0;
	}

	success = getIndexDirectorio(path);
	printf("success %d\n", success);

	if (success == -1){
		rutaPadre = rutaSinArchivo(path);
		indexPadre = getIndexDirectorio(rutaPadre);
		if (indexPadre == -1)
			printf("no existe ruta padre %d\n", success);
		else{
			while(tablaDeDirectorios[i].index != -1){
				++i;
			}
			tablaDeDirectorios[i].index = i;
			tablaDeDirectorios[i].padre = indexPadre;
			nombre = ultimaParteDeRuta(path);
			memcpy(tablaDeDirectorios[i].nombre, nombre, strlen(nombre));
			guardarTablaDirectorios();
		}

	}
	else
		printf("ya existe el directorio\n");


	//success = system(comando);

	return 0;
}

int mostrarArchivo(char* comando){

	int success = 1;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	if (buscarRutaArchivo(rutaArchivoYamafs) == string_itoa(-1))
		return success;
	char* contenido = leerArchivo(rutaArchivoYamafs);
	printf("%s\n", contenido);

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
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* directorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	if (string_starts_with(string_reverse(buscarRutaArchivo(directorioYamafs)), "-1"))
		return success;
	char* contenido = leerArchivo(rutaArchivoYamafs);
	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);

	FILE* archivo = fopen(nombreArchivo, "w");
	fwrite(contenido, strlen(contenido), 1, archivo);

	char* MD5 = malloc(8 + strlen(nombreArchivo));
	memcpy(MD5, "md5sum ", 7);
	memcpy(MD5 + 7, nombreArchivo, strlen(nombreArchivo)+1);

	char* RM = malloc(4 + strlen(nombreArchivo));
	memcpy(RM, "rm ", 3);
	memcpy(RM + 3, nombreArchivo, strlen(nombreArchivo)+1);

	printf("%s\n", MD5);
	printf("%d-a-a-", strlen(contenido));
	success = system(MD5);
	printf("\n");
	fclose(archivo);
	success = system(RM);
	free(MD5);
	free(RM);

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

int formatearFS(char* comando){
	char* rutaArchivoEnMetadata = "/home/utnso/project/tp-2017-2c-PEQL/FileSystem/metadata/Archivos/11/utnso.txt";
	char* command = malloc(strlen(rutaArchivoEnMetadata) + 4);
	memset(command, 0, strlen(rutaArchivoEnMetadata) + 4);
	memcpy(command, "rm ", 3);
	memcpy(command + 3, rutaArchivoEnMetadata, strlen(rutaArchivoEnMetadata));

	printf("%s\n", command);

	return 0;
}
