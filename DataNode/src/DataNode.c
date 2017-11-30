#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include "Configuracion.h"
#include "FuncionesDN.h"
#include "Serializacion.h"

struct configuracionNodo  config;

int main(int argc, char *argv[]) {
	limpiarPantalla();
	cargarConfiguracionNodo(&config,argv[1]);
	logger = log_create("logWorker", "Worker.c", 1, LOG_LEVEL_TRACE);
	inicializarDataBin();
	conectarseConFs();
	return EXIT_SUCCESS;
}
