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
#include "Serializacion.h"
#include <dirent.h>
#include <errno.h>
#include "Serial.h"
#include <Globales.h>
#include <math.h>

char* pathArchivoDirectorios = "../metadata/Directorios.dat";
struct sockaddr_in direccionCliente;
unsigned int tamanioDireccion = sizeof(direccionCliente);
int servidorFS;
extern sem_t pedidoLecturaFS[];
int sizeBloque;

void establecerServidor(){
	struct sockaddr_in direccionServidor = cargarDireccion("127.0.0.1",7000);
	int activado = 1;
	setsockopt(servidorFS, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	asociarSocketA(direccionServidor, servidorFS);
}

int recibirConexionYama(){
	respuesta respuestaId;
	while(1){
		int cliente =0;

		if((cliente = accept(servidorFS, (struct sockaddr *)&direccionCliente, &tamanioDireccion)) != -1){
			printf("asdasd\n");
			respuestaId = desempaquetar(cliente);
			int id = *(int*)respuestaId.envio;
			if(id == 1){//yama
				log_trace(loggerFS, "Nueva Conexion de Yama");
				empaquetar(cliente,mensajeOk,0,0);
				return cliente;
			}
		}
	}
}

void inicializarTablaDirectorios(){
	int i;
	struct stat fileStat;
	FILE* archivoDirectorios;

	tablaDeDirectorios[0].index = 0;
	tablaDeDirectorios[0].padre = -1;
	memcpy(tablaDeDirectorios[0].nombre,"/",1);

	for (i = 1; i < 100; ++i){
		tablaDeDirectorios[i].index = -1;
		tablaDeDirectorios[i].padre = -1;
		memcpy(tablaDeDirectorios[i].nombre," ",1);
	}

	archivoDirectorios = fopen(pathArchivoDirectorios, "ab");
	fclose(archivoDirectorios);

	int leido;
	if(stat(pathArchivoDirectorios,&fileStat) < 0){
		printf("no se pudo abrir\n");
		exit(1);
	}

	if (fileStat.st_size != 0){
		archivoDirectorios = fopen(pathArchivoDirectorios, "r");
		leido = fread(tablaDeDirectorios,sizeof(t_directory), cantidadDirectorios, archivoDirectorios);
		fclose(archivoDirectorios);
	}
	else
		leido = 0;

	if (leido == cantidadDirectorios)
		log_trace(loggerFS, "Se cargaron los directorios correctamente");
	else
		log_trace(loggerFS, "No se pudieron cargar todos los directorios");
}

void guardarTablaDirectorios(){
	int escrito;
	FILE* archivoDirectorios = fopen(pathArchivoDirectorios, "w");
	escrito = fwrite(tablaDeDirectorios,sizeof(t_directory), cantidadDirectorios, archivoDirectorios);
	if (escrito == cantidadDirectorios)
		log_trace(loggerFS, "Se escribieron los directorios correctamente");
	else
		log_trace(loggerFS, "No se pudieron escribir todos los directorios");
	fclose(archivoDirectorios);
}

int getIndexDirectorio(char* ruta){
	int index = 0, i = 0, j = 0, indexFinal = 0;

	if(strcmp(ruta, "/") == 0) //si la ruta es root devuelvo 0
		return 0;

	char* rutaSinRoot = string_substring_from(ruta,1);
	char** arrayPath = string_split(rutaSinRoot, "/"); //le saco root a la ruta
	char* arrayComparador[100];
	while(arrayPath[index] != NULL){ // separo por '/' la ruta en un array
		arrayComparador[index] = malloc(strlen(arrayPath[index])+1);
		memset(arrayComparador[index],0 ,strlen(arrayPath[index])+1);
		memcpy(arrayComparador[index],arrayPath[index],strlen(arrayPath[index]));
		++index; // guardo cuantas partes tiene el array
	}
	indexFinal = index;
	int indexDirectorios[index];

	for (i = 0; i <= index; ++i)
		indexDirectorios[i] = -1; // inicializo todo en -1, si al final me queda alguno en algun lado del array, significa que la ruta que pase no existe
	--index;

	indexDirectorios[0] = 0;

	for(i = 0; i <= index; --index){
		for(j = 0; j < 100; ++j){ // busco en la tabla que posicion coincide con el fragmento de ruta del loop
			if (strcmp(tablaDeDirectorios[j].nombre,arrayComparador[index]) == 0){ // me fijo si el padre de ese es el mismo que aparece en el fragmento anterior de la ruta
				if ((index != 0 && strcmp(tablaDeDirectorios[tablaDeDirectorios[j].padre].nombre,arrayComparador[index-1]) == 0) || (index == 0 && tablaDeDirectorios[j].padre == 0)){ // si es asi lo guardo
					indexDirectorios[index+1] = tablaDeDirectorios[j].index;
				}
			}
		}
	}

	for (i = 0; i < indexFinal; ++i)
		if (indexDirectorios[i] == -1){
			for (j = 0; j < indexFinal; ++j){
				free(arrayComparador[j]);
			}
			return -1;
		} //me fijo si quedo algun -1, si es asi no existe la ruta

	for (i = 0; i < indexFinal; ++i){
		free(arrayComparador[i]);
	}
	return indexDirectorios[indexFinal]; // devuelvo el index de la ruta
}

char* buscarRutaArchivo(char* ruta){
	ruta = rutaSinPrefijoYama(ruta);
	int indexDirectorio = getIndexDirectorio(ruta);
	char* numeroIndexString = string_itoa(indexDirectorio);
	char* rutaGenerada = calloc(1,strlen(rutaArchivos) + strlen(numeroIndexString) + 1);
	memset(rutaGenerada,0,strlen(rutaArchivos) + strlen(numeroIndexString) + 1);
	memcpy(rutaGenerada, rutaArchivos, strlen(rutaArchivos));
	memcpy(rutaGenerada + strlen(rutaArchivos), numeroIndexString, strlen(numeroIndexString));
	return rutaGenerada; //poner free despues de usar
}

int nodoRepetido(informacionNodo info){
	int cantidadNodos = list_size(nodosConectados);
	int i = 0, repetido = 0;
	informacionNodo infoAux;
	for (i = 0; i < cantidadNodos; ++i){
		infoAux = *(informacionNodo*)list_get(nodosConectados,i);
		if (infoAux.numeroNodo == info.numeroNodo)
			repetido = 1;
	}
	return repetido;
}

int validarArchivoYamaFS(char* ruta){
	if (string_starts_with(ruta,"yamafs:/")){
		printf("ruta valida\n");
		return 1;
	}
	printf("ruta invalida\n");
	return 0;
}

char* rutaSinPrefijoYama(char* ruta){
	if (validarArchivoYamaFS(ruta) == 0)
		return ruta;
	else
		return string_substring_from(ruta, 7);
}

char* rutaSinArchivo(char* rutaArchivo){
	int index = 0, cantidadPartesRuta = 0;
	char* rutaInvertida = string_reverse(rutaArchivo);
	char* rutaFinal = malloc(strlen(rutaArchivo)+1);
	char* currentChar = malloc(2);
	char* nombreInvertido = malloc(strlen(rutaArchivo)+1);
	char** arrayPath = string_split(rutaArchivo, "/");

	while(arrayPath[cantidadPartesRuta] != NULL)
		++cantidadPartesRuta;

	if (cantidadPartesRuta == 1)
		return "/";

	memset(nombreInvertido,0,strlen(rutaArchivo)+1);
	memset(rutaFinal,0,strlen(rutaArchivo)+1);
	memset(currentChar,0,2);

	currentChar = string_substring(rutaInvertida, index, 1);
	while(strcmp(currentChar,"/")){
		memcpy(nombreInvertido + index, currentChar, 1);
		++index;
		currentChar = string_substring(rutaInvertida, index, 1);
	}


	memcpy(rutaFinal, rutaArchivo, strlen(rutaArchivo)-index-1);

	printf("ruta sin archivo %s\n", rutaFinal);

	//free(rutaFinal);
	free(currentChar);
	free(nombreInvertido);

	return rutaFinal;
}

char* ultimaParteDeRuta(char* rutaArchivo){
	int index = 0;
	char* rutaInvertida = string_reverse(rutaArchivo);
	char* currentChar = malloc(2);
	char* nombreInvertido = malloc(strlen(rutaArchivo)+1);

	memset(nombreInvertido,0,strlen(rutaArchivo)+1);
	memset(currentChar,0,2);

	currentChar = string_substring(rutaInvertida, index, 1);
	while(strcmp(currentChar,"/")){
		memcpy(nombreInvertido + index, currentChar, 1);
		++index;
		currentChar = string_substring(rutaInvertida, index, 1);
	}

	free(currentChar);

	char* nombre = string_reverse(nombreInvertido);
	free(nombreInvertido);

	return nombre;
}

char* leerArchivo(char* rutaArchivo){
	int index = 0, sizeArchivo, cantBloquesArchivo = 0, sizeAux = 0, i, j, k, l;
	int cantidadNodos = list_size(nodosConectados);
	int cargaNodos[cantidadNodos], indexNodos[cantidadNodos], peticiones[cantidadNodos];
	int numeroNodoDelBloque[numeroCopiasBloque], posicionCopiaEnIndexNodo[numeroCopiasBloque];
	informacionNodo info;
	char* rutaInvertida = string_reverse(rutaArchivo);
	char* rutaFinal = malloc(strlen(rutaArchivo)+1);
	char* currentChar = malloc(2);
	char* nombreInvertido = malloc(strlen(rutaArchivo)+1);
	char** arrayInfoBloque;
	int posicionNodoAPedir = -1;
	int numeroBloqueDataBin = -1;

	memset(nombreInvertido,0,strlen(rutaArchivo)+1);
	memset(rutaFinal,0,strlen(rutaArchivo)+1);
	memset(currentChar,0,2);

	currentChar = string_substring(rutaInvertida, index, 1);
	while(strcmp(currentChar,"/")){
		memcpy(nombreInvertido + index, currentChar, 1);
		++index;
		currentChar = string_substring(rutaInvertida, index, 1);
	}


	char* nombre = malloc(index + 1);
	memset(nombre,0,index + 1);
	memcpy(nombre, string_reverse(nombreInvertido), index);

	memcpy(rutaFinal, rutaArchivo, strlen(rutaArchivo)-index);

	char* rutaMetadata = buscarRutaArchivo(rutaFinal);
	char* rutaArchivoEnMetadata = malloc(strlen(rutaArchivos) + strlen(nombre) + 2);
	memset(rutaArchivoEnMetadata,0,strlen(rutaArchivos) + strlen(nombre) + 2);
	memcpy(rutaArchivoEnMetadata, rutaMetadata, strlen(rutaMetadata));
	memcpy(rutaArchivoEnMetadata + strlen(rutaMetadata), "/", 1);
	memcpy(rutaArchivoEnMetadata + strlen(rutaMetadata) + 1, nombre, strlen(nombre));

	t_config* infoArchivo = config_create(rutaArchivoEnMetadata);
	FILE* informacionArchivo = fopen(rutaArchivoEnMetadata,"r");

	if (config_has_property(infoArchivo, "TAMANIO")){
		sizeArchivo = config_get_int_value(infoArchivo,"TAMANIO");
		printf("size %d \n",sizeArchivo);
	}

	char* lectura = malloc(sizeArchivo + 1);
	memset(lectura, 0, sizeArchivo + 1);

	sizeAux = sizeArchivo;

	while(sizeAux > 0){
		sizeAux -= mb;
		++cantBloquesArchivo;
	}
	char* respuestas[cantBloquesArchivo];
	parametrosLecturaBloque params[cantBloquesArchivo];
	for (j = 0; j < cantBloquesArchivo; ++j){
		respuestas[j] = malloc(mb+1);
		memset(respuestas[j], 0, mb + 1);
	}
	pthread_t nuevoHilo[cantBloquesArchivo];

	for (j = 0; j < cantidadNodos; ++j){
		info = *(informacionNodo*)list_get(nodosConectados, j);
		indexNodos[j] = info.numeroNodo;
		cargaNodos[j] = 0;
		peticiones[j] = 0;
	}

	for (i = 0; i < cantBloquesArchivo; ++i){
		for (l = 0; l < numeroCopiasBloque; ++l){
			arrayInfoBloque = config_get_array_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i,l));
			numeroNodoDelBloque[l] = atoi(string_substring_from(arrayInfoBloque[0], 4));
			for (k = 0; k < cantidadNodos; ++k)
				if (indexNodos[k] == numeroNodoDelBloque[l])
					++cargaNodos[k];
		}
	}

	for (i = 0; i < cantBloquesArchivo; ++i){
		for (l = 0; l < numeroCopiasBloque; ++l){
			arrayInfoBloque = config_get_array_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i,l));
			numeroNodoDelBloque[l] = atoi(string_substring_from(arrayInfoBloque[0], 4));
			for (k = 0; k < cantidadNodos; ++k)
				if (indexNodos[k] == numeroNodoDelBloque[l])
					posicionCopiaEnIndexNodo[l] = k;
		}
		posicionNodoAPedir = posicionCopiaEnIndexNodo[0];
		for (l = 0; l < numeroCopiasBloque; ++l){

			if (peticiones[posicionCopiaEnIndexNodo[l]] < peticiones[posicionNodoAPedir]){
				posicionNodoAPedir = posicionCopiaEnIndexNodo[l];
			}
			else if (peticiones[posicionCopiaEnIndexNodo[l]] == peticiones[posicionNodoAPedir])
				if(cargaNodos[posicionCopiaEnIndexNodo[l]] < cargaNodos[posicionNodoAPedir]){
					posicionNodoAPedir = posicionCopiaEnIndexNodo[l];

				}

		}

		info = *(informacionNodo*)list_get(nodosConectados,posicionNodoAPedir);
		++peticiones[posicionNodoAPedir];
		numeroBloqueDataBin = atoi(arrayInfoBloque[1]);


		params[i].bloque = numeroBloqueDataBin;
		params[i].socket = info.socket;
		params[i].sem = posicionNodoAPedir;

		pthread_create(&nuevoHilo[i], NULL, &leerDeDataNode,(void*) &params[i]);
		//sem_wait(&pedidoFS);

		//printf("size bloque yay %d\n", strlen(respuestas[i]));


	}
	int contador = 0;
	for (i = 0; i < cantBloquesArchivo; ++i){
		pthread_join(nuevoHilo[i], (void**)&respuestas[i]);
	}
	for (i = 0; i < cantBloquesArchivo; ++i){
		memcpy(lectura + contador, respuestas[i], strlen(respuestas[i]));
		contador += strlen(respuestas[i]);
		//free(respuestas[i]);
	}


	free(currentChar);
	free(rutaArchivoEnMetadata);
	free(nombre);
	free(rutaFinal);
	fclose(informacionArchivo);
	return lectura;
}

void* leerDeDataNode(void* parametros){
	 struct parametrosLecturaBloque* params;
	 params = (struct parametrosLecturaBloque*) parametros;
	 sem_wait(&pedidoLecturaFS[params->sem]);
	 respuesta respuesta;
	 empaquetar(params->socket, mensajeNumeroLecturaBloqueANodo, sizeof(int),&params->bloque);
	 respuesta = desempaquetar(params->socket);
	 params->contenidoBloque = malloc(respuesta.size + 1);
	 memset(params->contenidoBloque, 0, respuesta.size + 1);
	 memcpy(params->contenidoBloque, respuesta.envio, respuesta.size);
	 sem_post(&pedidoLecturaFS[params->sem]);
	 pthread_exit(params->contenidoBloque);//(void*)params->contenidoBloque;
}

void guardarEnNodos(char* path, char* nombre, char* tipo, string* mapeoArchivo){
	respuesta respuestaPedidoAlmacenar;
	int totalRestante = 0;

	char* ruta = buscarRutaArchivo(path);
	char* rutaFinal = malloc(strlen(ruta) + strlen(nombre) + strlen(tipo) + 2);
	memset(rutaFinal, 0, strlen(ruta) + strlen(nombre) +  strlen(tipo) + 2);

	if (!validarDirectorio(ruta))
		mkdir(ruta,0777);

	memcpy(rutaFinal, ruta, strlen(ruta));
	memcpy(rutaFinal + strlen(ruta), "/", 1);
	memcpy(rutaFinal + strlen(ruta) + 1, nombre, strlen(nombre));
	memcpy(rutaFinal + strlen(ruta) + 1 + strlen(nombre), tipo, strlen(tipo));

	int sizeAux = mapeoArchivo->longitud;
	int cantBloquesArchivo = 0;
	int ultimoSize = 0, sizeEnvio = 0;

	while(sizeAux > 0){
		sizeAux -= mb;
		++cantBloquesArchivo;
	}
	//--cantNodosNecesarios;

	int sizeUltimoNodo = sizeAux+mb;


	FILE* archivos = fopen(rutaFinal, "wb+");
	fclose(archivos); //para dejarlo vacio

	t_config* infoArchivo = config_create(rutaFinal);

	//Busco la ruta donde tengo que guardar el archivo y lo dejo en blanco

	int i, j, k, success = 1, bloqueLibre = -1;
	int nodoAUtilizar = -1;
	int offset = 0;
	int sizeRestante = 0;
	int totalAsignado = 0;

	printf("bloques necesarios %d\n",cantBloquesArchivo);
	informacionNodo infoAux;
	int cantidadNodos = list_size(nodosConectados);
	int bloquesLibreNodo[cantidadNodos];
	int indexNodos[cantidadNodos];
	int masBloquesLibres[numeroCopiasBloque];
	int nodosEnUso[cantidadNodos];
	int indexNodoEnListaConectados[numeroCopiasBloque];
	parametrosEnvioBloque params;
	int sizeTotal = 0, ultimoUtilizado = 0;

	params.restanteAnterior = 0;
	params.mapa = mapeoArchivo->cadena;
	for (i = 0; i < cantidadNodos; ++i){
		infoAux = *(informacionNodo*)list_get(nodosConectados,i);
		bloquesLibreNodo[i] = infoAux.sizeNodo-infoAux.bloquesOcupados;
		indexNodos[i] = infoAux.numeroNodo;
		indexNodoEnListaConectados[i] = i;
		printf("bloques libres %d\n", bloquesLibreNodo[i]);
	}

	for (i = 0; i < cantBloquesArchivo; ++i){	//Primer for: itera por cada bloque que ocupa el archivo //Segundo y tercer for: itera para ver cuales nodos tienen menos bloques
		for (j = 0; j < numeroCopiasBloque; ++j)	// y se queda con la cantidad de nodos por copia que cumplan con ese
			masBloquesLibres[j] = -1;				//criterio

		for (j = 0; j < cantidadNodos; ++j)
			nodosEnUso[j] = 0;

		for (k = 0; k < numeroCopiasBloque; ++k){
			for (j = 0; j < cantidadNodos; ++j){
				if (nodosEnUso[j] != 1){
					if (masBloquesLibres[k] == -1){
						masBloquesLibres[k] = indexNodos[j];
						ultimoUtilizado = j;
						nodosEnUso[j] = 1;
						bloquesLibreNodo[j] -= 1;
					}
					else if(bloquesLibreNodo[ultimoUtilizado] < bloquesLibreNodo[j]){
						nodosEnUso[ultimoUtilizado] = 0;
						++bloquesLibreNodo[ultimoUtilizado];
						masBloquesLibres[k] = indexNodos[j];
						nodosEnUso[j] = 1;
						bloquesLibreNodo[j] -= 1;
					}
				}
			}
		}
		if (i < cantBloquesArchivo-1)
			ultimoSize = mb;
		else
			ultimoSize = sizeUltimoNodo;

		params.offset = offset;
		params.sizeBloque = ultimoSize;
		for (j = 0; j < numeroCopiasBloque; ++j){
			for (k = 0; k < cantidadNodos; ++k)
				if(masBloquesLibres[j] ==  indexNodos[k]){
					nodoAUtilizar = k;
				}
			pthread_attr_t attr;
			pthread_t nuevoHilo;

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			infoAux = *(informacionNodo*)list_get(nodosConectados,indexNodoEnListaConectados[nodoAUtilizar]);
			printf("le mando el bloque a %d\n", infoAux.numeroNodo);

			bloqueLibre = buscarPrimerBloqueLibre(indexNodoEnListaConectados[nodoAUtilizar], infoAux.sizeNodo);
			params.socket = infoAux.socket;
			params.bloque = bloqueLibre;

			if (ultimoSize == mb){
				 sizeRestante = bytesACortar(params.mapa);
				 params.sizeBloque = mb -sizeRestante;
				 totalRestante += sizeRestante;
			 }
			 else if(totalAsignado == 0){
				params.sizeBloque += sizeRestante;
				sizeUltimoNodo = params.sizeBloque;
				totalAsignado = 1;
			 }
			if (j == 0)
				sizeTotal += params.sizeBloque;

			pthread_create(&nuevoHilo, &attr, &enviarADataNode,(void*) &params);
			sem_wait(&pedidoFS);

			printf("bloque libre %d %d\n",bloqueLibre, infoAux.numeroNodo);
			//respuestaPedidoAlmacenar = desempaquetar(infoAux.socket);

			//memcpy(&success,respuestaPedidoAlmacenar.envio, sizeof(int));

			printf("success %d\n", success);

			if (success == 1){
				setearBloqueOcupadoEnBitmap(indexNodoEnListaConectados[nodoAUtilizar], bloqueLibre);
				config_set_value(infoArchivo, string_from_format("BLOQUE%dBYTES",i), string_itoa(params.sizeBloque));
				config_set_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i ,j), generarArrayBloque(masBloquesLibres[j], bloqueLibre));
			}
		}
		params.restanteAnterior = sizeRestante;
		if (i < cantBloquesArchivo-1)
			offset += mb;


	}
	if(success == 1){ //Por cada bloque agrego sus valores para la tabla
		config_set_value(infoArchivo, "RUTA", rutaFinal);
		config_set_value(infoArchivo, "TAMANIO", string_itoa(sizeTotal));
	}
	config_save_in_file(infoArchivo, rutaFinal); //guarda la tabla de archivos
	free(rutaFinal);

}

void* enviarADataNode(void* parametros){
	 struct parametrosEnvioBloque* params;
	 params = (struct parametrosEnvioBloque*) parametros;
	 char* buff = malloc(params->sizeBloque);
	 memcpy(buff, params->mapa+params->offset-params->restanteAnterior, params->sizeBloque);
	 empaquetar(params->socket, mensajeNumeroCopiaBloqueANodo, sizeof(int),&params->bloque);
	 empaquetar(params->socket, mensajeEnvioBloqueANodo, params->sizeBloque,buff);
	 free(buff);
	 sem_post(&pedidoFS);
	 return 0;
}

int bytesACortar(char* mapa){
	int index = 0;
	char* mapaInvertido = string_reverse(mapa);
	char currentChar;
	memcpy(&currentChar,mapaInvertido,1);
	while(currentChar != '\n'){
		++index;
		memcpy(&currentChar,mapaInvertido + index,1);
	}
	return index;
}

void setearBloqueOcupadoEnBitmap(int numeroNodo, int bloqueLibre){
	informacionNodo* infoAux;
	t_bitarray* bitarrayNodo = list_get(bitmapsNodos,numeroNodo);
	bitarray_set_bit(bitarrayNodo,bloqueLibre);
	infoAux = list_get(nodosConectados,numeroNodo);
	++infoAux->bloquesOcupados;
	printf("lala %d %d\n", infoAux->bloquesOcupados, infoAux->numeroNodo);
}

void actualizarBitmapNodos(){
	char* sufijo = ".bin";
	int cantidadNodos = list_size(nodosConectados);
	int i,j;

	t_bitarray* bitarrayNodo;
	informacionNodo infoAux;
	for (j = 0; j < cantidadNodos; j++){
		bitarrayNodo = list_get(bitmapsNodos,j);
		infoAux = *(informacionNodo*)list_get(nodosConectados,j);

		int longitudPath = strlen(rutaBitmaps);
		char* ruta = malloc(strlen(rutaBitmaps) + 1);
		memset(ruta, 0, strlen(rutaBitmaps) + 1);
		memcpy(ruta, rutaBitmaps, strlen(rutaBitmaps));
		char* nombreNodo = string_from_format("NODO%d",infoAux.numeroNodo);
		int longitudNombre = strlen(nombreNodo);
		int longitudSufijo = strlen(sufijo);
		char* pathParticular = malloc(longitudPath + longitudNombre + longitudSufijo + 1);
		memset(pathParticular,0,longitudPath + longitudNombre + longitudSufijo +1);

		memcpy(pathParticular, ruta, longitudPath);
		memcpy(pathParticular + longitudPath, nombreNodo, longitudNombre);
		memcpy(pathParticular + longitudPath + longitudNombre, sufijo, longitudSufijo);

		FILE* archivoBitmap = fopen(pathParticular, "wb+");

		for (i = 0; i < infoAux.sizeNodo; ++i){
			if (bitarray_test_bit(bitarrayNodo,i) == 0)
				fwrite("0",1,1,archivoBitmap);
			else
				fwrite("1",1,1,archivoBitmap);
		}
		fclose(archivoBitmap);
		free(pathParticular);
	}
}

char* generarArrayBloque(int numeroNodo, int numeroBloque){
	return string_from_format("[NODO%d,%d]",numeroNodo ,numeroBloque);
}

int buscarPrimerBloqueLibre(int numeroNodo, int sizeNodo){
	t_bitarray* bitarrayNodo = list_get(bitmapsNodos,numeroNodo);
	int i;
	for (i = 0; i < sizeNodo; ++i){
		if (bitarray_test_bit(bitarrayNodo,i) == 0){
			return i;
		}
	}
	return -1;
}

int levantarBitmapNodo(int numeroNodo, int sizeNodo) { //levanta el bitmap y a la vez devuelve la cantidad de bloques libres en el nodo
	char* sufijo = ".bin";
	t_bitarray* bitmap;

	int BloquesOcupados = 0, i;
	int longitudPath = strlen(rutaBitmaps);
	char* nombreNodo = string_from_format("NODO%d",numeroNodo);
	int longitudNombre = strlen(nombreNodo);
	int longitudSufijo = strlen(sufijo);

	char* pathParticular = malloc(longitudPath + longitudNombre + longitudSufijo + 1);
	memset(pathParticular, 0, longitudPath + longitudNombre + longitudSufijo + 1);

	char* espacioBitarray = malloc(sizeNodo+1);
	char* currentChar = malloc(sizeof(char)+1);
	memset(espacioBitarray, 0, sizeNodo + 1);
	memset(currentChar, 0, 2);
	int posicion = 0;

	memcpy(pathParticular, rutaBitmaps, longitudPath);
	memcpy(pathParticular + longitudPath, nombreNodo, longitudNombre);
	memcpy(pathParticular + longitudPath + longitudNombre, sufijo, longitudSufijo);

	FILE* bitmapFile;

	if (!validarArchivo(pathParticular) == 1){
		bitmapFile = fopen(pathParticular, "w");
		for(i = 0; i < sizeNodo; ++i)
			fwrite("0",1,1, bitmapFile);
		fclose(bitmapFile);
	}

	bitmapFile = fopen(pathParticular, "r");

	bitmap = bitarray_create_with_mode(espacioBitarray, cantBloques, LSB_FIRST);

	fread(currentChar, 1, 1, bitmapFile);
	while (!feof(bitmapFile)) {
		if (strcmp(currentChar, "1") == 0){
			bitarray_set_bit(bitmap, posicion);
			++BloquesOcupados;
		}
		else
			bitarray_clean_bit(bitmap, posicion);
		++posicion;
		fread(currentChar, 1, 1, bitmapFile);
	}
	printf("ocupados %d\n", BloquesOcupados);

	/*int contador = 0;
	while(contador < posicion){
		printf("bit %d\n", bitarray_test_bit(bitmap,contador));
		++contador;
	} /*para verificar que lo lee bien */

	free(pathParticular);
	fclose(bitmapFile);

	list_add(bitmapsNodos,bitmap);

	return BloquesOcupados;
}

void actualizarArchivoNodos(){

	char* pathArchivo = "../metadata/Nodos.bin";
	FILE* archivoNodes = fopen(pathArchivo, "wb+");
	fclose(archivoNodes); //para dejarlo vacio
	int cantidadNodos = list_size(nodosConectados), i = 0;
	int bloquesLibresNodo = 0;
	informacionNodo info;

	t_config* nodos = config_create(pathArchivo);

	sizeTotalNodos = 0;
	nodosLibres = 0;
	for (i = 0; i < cantidadNodos; ++i){
		info = *(informacionNodo*)list_get(nodosConectados,i);
		bloquesLibresNodo = info.sizeNodo-info.bloquesOcupados;
		sizeTotalNodos += info.sizeNodo;
		nodosLibres += bloquesLibresNodo;
		config_set_value(nodos, string_from_format("NODO%dTOTAL",info.numeroNodo), string_itoa(info.sizeNodo));
		config_set_value(nodos, string_from_format("NODO%dLIBRE",info.numeroNodo), string_itoa(bloquesLibresNodo));
	}

	char* arrayNodos = generarArrayNodos();


	config_set_value(nodos, "TAMANIO", string_itoa(sizeTotalNodos));

	config_set_value(nodos, "LIBRE", string_itoa(nodosLibres));

	config_set_value(nodos, "NODOS", arrayNodos);

	config_save_in_file(nodos, pathArchivo);

	free(arrayNodos);
}

char* generarArrayNodos(){
	int i, cantidadNodos = list_size(nodosConectados);
	int indexes[cantidadNodos];
	char* array;
	int longitudEscrito = 0;
	int longitudSting = 1 + cantidadNodos*5; // 2 [] + "NODO," * cantidad nodos - 1 de la ultima coma, despues se le suman los numeros
	informacionNodo info;
	for (i = 0; i < cantidadNodos; ++i){
		info = *(informacionNodo*)list_get(nodosConectados,i);
		indexes[i] = info.numeroNodo;
		longitudSting += strlen(string_itoa(info.numeroNodo));
	}
	array = malloc(longitudSting + 1);
	memset(array, 0,longitudSting + 1);
	memcpy(array,"[",1);
	++longitudEscrito;
	for (i = 0; i < cantidadNodos; ++i){
		char* temp = string_from_format("NODO%d,",indexes[i]);
		memcpy(array + longitudEscrito,  temp, strlen(temp));
		longitudEscrito += strlen(temp);
	}
	memcpy(array + longitudEscrito-1,"]",1);
	return array;
}



void atenderSolicitudYama(int socketYama, void* envio){



	//empaquetar(socketYama, mensajeInfoArchivo, 0 ,0);

	//aca hay que aplicar la super funcion mockeada de @Ronan

}

informacionNodo* informacionNodosConectados(){
	int cantidadNodos = list_size(nodosConectados);
	informacionNodo* listaInfoNodos = malloc(sizeof(informacionNodo)*cantidadNodos);

	int i;

	for (i = 0; i < cantidadNodos; i++){
		listaInfoNodos[i] = *(informacionNodo*) list_get(nodosConectados, i);
	}
	return listaInfoNodos;
}

informacionArchivoFsYama obtenerInfoArchivo(string rutaDatos){
	informacionArchivoFsYama info;
	info.informacionBloques = list_create();
	char* rutaArchivo = buscarRutaArchivo(rutaDatos.cadena);
	strcat(rutaArchivo,"/");
	strcat(rutaArchivo,rutaDatos.cadena);

	char* rutaPrueba = "/home/utnso/tp-2017-2c-PEQL/FileSystem/metadata/Archivos/0/hola3.txt";

	t_config* archivo = config_create(rutaPrueba);

	info.tamanioTotal = config_get_int_value(archivo,"TAMANIO");

	int cantBloques = redondearHaciaArriba(info.tamanioTotal , sizeBloque);

	int i;
	for(i=0;i<cantBloques;i++){
		char* clave = calloc(1,8);
		string_append_with_format(&clave,"BLOQUE%d",i);
		infoBloque* infoBloqueActual = malloc(sizeof(infoBloque));
		char* claveCopia0 = strdup(clave);
		char* claveCopia1 = strdup(clave);
		char* claveBytes = strdup(clave);

		string_append(&claveCopia0,"COPIA0");
		string_append(&claveCopia1,"COPIA1");
		string_append(&claveBytes,"BYTES");

		infoBloqueActual->numeroBloque = i;

		infoBloqueActual->bytesOcupados = config_get_int_value(archivo,claveBytes);
		obtenerNumeroNodo(archivo,claveCopia0,&(infoBloqueActual->ubicacionCopia0));
		obtenerInfoNodo(&infoBloqueActual->ubicacionCopia0);

		obtenerNumeroNodo(archivo,claveCopia1,&(infoBloqueActual->ubicacionCopia1));
		obtenerInfoNodo(&infoBloqueActual->ubicacionCopia1);
		free(clave);
		list_add(info.informacionBloques,infoBloqueActual);
	}
	return info;
}

void obtenerInfoNodo(ubicacionBloque* ubicacion){
	int i;
	informacionNodo* info = malloc(sizeof(informacionNodo));
	for(i=0;i<list_size(nodosConectados);i++){
		info =(informacionNodo*)list_get(nodosConectados,i);

		if(info->numeroNodo == ubicacion->numeroNodo){
			ubicacion->ip.cadena = strdup(info->ip.cadena);
			ubicacion->ip.longitud = info->ip.longitud;
			ubicacion->puerto = info->puerto;
		}
	}
}
