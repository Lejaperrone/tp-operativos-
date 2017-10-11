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

char* pathArchivoDirectorios = "/home/utnso/tp-2017-2c-PEQL/FileSystem/metadata/Directorios.dat";

void establecerServidor(int servidorFS){
	struct sockaddr_in direccionServidor = cargarDireccion("127.0.0.1",7000);
	int activado = 1;
	setsockopt(servidorFS, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	asociarSocketA(direccionServidor, servidorFS);
}

int recibirConexionYama(int servidorFS){
	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(direccionCliente);
	respuesta respuestaId;
	while(1){
		int cliente =0;

		if((cliente = accept(servidorFS, (struct sockaddr *)&direccionCliente, &tamanioDireccion)) != -1){
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
	for (i = 0; i < 100; ++i){
		tablaDeDirectorios[i].index = -1;
		tablaDeDirectorios[i].padre = -1;
		memcpy(tablaDeDirectorios[i].nombre," ",1);
	}
	int leido;
	FILE* archivoDirectorios = fopen(pathArchivoDirectorios, "r");
	leido = fread(tablaDeDirectorios,sizeof(t_directory), cantidadDirectorios, archivoDirectorios);
	if (leido == cantidadDirectorios)
		log_trace(loggerFS, "Se cargaron los directorios correctamente");
	else
		log_trace(loggerFS, "No se pudieron cargar todos los directorios");
	fclose(archivoDirectorios);
}

void guardarTablaDirectorios(){
	int escrito;
	FILE* archivoDirectorios = fopen(pathArchivoDirectorios, "wb+");
	escrito = fwrite(tablaDeDirectorios,sizeof(t_directory), cantidadDirectorios, archivoDirectorios);
	if (escrito == cantidadDirectorios)
		log_trace(loggerFS, "Se escribieron los directorios correctamente");
	else
		log_trace(loggerFS, "No se pudieron escribir todos los directorios");
	fclose(archivoDirectorios);
}

int getIndexDirectorio(char* ruta){
	int index = 0, i = 0, j = 0, indexFinal = 0;
	char** arrayPath = string_split(ruta, "/");
	char* arrayComparador[100];
	while(arrayPath[index] != NULL){ // separo por '/' la ruta en un array
		arrayComparador[index] = malloc(strlen(arrayPath[index]));
		memcpy(arrayComparador[index],arrayPath[index],strlen(arrayPath[index]));
		++index; // guardo cuantas partes tiene el array
	}
	indexFinal = index - 1;
	int indexDirectorios[index];

	--index;
	for (i = 0; i < index; ++i)
		indexDirectorios[i] = -1; // inicializo todo en -1, si al final me queda alguno en algun lado del array, significa que la ruta que pase no existe

	indexDirectorios[0] = 0;

	for(i = 0; i < index; --index){
		for(j = 0; j < 100; ++j){ // busco en la tabla que posicion coincide con el fragmento de ruta del loop
			if (strcmp(tablaDeDirectorios[j].nombre,arrayComparador[index]) == 0){ // me fijo si el padre de ese es el mismo que aparece en el fragmento anterior de la ruta
				if (index != 0 && strcmp(tablaDeDirectorios[tablaDeDirectorios[j].padre].nombre,arrayComparador[index-1]) == 0){ // si es asi lo guardo
					indexDirectorios[index] = tablaDeDirectorios[j].index;
				}
			}
		}
	}

	for (i = 0; i < indexFinal; ++i)
		if (indexDirectorios[i] == -1){
			for (i = 0; i < indexFinal; ++i){
				free(arrayComparador[i]);
			}
			return -1;
		} //me fijo si quedo algun -1, si es asi no existe la ruta

	for (i = 0; i < indexFinal; ++i){
		free(arrayComparador[i]);
	}
	return indexDirectorios[indexFinal]; // devuelvo el index de la ruta
}

char* buscarRutaArchivo(char* ruta){
	int indexDirectorio = getIndexDirectorio(ruta);
	char* numeroIndexString = string_itoa(indexDirectorio);
	char* rutaGenerada = malloc(strlen(rutaArchivos) + strlen(numeroIndexString));
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

void guardarEnNodos(char* path, char* nombre, char* tipo, string* mapeoArchivo){
	int mockSizeArchivo = 2*mb;
	respuesta respuestaPedidoAlmacenar;

	char* ruta = buscarRutaArchivo(path);
	char* rutaFinal = malloc(strlen(ruta) + strlen(nombre) + 1);

	if (!validarDirectorio(ruta))
		mkdir(ruta,0777);

	memcpy(rutaFinal, ruta, strlen(ruta));
	memcpy(rutaFinal + strlen(ruta), "/", 1);
	memcpy(rutaFinal + strlen(ruta) + 1, nombre, strlen(nombre));
	memcpy(rutaFinal + strlen(ruta) + 1 + strlen(nombre), tipo, strlen(tipo));

	printf("----ruta final    %s\n", rutaFinal);

	int sizeAux = mapeoArchivo->longitud;
	int cantNodosNecesarios = 0;
	int ultimoSize = 0;

	while(sizeAux > 0){
		sizeAux -= mb;
		++cantNodosNecesarios;
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

	printf("nodos a usar %d\n",cantNodosNecesarios);
	informacionNodo infoAux;
	int cantidadNodos = list_size(nodosConectados);
	int bloquesLibreNodo[cantidadNodos];
	int indexNodos[cantidadNodos];
	int masBloquesLibres[numeroCopiasBloque];
	int nodosEnUso[cantidadNodos];
	int indexNodoEnListaConectados[numeroCopiasBloque];
	parametrosEnvioBloque params;

	params.mapa = mapeoArchivo->cadena;
	for (i = 0; i < cantidadNodos; ++i){
		infoAux = *(informacionNodo*)list_get(nodosConectados,i);
		bloquesLibreNodo[i] = infoAux.sizeNodo-infoAux.bloquesOcupados;
		indexNodos[i] = infoAux.numeroNodo;
		indexNodoEnListaConectados[i] = i;
		printf("bloques libres %d\n", bloquesLibreNodo[i]);
	}

	for (i = 0; i < cantNodosNecesarios; ++i){	//Primer for: itera por cada bloque que ocupa el archivo
		printf("-------------------offset %d\n", offset);
		printf("--%d\n",cantNodosNecesarios);	//Segundo y tercer for: itera para ver cuales nodos tienen menos bloques
		for (j = 0; j < numeroCopiasBloque; ++j)	// y se queda con la cantidad de nodos por copia que cumplan con ese
			masBloquesLibres[j] = -1;				//criterio
		for (j = 0; j < cantidadNodos; ++j)
			nodosEnUso[j] = 0;
		for (j = 0; j < cantidadNodos; ++j){
			for (k = 0; k < numeroCopiasBloque; ++k){
				if (nodosEnUso[j] != 1){
					if (masBloquesLibres[k] == -1){
						masBloquesLibres[k] = indexNodos[j];
						nodosEnUso[j] = 1;
						bloquesLibreNodo[j] -= 1;
					}
					else if(bloquesLibreNodo[masBloquesLibres[k]] < bloquesLibreNodo[j]){
						masBloquesLibres[k] = indexNodos[j];
						nodosEnUso[j] = 1;
						bloquesLibreNodo[j] -= 1;
					}
				}
			}
		}
		if (i < cantNodosNecesarios-1)
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

			pthread_create(&nuevoHilo, &attr, &enviarADataNode,(void*) &params);
			sem_wait(&pedidoFS);

			printf("bloque libre %d %d\n",bloqueLibre, infoAux.numeroNodo);
			//respuestaPedidoAlmacenar = desempaquetar(infoAux.socket);

			//memcpy(&success,respuestaPedidoAlmacenar.envio, sizeof(int));

			printf("success %d\n", success);

			if (success == 1){
				setearBloqueOcupadoEnBitmap(indexNodoEnListaConectados[nodoAUtilizar], bloqueLibre);
				config_set_value(infoArchivo, string_from_format("BLOQUE%dBYTES",i), string_itoa(ultimoSize));
				config_set_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i ,j), generarArrayBloque(masBloquesLibres[j], bloqueLibre));
			}
		}
		if (i < cantNodosNecesarios-1)
			offset += mb;


	}
	if(success == 1){ //Por cada bloque agrego sus valores para la tabla
		config_set_value(infoArchivo, "RUTA", rutaFinal);
		config_set_value(infoArchivo, "TAMANIO", string_itoa(mb*(cantNodosNecesarios-1)+sizeUltimoNodo));
	}
	config_save_in_file(infoArchivo, rutaFinal); //guarda la tabla de archivos


}

void* enviarADataNode(void* parametros){
	 struct parametrosEnvioBloque* params;
	 params = (struct parametrosEnvioBloque*) parametros;
	 char* buff = malloc(params->sizeBloque);
	 printf("........................nro bloque %d\n", params->bloque);
	 memcpy(buff, params->mapa+params->offset, params->sizeBloque);
	 empaquetar(params->socket, mensajeNumeroBloqueANodo, sizeof(int),&params->bloque);
	 empaquetar(params->socket, mensajeEnvioBloqueANodo, params->sizeBloque,buff);
	 free(buff);
	 sem_post(&pedidoFS);
	 return 0;
}

void setearBloqueOcupadoEnBitmap(int numeroNodo, int bloqueLibre){
	informacionNodo* infoAux;
	t_bitarray* bitarrayNodo = list_get(bitmapsNodos,numeroNodo);
	bitarray_set_bit(bitarrayNodo,bloqueLibre);
	infoAux = list_get(nodosConectados,numeroNodo);
	++infoAux->bloquesOcupados;
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
		char* ruta = malloc(strlen(rutaBitmaps));
		memcpy(ruta, rutaBitmaps, strlen(rutaBitmaps));
		char* nombreNodo = string_from_format("NODO%d",infoAux.numeroNodo);
		int longitudNombre = strlen(nombreNodo);
		int longitudSufijo = strlen(sufijo);
		char* pathParticular = malloc(longitudPath + longitudNombre + longitudSufijo);
		memset(pathParticular,0,longitudPath + longitudNombre + longitudSufijo +1);

		memcpy(pathParticular, ruta, longitudPath);
		printf("partial path %s\n", rutaBitmaps);
		memcpy(pathParticular + longitudPath, nombreNodo, longitudNombre);
		printf("partial path %d\n", longitudPath);
		memcpy(pathParticular + longitudPath + longitudNombre, sufijo, longitudSufijo);

		printf("numero nodo %s\n", sufijo);

		printf("--path-- %s\n", pathParticular);

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
			printf("------------------------------------i %d\n", i);
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

	char* pathParticular = malloc(longitudPath + longitudNombre + longitudSufijo);

	char* espacioBitarray = malloc(cantBloques);
	char* currentChar = malloc(sizeof(char));
	int posicion = 0;

	memcpy(pathParticular, rutaBitmaps, longitudPath);
	memcpy(pathParticular + longitudPath, nombreNodo, longitudNombre);
	memcpy(pathParticular + longitudPath + longitudNombre, sufijo, longitudSufijo);

	printf("path %s", pathParticular);
	FILE* bitmapFile;

	if (!validarArchivo(pathParticular) == 1){
		bitmapFile = fopen(pathParticular, "w");
		for(i = 0; i < 50; ++i)
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

	int contador = 0;
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

	char* pathArchivo = "/home/utnso/tp-2017-2c-PEQL/FileSystem/metadata/Nodos.bin";
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
	array = malloc(longitudSting);
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


void almacenarArchivo(char* ruta, char* nombreArchivo, char tipo, char* datos);

void inicializarBitmaps(){ //TODO (y ver si aca hace falta mmap)
	/*char* path = "/Metadata/Bitmap.bin"; //cambiar
	int fd = open(path,O_RDWR | O_TRUNC);
	if (fd == -1)
	{
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}
	if (lseek(fd, cantBloques-1, SEEK_SET) == -1)
		{
			close(fd);
			perror("Error calling lseek() to 'stretch' the file");
			exit(EXIT_FAILURE);
	}
	if (write(fd, "", 1) == -1)
	{
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}
	char* mapa = mmap(0,cantBloques/8,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);

	bitmap[0] = bitarray_create_with_mode(mapa, cantBloques/8 , LSB_FIRST);*/
}

void atenderSolicitudYama(int socketYama, void* envio){



	//empaquetar(socketYama, mensajeInfoArchivo, 0 ,0);

	//aca hay que aplicar la super funcion mockeada de @Ronan

}
