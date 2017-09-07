/*
 * Serializacion.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define mensajeHandshake 1

typedef struct{
	int idMensaje;
	int tamanio;
} header;

typedef struct{
	int idMensaje;
	void* envio;
} respuesta;

void empaquetar(int socket, int idMensaje,int tamanioS, void* paquete);
respuesta desempaquetar(int socket);

#endif /* SERIALIZACION_H_ */
