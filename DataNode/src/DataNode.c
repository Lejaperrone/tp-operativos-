/*
 ============================================================================
 Name        : DataNode.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include "Configuracion.h"
#include "FuncionesDN.h"
#include "Serializacion.h"

struct configuracionNodo  config;

t_bitarray* bitmap;

int main(int argc, char *argv[]) {
	respuesta conexionConFS;
	cargarConfiguracionNodo(&config,argv[1]);
	levantarBitmap(config.NOMBRE_NODO);
	conectarseConFs();
	while(1){

	}
	puts("!!!Hello World!!!");
	return EXIT_SUCCESS;
}
