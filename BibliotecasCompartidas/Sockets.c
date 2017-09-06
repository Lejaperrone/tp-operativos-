/*
 * Sockets.c
 *
 *  Created on: 16/4/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Sockets.h"

#define mensajeIdentificacion 1

int crearSocket() {
	int sockete;
	if ((sockete = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error al crear el socket");
		exit(1);
	}
	return sockete;
}

void conectarCon(struct sockaddr_in direccionServidor, int cliente,	int tipoCliente) { //Agregar un parametro para que cada cliente le envie su tipo

	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) == -1) {
		perror("No se pudo conectar");
		exit(1);
	}
	void* paqueteId = &tipoCliente;
	//empaquetar(cliente, mensajeIdentificacion, 0, paqueteId);
}

void enviarMensajeA(int *socket, int longitud) {
	char mensaje[longitud];
	while (1) {
		bzero(mensaje, longitud);
		printf("Introduzca mensaje: ");
		scanf("%s", mensaje);
		send((*socket), mensaje, strlen(mensaje), 0);
	}
}

struct sockaddr_in cargarDireccion(char* direccionIP, int puerto) {
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = inet_addr(direccionIP);
	direccion.sin_port = htons(puerto);
	return direccion;
}

void recibirMensajes(int cliente) {
	char* buffer = malloc(30);
	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 30, 0);
		if (bytesRecibidos <= 0) {
			perror("Problema de conexión");
			exit(1);
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llego este mensaje: %s\n", buffer);
	}
	free(buffer);
}

void asociarSocketA(struct sockaddr_in direccionServidor, int servidor) {
	if (bind(servidor,(void*) &direccionServidor, sizeof(direccionServidor)) == -1) {
		perror("Falló el bind");
		exit(1);
	}

	printf("Estoy escuchando\n");
	listen(servidor, SOMAXCONN);
}



