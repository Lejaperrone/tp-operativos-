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
#include "Configuracion.c"
#include "FuncionesDN.h"
#include "Serializacion.h"

struct configuracionNodo  config;

int main(void) {
	respuesta conexionConFS;

	cargarConfiguracionNodo(&config);
	conectarseConFs();
	while(1){

	}
	puts("!!!Hello World!!!");
	return EXIT_SUCCESS;
}
