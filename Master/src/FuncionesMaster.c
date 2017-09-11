/*
 * FuncionesMaster.c
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#include "FuncionesMaster.h"

void conectarseConYama(char* ip, int port) {
	int socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketYama, idMaster);
}

void conectarseConWorkers(char* ip, int port){
	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketWorker, idMaster);
}

char* recibirRuta(char* mensaje){
	printf("%s\n",mensaje);
	char* comando = malloc(sizeof(char)*256);
	bzero(comando,256);
	fgets(comando,256,stdin);
	string_trim(&comando);
	return comando;
}
