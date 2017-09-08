/*
 * Master.h
 *
 *  Created on: 6/9/2017
 *      Author: utnso
 */

#ifndef MASTER_H_
#define MASTER_H_

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

#define idMaster 2

struct configuracionMaster config;
void conectarseConYama(char* ip, int port);

#endif /* MASTER_H_ */
