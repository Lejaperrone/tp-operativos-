#include "FuncionesYama.h"

int main(int argc, char *argv[]) {
	struct configuracionYama config;
	logger = log_create("logYama", "YAMA.c", 1, LOG_LEVEL_TRACE);
	socketFs = conectarseConFs();
	cargarConfiguracionYama(&config,argv[1]);

	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);

	return EXIT_SUCCESS;
}

