#include "YAMA.h"
#include "Sockets.c"
#include "Configuracion.c"

t_log* logger;

int main(void) {

	struct configuracionYama config;
	logger = log_create("logYama", "YAMA.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);
	//conectarseConFS();

	cargarConfiguracionYama(&config);

	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);

	return EXIT_SUCCESS;
}

void conectarseConFs(){
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 3000);
	conectarCon(direccion, socketFs, 1);
}


