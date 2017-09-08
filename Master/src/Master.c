/*
 ============================================================================
 Name        : Master.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Master.h"

int main(void) {

	cargarConfiguracionMaster(&config);

	conectarseConYama(config.YAMA_IP,config.YAMA_PUERTO);

	return EXIT_SUCCESS;
}

void conectarseConYama(char* ip, int port) {
	int socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketYama, idMaster);
}
