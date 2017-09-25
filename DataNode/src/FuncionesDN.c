/*
 * FuncionesDN.c
 *
 *  Created on: 12/9/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "FuncionesDN.h"
#include "Globales.h"
#include "Serializacion.h"
#include "Sockets.h"
#include "Configuracion.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <commons/string.h>

char* path = "/home/utnso/Escritorio/tp-2017-2c-PEQL/FileSystem/metadata/Bitmaps/";
extern t_bitarray* bitmap;
int cantBloques = 50;
extern struct configuracionNodo  config;

void enviarBloqueAFS(int numeroBloque) {

}

void setearBloque(int numeroBloque, void* datos) {

}

void conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 7000);
	conectarCon(direccion, socketFs, 3);
	informacionNodo info;
	info.sizeNodo = config.SIZE_NODO;
	info.bloquesOcupados = levantarBitmap(config.NOMBRE_NODO);
	info.numeroNodo = atoi(string_substring_from(config.NOMBRE_NODO,4));
	empaquetar(socketFs, mensajeInformacionNodo, sizeof(informacionNodo),&info );
}


int levantarBitmap(char* nombreNodo) { //levanta el bitmap y a la vez devuelve la cantidad de bloques libres en el nodo
	char* sufijo = ".bin";

	int bloquesLibres = 0;
	int longitudPath = strlen(path);
	int longitudNombre = strlen(nombreNodo);
	int longitudSufijo = strlen(sufijo);

	char* pathParticular = malloc(longitudPath + longitudNombre + longitudSufijo);

	char* espacioBitarray = malloc(cantBloques);
	char* currentChar = malloc(sizeof(char));
	int posicion = 0;

	memcpy(pathParticular, path, longitudPath);
	memcpy(pathParticular + longitudPath, nombreNodo, longitudNombre);
	memcpy(pathParticular + longitudPath + longitudNombre, sufijo, longitudSufijo);

	printf("path %s", pathParticular);

	FILE* bitmapFile = fopen(pathParticular, "r");


	bitmap = bitarray_create_with_mode(espacioBitarray, cantBloques, LSB_FIRST);

	while (!feof(bitmapFile)) {
		fread(currentChar, 1, 1, bitmapFile);
		if (strcmp(currentChar, "1")){
			bitarray_set_bit(bitmap, posicion);
			++bloquesLibres;
		}
		else
			bitarray_clean_bit(bitmap, posicion);
		++posicion;
	}

	/*while(posicion > 0){
		printf("bit %d", bitarray_test_bit(bitmap,posicion));
		--posicion;
	} para verificar que lo lee bien */

	free(pathParticular);
	fclose(bitmapFile);
	return bloquesLibres;
}

