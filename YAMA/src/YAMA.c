/*
 ============================================================================
 Name        : YAMA.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "YAMA.h"


int main(void) {
	int socketFs = crearSocket();

	cargarConfiguracionYama(&config);

	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 6000);//6000 PUERTO FS.
	conectarCon(direccion, socketFs, 1);

	levantarServidorYama();

	return EXIT_SUCCESS;
}


