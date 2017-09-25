/*
 ============================================================================
 Name        : Worker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "FuncionesWorker.h"

struct configuracionNodo config;

int main(int argc, char *argv[]) {
	logger = log_create("logWorker", "Worker.c", 1, LOG_LEVEL_TRACE);
	cargarConfiguracionNodo(&config,argv[1]);

	esperarConexionesMaster(config.IP_NODO, config.PUERTO_WORKER);

	while(1){

	}
	//forkear por cada tarea mandada por el master
	return EXIT_SUCCESS;
}
