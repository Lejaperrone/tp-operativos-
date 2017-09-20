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
	loggerMaster = log_create("logMaster", "Master.c", 1, LOG_LEVEL_TRACE);

	cargarConfiguracionMaster(&config,argv[1]);

	conectarseConYama(config.YAMA_IP,config.YAMA_PUERTO);

	/*char* rutaTrans = recibirRuta("Ingrese ruta transformador");
	char* rutaRedu = recibirRuta("Ingrese ruta reductor");
	char* rutaDatos = recibirRuta("Ingrese ruta de los datos");
	char* rutaAlma = recibirRuta("Ingrese ruta donde almacenar");*/


	enviarJobAYama();//antes hay que crear un job para enviarle a yama con todos esos archivos

	esperarInstruccionesDeYama();

	return EXIT_SUCCESS;
}


