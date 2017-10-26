#include "FuncionesWorker.h"

struct configuracionNodo config;

int main(int argc, char *argv[]) {
	limpiarPantalla();
	logger = log_create("logWorker", "Worker.c", 1, LOG_LEVEL_TRACE);
	cargarConfiguracionNodo(&config,argv[1]);
	levantarServidorWorker(config.IP_NODO, config.PUERTO_WORKER);

	return EXIT_SUCCESS;
}
