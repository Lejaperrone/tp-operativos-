/*
 * FuncionesMaster.h
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#ifndef FUNCIONESMASTER_H_
#define FUNCIONESMASTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <Configuracion.h>
#include <commons/string.h>
#include <Globales.h>
#include <Serializacion.h>
#include <pthread.h>
#include <commons/log.h>

#define mensajeArchivo 2
#define mensajeEtapaTransformacion 5

/*----VARIABLES GLOBALES----*/
t_log* loggerMaster;
int socketYama;
/*--------------------------*/

typedef struct{
	char* ip;
	int port;
	int id;
} parametrosConexionMaster;

void conectarseConYama(char* ip, int port);

void* conectarseConWorkers(parametrosConexionMaster* parametros);

void enviarJobAYama();

void esperarInstruccionesDeYama();

char* recibirRuta(char* mensaje);

#endif /* FUNCIONESMASTER_H_ */
