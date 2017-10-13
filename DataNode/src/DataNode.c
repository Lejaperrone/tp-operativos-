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

int main(int argc, char *argv[]) {
	limpiarPantalla();

	respuesta conexionConFS;
	cargarConfiguracionNodo(&config,argv[1]);
	inicializarDataBin();
	conectarseConFs();
	while(1){

	}
	return EXIT_SUCCESS;
}
