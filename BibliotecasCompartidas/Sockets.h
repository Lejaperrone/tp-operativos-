/*
 * Sockets.h
 *
 *  Created on: 18/4/2017
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include "Serializacion.h"
#include <netinet/in.h>
#include <sys/select.h>

#define idMaster 2

struct t_tam{
	int menu;
	int length;
};

int crearSocket();

void conectarCon(struct sockaddr_in direccionServidor, int cliente, int tipoCliente);

void enviarMensajeA(int *socket, int longitud);

struct sockaddr_in cargarDireccion(char* direccionIP, int puerto);

void recibirMensajes(int cliente);

void asociarSocketA(struct sockaddr_in direccionServidor, int servidor);


fd_set master;   // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
int fdmax;        // número máximo de descriptores de fichero
int servidor;     // descriptor de socket a la escucha
int nuevoMaster;        // descriptor de socket de nueva conexión aceptada
char buf[256];    // buffer para datos del cliente
int nbytes;
//int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
int addrlen;
int i, j;
struct sockaddr_in direccionCliente;

void levantarServidorYama(char* ip, int port);

int crearServidorAsociado(char* ip, int puerto);

void levantarServidorFS(int servidor, int cliente);

#endif /* SOCKETS_H_ */
