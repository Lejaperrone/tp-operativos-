/*
 * Sockets.c

 *
 *  Created on: 16/4/2017
 *      Author: utnso
 */

#include "Sockets.h"

int crearSocket() {
	int sockete;
	if ((sockete = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error al crear el socket");
		exit(1);
	}
	return sockete;
}

void conectarCon(struct sockaddr_in direccionServidor, int cliente,	int tipoCliente) { //Agregar un parametro para que cada cliente le envie su tipo

	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) == -1) {
		perror("No se pudo conectar");
		exit(1);
	}

	empaquetar(cliente, mensajeHandshake, 0, &tipoCliente);
}

void enviarMensajeA(int *socket, int longitud) {
	char mensaje[longitud];
	while (1) {
		bzero(mensaje, longitud);
		printf("Introduzca mensaje: ");
		scanf("%s", mensaje);
		send((*socket), mensaje, strlen(mensaje), 0);
	}
}

struct sockaddr_in cargarDireccion(char* direccionIP, int puerto) {
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = inet_addr(direccionIP);
	direccion.sin_port = htons(puerto);
	memset(&(direccion.sin_zero), '\0', 8);
	return direccion;
}

void recibirMensajes(int cliente) {
	char* buffer = malloc(30);
	while (1) {
		int bytesRecibidos = recv(cliente, buffer, 30, 0);
		if (bytesRecibidos <= 0) {
			perror("Problema de conexión");
			exit(1);
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llego este mensaje: %s\n", buffer);
	}
	free(buffer);
}

void asociarSocketA(struct sockaddr_in direccionServidor, int servidor) {
	if (bind(servidor,(void*) &direccionServidor, sizeof(direccionServidor)) == -1) {
		perror("Falló el bind");
		exit(1);
	}

	printf("Estoy escuchando\n");
	listen(servidor, SOMAXCONN);
}

int crearServidorAsociado(char* ip, int puerto) {
	int servidor = crearSocket();
	struct sockaddr_in direccionServidor = cargarDireccion(ip, puerto);
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
	asociarSocketA(direccionServidor, servidor);
	return servidor;
}

void levantarServidorYama(){
	servidor = crearServidorAsociado("127.0.0.1", 5000);
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// añadir listener al conjunto maestro
	FD_SET(servidor, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = servidor; // por ahora es éste
	// bucle principal
	while(1){
		read_fds = master; // cópialo
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == servidor) {
					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((nuevoMaster = accept(servidor, (struct sockaddr *)&direccionCliente, &addrlen)) == -1)
					{
						perror("accept");
					} else {
						FD_SET(nuevoMaster, &master); // añadir al conjunto maestro
						if (nuevoMaster > fdmax) {    // actualizar el máximo
							fdmax = nuevoMaster;
						}
						printf("Nueva conexion de %s en el socket %d\n", inet_ntoa(direccionCliente.sin_addr), nuevoMaster);
					}
				} else {
					// gestionar datos de un cliente
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
						// error o conexión cerrada por el cliente
						if (nbytes == 0) {
							// conexión cerrada
							printf("socket %d se desconecto\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						// tenemos datos de algún cliente
						for(j = 0; j <= fdmax; j++) {
							// ¡enviar a to do el mundo!
							if (FD_ISSET(j, &master)) {
								// excepto al listener y a nosotros mismos
								if (j != servidor && j != i) {
									if (send(j, buf, nbytes, 0) == -1) {
										perror("send");
									}
								}
							}
						}
					}
				}
			}
		}
	}

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
