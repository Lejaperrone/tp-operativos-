/*
 ============================================================================
 Name        : FileSystem.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "FileSystem.h"

int main(void) {
	int clienteYama = 0;
	int servidorFS = crearSocket();

	levantarServidorFS(servidorFS, clienteYama);

	return EXIT_SUCCESS;
}

void levantarServidorFS(int servidor, int cliente){
	char* buffer = malloc(300);
	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(direccionCliente);
	struct sockaddr_in direccionServidor = cargarDireccion("127.0.0.1",6000);
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	asociarSocketA(direccionServidor, servidor);

		cliente = accept(servidor, (struct sockaddr *) &direccionCliente, &tamanioDireccion);
	while(1){

	}

	//falta agregar el manejo de error cuando se desconecta el fs,
	//handshake y el protocolo de envio de mensajes
	free(buffer);
}
