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

#define idDataNodes 3
#define cantDataNodes 10
#define mb 1048576

extern int numeroCopiasBloque;
extern t_directory tablaDeDirectorios[100];
extern char* rutaArchivos;
extern t_log* loggerFS;
extern int cantidadDirectorios;
extern int cantBloques;
extern int sizeTotalNodos, nodosLibres;
extern t_list* bitmapsNodos;;
extern t_list* nodosConectados;
extern char* rutaBitmaps;
char* pathArchivoDirectorios = "/home/utnso/Escritorio/tp-2017-2c-PEQL/FileSystem/metadata/Directorios.dat";

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

void* consolaFS(){

	int sizeComando = 256;

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando);
		bzero(comando, sizeComando);
		comando = readline(">");
		if (comando)
			add_history(comando);

		log_trace(loggerFS, "El usuario ingreso: %s", comando);

		if (string_starts_with(comando, "format")) {
			log_trace(loggerFS, "File system formateado");
		}
		else if (string_starts_with(comando, "rm -d")) {
			if (eliminarDirectorio(comando, 2) != -1)
				log_trace(loggerFS, "Directorio eliminado");
			else
				log_trace(loggerFS, "No se pudo eliminar el directorio");
		}
		else if (string_starts_with(comando, "rm -b")) {
			log_trace(loggerFS, "Bloque eliminado");
		}
		else if (string_starts_with(comando, "rm")) {
			if (eliminarArchivo(comando, 1) != -1)
				log_trace(loggerFS, "archivo eliminado");
			else
				log_trace(loggerFS, "No se pudo eliminar el archivo");
		}
		else if (string_starts_with(comando, "rename")) {
			if (cambiarNombre(comando, 1) == 1)
				log_trace(loggerFS, "Renombrado");
			else
				log_trace(loggerFS, "No se pudo renombrar");

		}
		else if (string_starts_with(comando, "mv")) {
			if (mover(comando,1) == 1)
				log_trace(loggerFS, "Archivo movido");
			else
				log_trace(loggerFS, "No se pudo mover el archivo");
		}
		else if (string_starts_with(comando, "cat")) {
			if (mostrarArchivo(comando, 1) == 1){
			log_trace(loggerFS, "Archivo mostrado");
			}else{
				log_trace(loggerFS, "No se pudo mostrar el archivo");
			}
		}
		else if (string_starts_with(comando, "mkdir")) {
			if (crearDirectorio(comando,1) == 1){

			log_trace(loggerFS, "Directorio creado");// avisar si ya existe
			}else{
				if (crearDirectorio(comando,1) == 2){
				log_trace(loggerFS, "El directorio ya existe");
				}else{
					log_trace(loggerFS, "No se pudo crear directorio");
				}
			}
		}
		else if (string_starts_with(comando, "cpfrom")) {
			if (copiarArchivo(comando) == 1)
				log_trace(loggerFS, "Archivo copiado a yamafs");
			else
				log_trace(loggerFS, "No se pudo copiar el archivo");
		}
		else if (string_starts_with(comando, "cpto")) {
			log_trace(loggerFS, "Archivo copiado desde yamafs");
		}
		else if (string_starts_with(comando, "cpblock")) {
			log_trace(loggerFS, "Bloque copiado en el nodo");
		}
		else if (string_starts_with(comando, "md5")) {
			log_trace(loggerFS, "MD5 del archivo");
		}
		else if (string_starts_with(comando, "ls")) {
			listarArchivos(comando, 1);
			log_trace(loggerFS, "Archivos listados");
		}
		else if (string_starts_with(comando, "info")) {
			if (informacion(comando,1) == 1)
				log_trace(loggerFS, "Mostrando informacion del archivo");
			else
				log_trace(loggerFS, "No se pudo mostrar informacion del archivo");
		}
		else {
			printf("Comando invalido\n");
			log_trace(loggerFS, "Comando invalido");
		}
		free(comando);
	}
	return 0;
}

void* levantarServidorFS(void* parametrosServidorFS){

	int maxDatanodes;
	int nuevoDataNode;
	int cantidadNodos;
	informacionNodo info;
	respuesta nuevaConexionYama;
	respuesta solicitudInfoArchivo;

	int i = 0, j = 0;
	int addrlen;

	struct parametrosServidorHilo*params;
	params = (struct parametrosServidorHilo*) parametrosServidorFS;

	int clienteYama = params->cliente;
	int servidor = params->servidor;

	char* buffer = malloc(300);
	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(direccionCliente);
	struct sockaddr_in direccionServidor = cargarDireccion("127.0.0.1",7000);
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	asociarSocketA(direccionServidor, servidor);

	//cliente = accept(servidor, (struct sockaddr *) &direccionCliente, &tamanioDireccion);

	//falta agregar el manejo de error cuando se desconecta el fs,
	//handshake y el protocolo de envio de mensajes
	free(buffer);

	fd_set datanodes;
	fd_set read_fds_datanodes;

	respuesta conexionNueva, paqueteInfoNodo;
	int bufferPrueba = 2;
	FD_ZERO(&datanodes);    // borra los conjuntos datanodes y temporal
	FD_ZERO(&read_fds_datanodes);
	// añadir listener al conjunto maestro
	FD_SET(servidor, &datanodes);
	// seguir la pista del descriptor de fichero mayor
	maxDatanodes = servidor; // por ahora es éste
	// bucle principal
	while(1){
		read_fds_datanodes = datanodes; // cópialo
		if (select(maxDatanodes+1, &read_fds_datanodes, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maxDatanodes; i++) {
			if (FD_ISSET(i, &read_fds_datanodes)) { // ¡¡tenemos datos!!
				if (i == servidor) {
					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((nuevoDataNode = accept(servidor, (struct sockaddr *)&direccionCliente,
							&addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(nuevoDataNode, &datanodes); // añadir al conjunto maestro
						if (nuevoDataNode > maxDatanodes) {    // actualizar el máximo
							maxDatanodes = nuevoDataNode;
						}
						conexionNueva = desempaquetar(nuevoDataNode);
						int idRecibido = *(int*)conexionNueva.envio;

						if (idRecibido == idDataNodes){
							//empaquetar(nuevoDataNode,1,0,&bufferPrueba);//FIXME:SOLO A MODO DE PRUEBA
							paqueteInfoNodo = desempaquetar(nuevoDataNode);
							info = *(informacionNodo*)paqueteInfoNodo.envio;
							if (nodoRepetido(info) == 0){
								log_trace(loggerFS, "Conexion de DataNode %d\n", info.numeroNodo);
								info.bloquesOcupados = info.sizeNodo - levantarBitmapNodo(info.numeroNodo);
								info.socket = nuevoDataNode;
								memcpy(paqueteInfoNodo.envio, &info, sizeof(informacionNodo));
								list_add(nodosConectados,paqueteInfoNodo.envio);
								cantidadNodos = list_size(nodosConectados);
								actualizarArchivoNodos();
							}
							else
								log_trace(loggerFS, "DataNode repetido\n");
						}
						else if(idRecibido == 1){//idYAMA
							log_trace(loggerFS, "Nueva Conexion de Yama");
							solicitudInfoArchivo = desempaquetar(nuevoDataNode);

							if(solicitudInfoArchivo.idMensaje == mensajeSolicitudTransformacion){
								atenderSolicitudYama(nuevoDataNode, solicitudInfoArchivo.envio);
								//cambiar NUEVO DATANODE POR OTRO NOMBRE FIXME
								log_trace(loggerFS, "Me llego una solicitud para dar informacion de este archivo");
							}
						}

						else {
							// gestionar datos de un cliente

						}
					}
				}
			}
		}
	}
	return 0;

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
	int mockNumeroBloqueAsignado = 0;
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

	for (i = 0; i < cantidadNodos; ++i){
		infoAux = *(informacionNodo*)list_get(nodosConectados,i);
		bloquesLibreNodo[i] = infoAux.sizeNodo-infoAux.bloquesOcupados;
		indexNodos[i] = infoAux.numeroNodo;
		indexNodoEnListaConectados[i] = i;
		printf("bloques libres %d\n", bloquesLibreNodo[i]);
	}

	for (i = 0; i < cantNodosNecesarios; ++i){	//Primer for: itera por cada bloque que ocupa el archivo
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
		for (j = 0; j < numeroCopiasBloque; ++j){
			for (k = 0; k < cantidadNodos; ++k)
				if(masBloquesLibres[j] ==  indexNodos[k]){
					nodoAUtilizar = k;
				}
			infoAux = *(informacionNodo*)list_get(nodosConectados,indexNodoEnListaConectados[nodoAUtilizar]);
			printf("le mando el bloque a %d\n", infoAux.numeroNodo);

			bloqueLibre = buscarPrimerBloqueLibre(indexNodoEnListaConectados[nodoAUtilizar], infoAux.sizeNodo);

			printf("bloque %d\n",bloqueLibre);

			if (i == cantNodosNecesarios-1){
				enviarADataNode(mapeoArchivo->cadena, bloqueLibre, offset, sizeUltimoNodo, infoAux.socket);
				offset = 0;
				ultimoSize = mb;
			}
			else{
				enviarADataNode(mapeoArchivo->cadena, bloqueLibre, offset, mb, infoAux.socket);
				offset += mb;
				ultimoSize = sizeUltimoNodo;
			}

			printf("bloque libre %d %d\n",bloqueLibre, infoAux.numeroNodo);
			respuestaPedidoAlmacenar = desempaquetar(infoAux.socket);

			memcpy(&success,respuestaPedidoAlmacenar.envio, sizeof(int));

			printf("success %d\n", success);

			if (success == 1){
				setearBloqueOcupadoEnBitmap(indexNodoEnListaConectados[nodoAUtilizar], bloqueLibre);
				config_set_value(infoArchivo, string_from_format("BLOQUE%dBYTES",i), string_itoa(ultimoSize));
				config_set_value(infoArchivo, string_from_format("BLOQUE%dCOPIA%d",i ,j), generarArrayBloque(masBloquesLibres[j], bloqueLibre));
			}
		}

	//Empaquetar bloques a guardar a los que esten en masBloquesLibres
	//Desempaqutar notificacion success. si falla, -1
		for (k = 0; k < cantNodosNecesarios; ++k){
			printf("---nodo %d---\n", masBloquesLibres[k]);
		}

		if(success == 1){ //Por cada bloque agrego sus valores para la tabla
			config_set_value(infoArchivo, "RUTA", rutaFinal);
			config_set_value(infoArchivo, "TAMANIO", string_itoa(mockSizeArchivo));
		}


	}
	config_save_in_file(infoArchivo, rutaFinal); //guarda la tabla de archivos

}

void enviarADataNode(char* map, int bloque, int tam, int size_bytes, int socket){
	 unsigned char buffer[size_bytes + sizeof(int)];
	 unsigned char buff[mb];
	 header head;
	 memcpy(buff, map+tam, size_bytes-1);
	 if(size_bytes != mb) printf("%s\n", buff);

	 head.tamanio = serial_pack(buffer,"hs",bloque,buff);
	 head.idMensaje = mensajeEnvioBloqueANodo;

	 empaquetar(socket, mensajeEnvioBloqueANodo, head.tamanio,buffer);
	 memset(buff, 0, mb);
}

	void setearBloqueOcupadoEnBitmap(int numeroNodo, int bloqueLibre){
		informacionNodo* infoAux;
		t_bitarray* bitarrayNodo = list_get(bitmapsNodos,numeroNodo);
		bitarray_set_bit(bitarrayNodo,bloqueLibre);
		infoAux = list_get(nodosConectados,numeroNodo);
		++infoAux->bloquesOcupados;
		actualizarBitmapNodo(numeroNodo);
}

void actualizarBitmapNodo(int numeroNodo){
	char* sufijo = ".bin";
	t_bitarray* bitarrayNodo = list_get(bitmapsNodos,numeroNodo);
	informacionNodo infoAux = *(informacionNodo*)list_get(nodosConectados,numeroNodo);

	int i;
	int longitudPath = strlen(rutaBitmaps);
	char* nombreNodo = string_from_format("NODO%d",infoAux.numeroNodo);
	int longitudNombre = strlen(nombreNodo);
	int longitudSufijo = strlen(sufijo);

	char* pathParticular = malloc(longitudPath + longitudNombre + longitudSufijo);

	memcpy(pathParticular, rutaBitmaps, longitudPath);
	memcpy(pathParticular + longitudPath, nombreNodo, longitudNombre);
	memcpy(pathParticular + longitudPath + longitudNombre, sufijo, longitudSufijo);

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

char* generarArrayBloque(int numeroNodo, int numeroBloque){
	return string_from_format("[NODO%d,%d]",numeroNodo ,numeroBloque);
}

int buscarPrimerBloqueLibre(int numeroNodo, int sizeNodo){
	t_bitarray* bitarrayNodo = list_get(bitmapsNodos,numeroNodo);
	int i, numeroBloque = 1;
	for (i = 0; i < sizeNodo; ++i){
		if (bitarray_test_bit(bitarrayNodo,i) == 0){
			numeroBloque = i;
			break;
		}
	}
	return numeroBloque;
}

int levantarBitmapNodo(int numeroNodo) { //levanta el bitmap y a la vez devuelve la cantidad de bloques libres en el nodo
	char* sufijo = ".bin";
	t_bitarray* bitmap;

	int BloquesOcupados = 0;
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

	FILE* bitmapFile = fopen(pathParticular, "r");


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

	char* pathArchivo = "/home/utnso/Escritorio/tp-2017-2c-PEQL/FileSystem/metadata/Nodos.bin";
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
	respuestaTransformacion* rtaTransf;
	solicitudTransformacion* solTransf =(solicitudTransformacion*)envio;


	empaquetar(socketYama, mensajeInfoArchivo, 0 ,0);

	//aca hay que aplicar la super funcion mockeada de @Ronan

}
