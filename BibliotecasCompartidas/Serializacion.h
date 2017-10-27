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
#include "Globales.h"
#include <commons/collections/list.h>

#define mensajeDesconexion -1
#define mensajeHandshake 1
#define mensajeArchivo 2
#define mensajeOk 3
#define mensajeSolicitudTransformacion 4
#define mensajeEtapaTransformacion 5
#define mensajeEtapaReduccionLocal 6
#define mensajeEtapaReduccionGlobal 7
#define mensajeInformacionNodo 8
#define mensajeProcesarTransformacion 10
#define mensajeProcesarRedLocal 11
#define mensajeProcesarRedGlobal 12
#define mensajeProcesarAlmFinal 13
#define mensajeDesignarWorker 14
#define mensajeInfoArchivo 15
#define mensajeEnvioBloqueANodo 16
#define mensajeRespuestaEnvioBloqueANodo 17
#define mensajeNumeroCopiaBloqueANodo 18
#define mensajeRespuestaGetBloque 19
#define mensajeSolicitudInfoNodos 20
#define mensajeRespuestaInfoNodos 21
#define mensajeNumeroLecturaBloqueANodo 22
#define mensajeRespuestaTransformacion 23
#define mensajeError 24

typedef struct{
	int idMensaje;
	int tamanio;
} header;

typedef struct{
	int idMensaje;
	void* envio;
	int size;
} respuesta;

void empaquetar(int socket, int idMensaje,int tamanioS, void* paquete);
respuesta desempaquetar(int socket);

void* serializarString(void* paquete,int *tamanio);
string* deserializarString(int socket,int tamanio);

void* serializarJob(void* paquete, int* tamanio);
job* deserializarJob(int socket, int tamanio);

void* serializarSolicitudInfoNodos(void* paquete,int* tamanio);
solicitudInfoNodos* deserializarSolicitudInfoNodos(int socket,int tamanio);

void* serializarInformacionNodos(void* paquete,int* tamanio);
informacionNodo* deserializarInformacionNodos(int socket,int tamanio);

void* serializarRespuestaInfoNodos(void* paquete,int* tamanio);
informacionArchivoFsYama* deserializarRespuestaInfoNodos(int socket,int tamanio);

void* serializarRespuestaTransformacion(void* paquete,int* tamanio);
respuestaSolicitudTransformacion* deserializarRespuestaTransformacion(int socket,int tamanio);

#endif /* SERIALIZACION_H_ */
