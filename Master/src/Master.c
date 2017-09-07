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
	conectarseConYama();

	return EXIT_SUCCESS;
}

void conectarseConYama() {
	int socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 5000);	//5000 PUERTO YAMA.
	conectarCon(direccion, socketYama, idMaster);
}
