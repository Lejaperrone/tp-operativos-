/*
 ============================================================================
 Name        : Worker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <Configuracion.h>

struct configuracionNodo config;

int main(void) {

	cargarConfiguracionNodo(&config);

	return EXIT_SUCCESS;
}
