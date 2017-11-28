#include "FuncionesYama.h"

int main(int argc, char *argv[]) {
	limpiarPantalla();
	logger = log_create("logYama", "YAMA.c", 0, LOG_LEVEL_TRACE);
	pthread_t hiloConfig;

	nodosConectados = list_create();
	tablaDeEstados = list_create();

	inicializarEstructuras();
	cargarConfiguracionYama(&config,argv[1]);
	log_trace(logger,"Estructuras cragdaas correctamente");

	socketFs = conectarseConFs();

	pthread_create(&hiloConfig,NULL,(void*)&manejarConfig,NULL);
	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);
	pthread_join(hiloConfig,NULL);

	return EXIT_SUCCESS;
}

