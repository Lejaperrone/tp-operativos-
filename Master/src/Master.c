#include "FuncionesMaster.h"

struct configuracionMaster config;

int main(int argc, char *argv[]) {
	limpiarPantalla();

	if(argc < 5) {
		puts("Faltan argumentos");
		return EXIT_FAILURE;
	}

	job* miJob;
	loggerMaster = log_create("logMaster", "Master.c", 1, LOG_LEVEL_TRACE);

	cargarConfiguracionMaster(&config,argv[1]);
	conectarseConYama(config.YAMA_IP,config.YAMA_PUERTO);
	miJob = crearJob(argv);
	enviarJobAYama(miJob);
	esperarInstruccionesDeYama();
	crearHilosConexion();

	return EXIT_SUCCESS;
}


