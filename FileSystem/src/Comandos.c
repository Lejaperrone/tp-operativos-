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

	char* rutaFSMetadata = buscarRutaArchivo(rutaFS);
	int numeroDirectorio = atoi(rutaFSMetadata);
	if (numeroDirectorio == -1){
		printf("No existe el directorio\n");
		return 0;
	}

	while(strcmp(caracterActual,dot)){
		memcpy(tipo + indice, caracterActual, 1);
		++indice;
		caracterActual = string_substring(rutaInvertida, indice, 1);
		if (strcmp(caracterActual,slash) == 0){
			printf("ruta invalida, no es un archivo");
			return 0;
		}

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
		free(tipo);
		free(nombre);
		return 0;
	}

	int fd = open(rutaNormal,O_RDWR);
	int size = fileStat.st_size;

	string* mapeoArchivo;

	mapeoArchivo = malloc(sizeof(string));
	mapeoArchivo->cadena = mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	mapeoArchivo->longitud = size;

	int resultado = guardarEnNodos(rutaFS, nombre, tipo, mapeoArchivo);

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

	return resultado;
}

int copiarArchivoAFs(char* comando){
	int respuesta = 1;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* directorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	if (atoi(buscarRutaArchivo(directorioYamafs)) == -1)
		return respuesta;

	char* contenido = leerArchivo(rutaArchivoYamafs);
	char* nombre = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirFs = devolverRuta(comando,2);

	char* rutaFinal = string_from_format("%s/%s", rutaDirFs, nombre);

	FILE* archivo = fopen(rutaFinal, nombre);
	fwrite(contenido, strlen(contenido), 1, archivo);

	fclose(archivo);
	free(rutaFinal);
	respuesta = 0;

	return respuesta;
}

int copiarBloqueANodo(char* comando){
	int respuesta = 1, bloqueNuevo, numeroCopiaBloqueNuevo;

	char* rutaArchivoYamafs = devolverRuta(comando,1);
	int bloqueACopiar = atoi(devolverRuta(comando,2));
	int nodoACopiar = atoi(devolverRuta(comando,3));

	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirectorioMetadata = buscarRutaArchivo(rutaSinArchivo(rutaArchivoYamafs));
	if (atoi(rutaDirectorioMetadata) == -1)
		return respuesta;

	char* rutaArchivoMetadata = string_from_format("%s/%s", rutaDirectorioMetadata, nombreArchivo);
	if (!validarArchivo(rutaArchivoMetadata))
		return respuesta;

	t_config* infoArchivo = config_create(rutaArchivoMetadata);

	bloqueNuevo = guardarBloqueEnNodo(bloqueACopiar, nodoACopiar, infoArchivo);

	if (bloqueNuevo == 1){
		printf("No existe el bloque");
		return respuesta;
	}

	numeroCopiaBloqueNuevo = obtenerNumeroCopia(infoArchivo, bloqueACopiar);

	config_set_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d", bloqueACopiar, numeroCopiaBloqueNuevo),
			string_from_format(generarArrayBloque(nodoACopiar,bloqueNuevo)));

	config_save(infoArchivo);

	actualizarBitmapNodos();
	respuesta = 0;

	return respuesta;
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
	int sizeArchivo, sizeAux, cantBloquesArchivo = 0, i, j, numeroNodo, bloqueNodo, respuesta;
	char** arrayInfoBloque;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirectorioYamafs = rutaSinArchivo(rutaArchivoYamafs);

	char* rutaMetadata = buscarRutaArchivo(rutaDirectorioYamafs);
	if (atoi(rutaMetadata) == -1)
		return 1;
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

	respuesta = system(command);

	free(rutaArchivoEnMetadata);
	free(command);

	return respuesta;
}

int eliminarDirectorio(char* comando){
	char* rutaDirectorioYamfs = devolverRuta(comando,2);
	char* rutaDirectorioMetadata = buscarRutaArchivo(rutaDirectorioYamfs);
	if (atoi(rutaDirectorioMetadata) == -1)
		return 2;
	int numeroTablaDirectorio = atoi(ultimaParteDeRuta(rutaDirectorioMetadata));

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

int eliminarBloque(char* comando){
	int respuesta = 1, otraCopia, numeroNodo, bloqueNodo;
	char** arrayInfoBloque;
	char* rutaArchivoYamafs = devolverRuta(comando,2);
	int numeroBloque = atoi(devolverRuta(comando,3));
	int numeroCopia = atoi(devolverRuta(comando,4));

	char* rutaDirectorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirectorioMetadata = buscarRutaArchivo(rutaDirectorioYamafs);

	if (atoi(rutaDirectorioMetadata) == -1)
		return respuesta;

	char* rutaArchivoEnMetadata = string_from_format("%s/%s", rutaDirectorioMetadata, nombreArchivo);

	if (!validarArchivo(rutaArchivoEnMetadata))
		return respuesta;

	t_config* infoArchivo = config_create(rutaArchivoEnMetadata);

	if (numeroCopia == 0)
		otraCopia = 1;
	else
		otraCopia = 0;

	if (config_has_property(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",numeroBloque,numeroCopia))
			&& config_has_property(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",numeroBloque,otraCopia))){
		arrayInfoBloque = config_get_array_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",numeroBloque,numeroCopia));
		numeroNodo = atoi(string_substring_from(arrayInfoBloque[0], 4));
		bloqueNodo = atoi(string_substring_from(arrayInfoBloque[1], 0));
		setearBloqueLibreEnBitmap(numeroNodo, bloqueNodo);
		actualizarBitmapNodos();
		respuesta = 0;
	}else{
		printf("El bloque no tiene ninguna copia en el FileSystem");
	}

	free(rutaArchivoEnMetadata);

	return respuesta;
}

int listarArchivos(char* comando){
	int respuesta = 1;
	char* rutaYamafs = devolverRuta(comando, 1);
	char* rutaFsLocal = buscarRutaArchivo(rutaYamafs);
	if (rutaFsLocal == string_itoa(-1))
		return respuesta;

	char* command = string_from_format("ls %s", rutaFsLocal);

	respuesta = system(command);
	free(command);


	return respuesta;
}

int crearDirectorio(char* comando){
	int respuesta = 1, i = 0;
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

	respuesta = getIndexDirectorio(path);
	//printf("success %d\n", respuesta);

	if (respuesta == -1){
		rutaPadre = rutaSinArchivo(path);
		indexPadre = getIndexDirectorio(rutaPadre);
		if (indexPadre == -1)
			printf("no existe ruta padre %d\n", respuesta);
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

	return 0;
}

int mostrarArchivo(char* comando){

	int respuesta = 1;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	if (buscarRutaArchivo(rutaSinArchivo(rutaArchivoYamafs)) == string_itoa(-1)){
		printf("No existe el directorio");
		return respuesta;
	}
	char* contenido = leerArchivo(rutaArchivoYamafs);
	printf("%s\n", contenido);
	respuesta = 0;

	return respuesta;
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
		free(rutaNombreViejoReverse);
		free(caracterActual);
		free(rutaNuevaDefinitiva);
		return 0;
	}else{
		free(rutaNombreViejoReverse);
		free(caracterActual);
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
	int respuesta = 1;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* directorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	if (atoi(buscarRutaArchivo(directorioYamafs)) == -1)
		return respuesta;
	char* contenido = leerArchivo(rutaArchivoYamafs);
	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);

	char* ubicacionArchivoTemporal = string_from_format("/tmp/%s", nombreArchivo);
	FILE* file = fopen(ubicacionArchivoTemporal, "w+");
	fwrite(contenido, sizeof(char), string_length(contenido), file);

	char* MD5 = string_from_format("md5sum /tmp/%s", nombreArchivo);
	char* RM = string_from_format("rm /tmp/%s", nombreArchivo);
	fclose(file);

	respuesta = system(MD5);
	printf("\n");
	respuesta = system(RM);
	free(MD5);
	free(RM);
	free(ubicacionArchivoTemporal);

	return respuesta;
}



int informacion(char* comando){
	int respuesta = 1;
	char* rutaArchivoYamafs = devolverRuta(comando, 1);
	char* rutaDirectorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirectorioMetadata = buscarRutaArchivo(rutaDirectorioYamafs);

	if (atoi(rutaDirectorioMetadata) == -1)
		return respuesta;

	char* rutaArchivoMetadata = string_from_format("%s/%s", rutaDirectorioMetadata, nombreArchivo);

	if (!validarArchivo(rutaArchivoMetadata))
		return respuesta;

	char* command = string_from_format("cat %s", rutaArchivoMetadata);

	respuesta = system(command);

	free(command);
	free(rutaArchivoMetadata);

	return respuesta;
}

int formatearFS(){


	return 0;
}
