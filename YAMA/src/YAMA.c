#include "FuncionesYama.h"

int main(int argc, char *argv[]) {
	limpiarPantalla();
	logger = log_create("logYama", "YAMA.c", 1, LOG_LEVEL_TRACE);

	nodosConectados = list_create();
	pthread_mutex_init(&mutex_NodosConectados, NULL);
	pthread_mutex_init(&mutexLog, NULL);

	cargarConfiguracionYama(&config,argv[1]);
	inicializarEstructuras();

	socketFs = conectarseConFs();

	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);

	return EXIT_SUCCESS;
}

