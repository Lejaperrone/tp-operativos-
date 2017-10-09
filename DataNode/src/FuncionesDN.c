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
#include "Serial.h"

#define mb 1048576

int cantBloques = 50;
extern struct configuracionNodo config;
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
	info.bloquesOcupados = -1; //levantarBitmap(config.NOMBRE_NODO);
	info.numeroNodo = atoi(string_substring_from(config.NOMBRE_NODO, 4));
	printf("soy el nodo %d\n", info.numeroNodo);
	info.socket = -1;
	empaquetar(socketFs, mensajeInformacionNodo, sizeof(informacionNodo),
			&info);
	escucharAlFS(socketFs);
}

void recibirMensajesFileSystem(int socketFs) {
	respuesta pedido2 = desempaquetar(socketFs);
	//char* buffer = malloc(mb + 4);
	int bloqueId = 0;
	char data[pedido2.size - 2];

	switch (pedido2.idMensaje) {
	case mensajeEnvioBloqueANodo:
		//serial_unpack(pedido2.envio + sizeof(header), "h", &bloqueId);
		memcpy(data, pedido2.envio + sizeof(int), pedido2.size-sizeof(int));
		printf("--------------------------%s\n ", data);
		setBloque(bloqueId, data);
		memset(data, 0, pedido2.size - 2);
		break;

	default:
		break;
	}
}

void escucharAlFS(int socketFs) {
	int success = 1;
	while (1) {
		//pedido = desempaquetar(socketFs);
		//memcpy(&bloqueMock, pedido.envio, sizeof(int));
		recibirMensajesFileSystem(socketFs);
		//free(envio);
		//success = setBloque(bloqueMock, envio);
		empaquetar(socketFs, mensajeRespuestaEnvioBloqueANodo, sizeof(int),
				&success);
	}
}


