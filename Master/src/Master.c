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
job* miJob;

int main(int argc, char *argv[]) {
	loggerMaster = log_create("logMaster", "Master.c", 1, LOG_LEVEL_TRACE);
	//controlarParametros(argc);
	cargarConfiguracionMaster(&config,argv[1]);

	conectarseConYama(config.YAMA_IP,config.YAMA_PUERTO);

	//miJob = crearJob(argv);

	//enviarJobAYama();

	enviarArchivo(socketYama,"/home/utnso/hola"); //Prueba del enviar archivo

	esperarInstruccionesDeYama();

	return EXIT_SUCCESS;
}


