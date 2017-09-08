#include "YAMA.h"

t_log* logger;

int main(void) {
	logger = log_create("logYama", "YAMA.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);
	//conectarseConFS();

	cargarConfiguracionYama(&config);

	levantarServidorYama("127.0.0.1",3000);//FIXME: cambiar configuracion y agregar puerto e ip de yama

	return EXIT_SUCCESS;
}

void conectarseConFs(){
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 3000);
	conectarCon(direccion, socketFs, 1);
}


