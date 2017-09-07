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
	char buffer[256];
	int socketYama = crearSocket();

	cargarConfiguracionMaster(&config);

	struct sockaddr_in direccion = cargarDireccion(config.YAMA_IP,config.YAMA_PUERTO);//5000 PUERTO YAMA.
	conectarCon(direccion, socketYama, 2);

	//recv(socketYama, buffer, sizeof(buffer), 0);

	return EXIT_SUCCESS;
}
