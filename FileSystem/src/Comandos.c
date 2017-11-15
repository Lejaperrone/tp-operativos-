/*
 * Comandos.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#include "Comandos.h"
#include "FuncionesFS.h"

#define mb 1048576

char* devolverRuta(char* comando, int posicionPalabra){
	char* copiaComando = malloc(strlen(comando)+1);
	memset(copiaComando,0, strlen(comando)+1);
	memcpy(copiaComando, comando,strlen(comando)+1);
	char* ruta = strtok(copiaComando, " ");
	int i;

	for (i = 0; i < posicionPalabra; ++i){
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

	char* rutaMetadata = buscarRutaArchivo(rutaFS);
	if (strcmp(rutaMetadata, "-1") == 0)
		return 0;

	printf("%s\n", rutaFS);
	if (!string_starts_with(rutaFS,"yamafs:/"))
		return 0;

	char* rutaFSMetadata = buscarRutaArchivo(rutaSinPrefijoYama(rutaFS));
	if (strcmp(rutaFSMetadata, "-1") == 0){
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
	char* rutaMetadata = buscarRutaArchivo(directorioYamafs);
	if (strcmp(rutaMetadata, "-1") == -0)
		return respuesta;

	if(!validarArchivo(string_from_format("%s/%s", rutaMetadata,ultimaParteDeRuta(rutaArchivoYamafs))))
		return 1;

	char* contenido = leerArchivo(rutaArchivoYamafs);
	char* nombre = ultimaParteDeRuta(rutaArchivoYamafs);
	char* rutaDirFs = devolverRuta(comando,2);

	char* rutaFinal = string_from_format("%s/%s", rutaDirFs, nombre);

	FILE* archivo = fopen(rutaFinal, "w");
	fwrite(contenido, strlen(contenido), 1, archivo);

	fclose(archivo);
	free(contenido);
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
	char* rutaDirectorioYamafs = rutaSinArchivo(rutaArchivoYamafs);

	char* rutaMetadata = buscarRutaArchivo(rutaDirectorioYamafs);
	if (strcmp(rutaMetadata, "-1") == 0)
		return 1;

	if (!string_starts_with(rutaArchivoYamafs,"yamafs:/"));
		return 2;

	char* nombreArchivo = ultimaParteDeRuta(rutaArchivoYamafs);

	if(!string_contains(nombreArchivo, "."))
		return 3;

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
	if (strcmp(rutaDirectorioMetadata, "-1") == 0)
		return 2;
	int numeroTablaDirectorio = atoi(ultimaParteDeRuta(rutaDirectorioMetadata));

	if(numeroTablaDirectorio == 0){
		printf("no se puede eliminar root\n");
		return 3;
	}

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

	if (strcmp(rutaDirectorioMetadata, "-1") == 0)
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
	int success = 1;

	if (validarArchivoYamaFS(pathComando) == 0){
		printf("no se creo el directorio, ruta invalida\n");
		return 2;
	}

	path = rutaSinPrefijoYama(pathComando);
	if (strcmp("/", path) == 0){
		printf("no se creo el directorio, el directorio no puede ser root\n");
		return 2;
	}

	respuesta = getIndexDirectorio(path);
	//printf("success %d\n", respuesta);

	if (respuesta == -1){
		rutaPadre = rutaSinArchivo(path);
		indexPadre = getIndexDirectorio(rutaSinPrefijoYama(rutaPadre));
		if (indexPadre == -1){
			printf("no existe ruta padre %d\n", respuesta);
			return 1;
		}
		else{
			while(tablaDeDirectorios[i].index != -1){
				++i;
			}
			success = 0;
			tablaDeDirectorios[i].index = i;
			tablaDeDirectorios[i].padre = indexPadre;
			nombre = ultimaParteDeRuta(path);
			memcpy(tablaDeDirectorios[i].nombre, nombre, strlen(nombre));
			guardarTablaDirectorios();
		}

	}
	else
		printf("ya existe el directorio\n");

	return success;
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


	int posicion = 0;
	int longitudNombreOriginal = 0;

	if (!string_starts_with(rutaNombreViejo,"yamafs:/"))
		return 2;

	rutaNombreViejo = rutaSinPrefijoYama(rutaNombreViejo);

	if(!string_contains(nombreNuevo, "."))
		return 3;

	char* rutaNombreViejoReverse = string_reverse(rutaNombreViejo);
	char* caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	char* slash = "/";

	while(strcmp(caracterActual,slash)){
		++longitudNombreOriginal;
		++posicion;
		caracterActual = string_substring(rutaNombreViejoReverse, posicion, 1);
	}
	char* rutaSinNombre = string_substring(rutaNombreViejo, 0, strlen(rutaNombreViejo) - posicion);
	char* nombre = string_substring(rutaNombreViejo, strlen(rutaNombreViejo) - posicion, posicion);

	char* ruta = buscarRutaArchivo(rutaSinNombre);

	if(strcmp(ruta, "-1") == 0)
		return 2;

	rutaNombreViejoReverse = string_substring_from(rutaNombreViejoReverse, longitudNombreOriginal + 1 );
	rutaNombreViejoReverse = string_reverse(rutaNombreViejoReverse);
	int tamanioRutaNueva = strlen(rutaNombreViejoReverse) + strlen(slash) + strlen(nombreNuevo);
	char* rutaNuevaDefinitiva = malloc(tamanioRutaNueva + 1);
	memset(rutaNuevaDefinitiva, 0, tamanioRutaNueva + 1);
	memcpy(rutaNuevaDefinitiva, rutaNombreViejoReverse, strlen(rutaNombreViejoReverse));
	memcpy(rutaNuevaDefinitiva + strlen(rutaNombreViejoReverse), slash, strlen(slash));
	memcpy(rutaNuevaDefinitiva + strlen(rutaNombreViejoReverse) + strlen(slash), nombreNuevo, strlen(nombreNuevo) + 1);

	char* nuevo = string_from_format("%s/%s", ruta, nombreNuevo);

	if (!validarArchivo(nuevo))
		return 0;

	char* nombreViejo = string_from_format("%s/%s", ruta, nombre);

	t_config* c = config_create(nombreViejo);
	config_set_value(c, "RUTA", string_from_format("%s%s", rutaSinNombre, nombre));
	config_save_in_file(c, nombreViejo);
	config_destroy(c);

	int resultado = rename(nombreViejo,nuevo);

	free(rutaNombreViejoReverse);
	free(caracterActual);
	free(rutaNuevaDefinitiva);
	free(nuevo);
	free(nombre);
	free(nombreViejo);
	free(rutaSinNombre);

	return resultado;
}

int esRutaDeYama(char* ruta){
	return string_starts_with(ruta, "yamafs:/");
}

int mover(char* comando){
	char* slash = "/";
	char* dot = ".";

	char** arguments = string_split(comando, " ");

	if(strcmp(arguments[1], "yamafs:/") == 0)
		return 3;

	if(!esRutaDeYama(arguments[1]) || !esRutaDeYama(arguments[2]))
		return 2;

	int indice = 0;
	char* rutaInvertida = string_reverse(arguments[1]);
	char* caracterActual = string_substring(rutaInvertida, indice, 1);
	int tipo = 0; //archivo

	while(strcmp(caracterActual,dot) != 0 && tipo == 0){
		++indice;
		caracterActual = string_substring(rutaInvertida, indice, 1);
		if (strcmp(caracterActual,slash) == 0){
			tipo = 1; //directorio
		}
	}

	int length = 0;
	if(strcmp(arguments[2], "yamafs:/") != 0){
		char* rutaInvertidaNuevaDir = string_reverse(arguments[2]);
		while(strcmp(caracterActual,slash) ){
			++length;
			caracterActual = string_substring(rutaInvertidaNuevaDir, length, 1);
			if (strcmp(caracterActual,dot) == 0){
				printf("ruta invalida, es un archivo");
				return 3;
			}
		}
	}

	int success = 0;
	if(tipo == 1){
		int indexDir = getIndexDirectorio(rutaSinPrefijoYama(arguments[1]));
		int indexDirPadre = getIndexDirectorio(rutaSinPrefijoYama(arguments[2]));
		if (indexDir == -1 || indexDirPadre == -1)
			return 2;
		tablaDeDirectorios[indexDir].padre = indexDirPadre;
		success = 0;
		guardarTablaDirectorios();
	}
	else{
		char* rutaAnterior;
		char* rutaNueva = buscarRutaArchivo(arguments[2]);
		rutaAnterior = buscarRutaArchivo(rutaSinArchivo(arguments[1]));
		if(strcmp(rutaAnterior, "yamafs:") == 0){
			rutaAnterior = string_from_format("%s/",rutaAnterior);
		}

		char** partesRutaAnterior = string_split(arguments[1],"/");
		int i = 0;
		while(partesRutaAnterior[i] != NULL)
			++i;
		char* nombreArchivo = partesRutaAnterior[i-1];
		char* rutaFinalAnterior = string_from_format("%s/%s", rutaAnterior, nombreArchivo);
		if (!validarArchivo(rutaFinalAnterior))
			return 3;
		t_config* c = config_create(rutaFinalAnterior);
		config_set_value(c, "RUTA", string_from_format("%s/%s", arguments[2], nombreArchivo));
		config_save_in_file(c, rutaFinalAnterior);
		config_destroy(c);
		success = system(string_from_format("mv %s %s", rutaFinalAnterior, rutaNueva));
	}

	return success;
}

int generarArchivoMD5(char* comando){
	int respuesta = 1;
	char* rutaArchivoYamafs = devolverRuta(comando,1);
	char* directorioYamafs = rutaSinArchivo(rutaArchivoYamafs);
	if (strcmp(buscarRutaArchivo(directorioYamafs), "-1") == 0){
		return respuesta;
	}
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
	int resultado = 0;

	resultado += borrarDirectorios();

	resultado += borrarArchivosEnMetadata();

	resultado += liberarNodosConectados();

	resultado += formatearDataBins();

	if (resultado == 4)
		return 0;

	return 1;
}
