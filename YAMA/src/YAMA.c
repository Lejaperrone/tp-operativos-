#include "FuncionesYama.h"

int main(int argc, char *argv[]) {

	logger = log_create("logYama", "YAMA.c", 1, LOG_LEVEL_TRACE);

	struct configuracionYama config;
	cargarConfiguracionYama(&config,argv[1]);
	inicializarEstructuras();

	socketFs = conectarseConFs();

	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);

	return EXIT_SUCCESS;
}

