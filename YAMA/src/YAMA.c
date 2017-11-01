#include "FuncionesYama.h"

int main(int argc, char *argv[]) {
	limpiarPantalla();
	logger = log_create("logYama", "YAMA.c", 1, LOG_LEVEL_TRACE);
	pthread_t hiloConfig;

	nodosConectados = list_create();
	tablaDeEstados = list_create();
	pthread_mutex_init(&mutexTablaEstados, NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexConfiguracion, NULL);
	pthread_mutex_init(&mutexLog, NULL);

	cargarConfiguracionYama(&config,argv[1]);
	inicializarEstructuras();

	socketFs = conectarseConFs();

	pthread_create(&hiloConfig,NULL,(void*)&manejarConfig,NULL);
	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);
	pthread_join(hiloConfig,NULL);

	return EXIT_SUCCESS;
}

