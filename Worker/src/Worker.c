/*
 ============================================================================
 Name        : Worker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <Configuracion.h>
#include <Configuracion.c>
#include <commons/log.h>
#include <commons/bitarray.h>

struct configuracionNodo config;

int main(void) {
	respuesta conexionConMaster;

	logger = log_create("logYama", "YAMA.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	cargarConfiguracionNodo(&config);

	socketMaster = levantarServidorWorker(config.IP_NODO,config.PUERTO_WORKER);//FIXME: CAMBIAR ARCHIVO CONFIGURACION CON IP NODO

	conexionConMaster = desempaquetar(socketMaster);
	realizarHandshake(conexionConMaster);

	//forkear por cada tarea mandada por el master
	return EXIT_SUCCESS;
}
int levantarServidorWorker(char* ip, int port){
	unsigned int tamanioDireccion;
	int server = crearServidorAsociado(ip,port);
	return accept(server, (void*) &direccionCliente, &tamanioDireccion);

}

void realizarHandshake(respuesta conexion){
	if (conexion.idMensaje == 1){ //que sea mensaje handshake
		int idMensaje = *(int*) conexion.envio;
		if (idMensaje == idMaster){//HANDSHAKE, RECONOCIA A MASTER
			log_trace(logger, "Conexion de Master");
			//logica con el master
		}
	}
}
