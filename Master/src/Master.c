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
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1",5000);//5000 PUERTO YAMA.
	conectarCon(direccion, socketYama, 2);

	//recv(socketYama, buffer, sizeof(buffer), 0);

	return EXIT_SUCCESS;
}
