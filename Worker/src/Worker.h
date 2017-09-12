/*
 * Worker.h
 *
 *  Created on: 9/9/2017
 *      Author: utnso
 */

#ifndef WORKER_H_
#define WORKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <Configuracion.h>
#include <commons/log.h>
#include "Serializacion.h"
#include <commons/log.h>

#define mensajeHandshake 1
#define idMaster 2

/*------VARIABLES-------------*/
struct configuracionNodo config;
struct sockaddr_in direccionCliente;
int socketMaster;
int mostrarLoggerPorPantalla = 1;
t_log* logger;

/*----PROTOTIPOS--------------------*/
int levantarServidorWorker(char* ip, int port);
void realizarHandshake(int socket, respuesta conexion);

#endif /* WORKER_H_ */
