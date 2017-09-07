/*
 * YAMA.h
 *
 *  Created on: 6/9/2017
 *      Author: utnso
 */

#ifndef YAMA_H_
#define YAMA_H_

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
#include <commons/log.h>
#include "Serializacion.h"

#define idMaster 2

int mostrarLoggerPorPantalla=1;
fd_set master;   // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
int fdmax;        // número máximo de descriptores de fichero
int servidor;     // descriptor de socket a la escucha
int nuevoMaster;        // descriptor de socket de nueva conexión aceptada
char buf[256];    // buffer para datos del cliente
int nbytes;
int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
int addrlen;
int i, j;
struct sockaddr_in direccionCliente;

void levantarServidorYama();
void conectarseConFs();
int crearServidorAsociado(char* ip, int puerto);


#endif /* YAMA_H_ */
