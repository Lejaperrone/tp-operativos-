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

int main(void) {

	cargarConfiguracionMaster(&config);

	conectarseConYama(config.YAMA_IP,config.YAMA_PUERTO);

	char* rutaTrans = recibirRuta("Ingrese ruta transformador");
	char* rutaRedu = recibirRuta("Ingrese ruta reductor");
	char* rutaDatos = recibirRuta("Ingrese ruta de los datos");
	char* rutaAlma = recibirRuta("Ingrese ruta donde almacenar");

	conectarseConWorkers("127.0.0.1", 5000);

	return EXIT_SUCCESS;
}


