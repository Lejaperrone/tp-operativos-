/*
 * FuncionesWorker.h
 *
 *  Created on: 24/9/2017
 *      Author: utnso
 */

#ifndef FUNCIONESWORKER_H_
#define FUNCIONESWORKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <Configuracion.h>
#include <commons/log.h>
#include "Serializacion.h"
#include <commons/log.h>
#include "FuncionesWorker.h"

/*------VARIABLES-------------*/
struct configuracionNodo config;
struct sockaddr_in direccionCliente;
int socketMaster;
t_log* logger;

/*----PROTOTIPOS--------------------*/
void esperarConexionesMaster(char* ip, int port);
void esperarJobDeMaster();
int levantarServidorWorker(char* ip, int port);
void realizarHandshake(int socket);

#endif /* FUNCIONESWORKER_H_ */
