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
#include <sys/types.h>

#define mb 1048576

int cantBloques = 50;
extern struct configuracionNodo config;
extern sem_t pedidoFS;
FILE* databin;

void enviarBloqueAFS(int numeroBloque) {

}//cpfrom /home/utnso/hola2.txt hola/chau

int setBloque(int numeroBloque, char* datos) {
	int fd = open(config.RUTA_DATABIN, O_RDWR);
	char* mapaDataBin = mmap(0, mb, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mb*numeroBloque);
	memcpy(mapaDataBin, datos, strlen(datos));
	if (msync(mapaDataBin, mb, MS_SYNC) == -1)
	{
		perror("Could not sync the file to disk");
	}
	if (munmap(mapaDataBin, mb) == -1)
	{
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);
	return 1;
}

void inicializarDataBin(){
	databin = fopen(config.RUTA_DATABIN,"a+");
	truncate(config.RUTA_DATABIN, config.SIZE_NODO*mb);
}

void conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(config.IP_FILESYSTEM, config.PUERTO_FILESYSTEM);
	conectarCon(direccion, socketFs, 3);
	informacionNodo info;
	info.sizeNodo = config.SIZE_NODO;
	info.bloquesOcupados = -1;
	info.numeroNodo = atoi(string_substring_from(config.NOMBRE_NODO, 4));
	printf("soy el nodo %d\n", info.numeroNodo);
	info.socket = -1;
	empaquetar(socketFs, mensajeInformacionNodo, sizeof(informacionNodo),
			&info);
	escucharAlFS(socketFs);
}

void recibirMensajesFileSystem(int socketFs) {
	respuesta numeroBloque = desempaquetar(socketFs);
	respuesta bloqueArchivo;
	//char* buffer = malloc(mb + 4);
	int bloqueId;
	memcpy(&bloqueId, numeroBloque.envio, sizeof(int));
	char* data;

	switch (bloqueArchivo.idMensaje) {
	case mensajeEnvioBloqueANodo:
		bloqueArchivo = desempaquetar(socketFs);
		data = malloc(bloqueArchivo.size);
		//serial_unpack(pedido2.envio + sizeof(header), "h", &bloqueId);
		memcpy(data, bloqueArchivo.envio + sizeof(int), bloqueArchivo.size-sizeof(int));
		//printf("--------------------------%s\n\n\n ", data);
		setBloque(bloqueId, data);
		free(data);
		break;

	default:
	printf("llegue %d %d\n", bloqueArchivo.idMensaje, mensajeEnvioBloqueANodo);
		break;
	}
	free(numeroBloque.envio);
	free(bloqueArchivo.envio);
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


