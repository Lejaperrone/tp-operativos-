/*
 ============================================================================
 Name        : Master.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "FuncionesMaster.h"

struct configuracionMaster config;

int main(int argc, char *argv[]) {
	limpiarPantalla();
	job* miJob;
	loggerMaster = log_create("logMaster", "Master.c", 1, LOG_LEVEL_TRACE);
	//controlarParametros(argc);
	cargarConfiguracionMaster(&config,argv[1]);

	conectarseConYama(config.YAMA_IP,config.YAMA_PUERTO);

	miJob = crearJob(argv);

	enviarJobAYama(miJob);

	esperarInstruccionesDeYama();

	return EXIT_SUCCESS;
}


