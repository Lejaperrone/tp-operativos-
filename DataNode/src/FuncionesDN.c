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
int cantBloques = 50;
extern struct configuracionNodo  config;
extern sem_t pedidoFS;

void enviarBloqueAFS(int numeroBloque) {

}

int setBloque(int numeroBloque, void* datos) {
	return 1;
}

void conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 7000);
	conectarCon(direccion, socketFs, 3);
	informacionNodo info;
	info.sizeNodo = config.SIZE_NODO;
	info.bloquesOcupados = -1;//levantarBitmap(config.NOMBRE_NODO);
	info.numeroNodo = atoi(string_substring_from(config.NOMBRE_NODO,4));
	printf("soy el nodo %d\n", info.numeroNodo);
	info.socket = -1;
	empaquetar(socketFs, mensajeInformacionNodo, sizeof(informacionNodo),&info );
	escucharAlFS(socketFs);
}

void escucharAlFS(int socketFs){
	respuesta pedido;
	int bloqueOcupado = 1;
	int bloqueMock;
	int archivoMock;
	int success = 0;
	while(1){
		pedido = desempaquetar(socketFs);
		memcpy(&bloqueMock, pedido.envio, sizeof(int));
		pedido = desempaquetar(socketFs);
		memcpy(&archivoMock, pedido.envio, sizeof(int));
		printf("la longitud es %d\n", bloqueMock);
		success = setBloque(bloqueMock, &archivoMock);
		empaquetar(socketFs, mensajeRespuestaEnvioBloqueANodo, sizeof(int), &success);
		sem_post(&pedidoFS);
	}
}

