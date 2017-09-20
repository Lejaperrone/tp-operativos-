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

struct configuracionNodo config;

int main(int argc, char *argv[]) {
	respuesta conexionConMaster;

	logger = log_create("logYama", "YAMA.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	cargarConfiguracionNodo(&config,argv[1]);

	socketMaster = levantarServidorWorker(config.IP_NODO,config.PUERTO_WORKER);//FIXME: CAMBIAR ARCHIVO CONFIGURACION CON IP NODO

	conexionConMaster = desempaquetar(socketMaster);
	realizarHandshake(socketMaster, conexionConMaster);
	while(1){
		//para que no cierre
	}
	//forkear por cada tarea mandada por el master
	return EXIT_SUCCESS;
}
int levantarServidorWorker(char* ip, int port){
	unsigned int tamanioDireccion;
	int server = crearServidorAsociado(ip,port);
	return accept(server, (void*) &direccionCliente, &tamanioDireccion);

}

void realizarHandshake(int socket, respuesta conexion){
	if (conexion.idMensaje == 1){ //que sea mensaje handshake
		int idMensaje = *(int*) conexion.envio;
		if (idMensaje == idMaster){//HANDSHAKE, RECONOCIA A MASTER
			log_trace(logger, "Conexion de Master con id:\n");
			//logica con el master
		}
		close(socket);
	}
}
