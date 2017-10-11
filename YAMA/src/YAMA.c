#include "FuncionesYama.h"

int main(int argc, char *argv[]) {

	jobsAPlanificar = list_create();

	logger = log_create("logYama", "YAMA.c", 1, LOG_LEVEL_TRACE);

	struct configuracionYama config;
	cargarConfiguracionYama(&config,argv[1]);

	socketFs = conectarseConFs();

	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);

	iniciarListasPlanificacion();

	return EXIT_SUCCESS;
}

