#include "YAMA.h"
#include "Sockets.h"
#include "Sockets.c"
#include "Configuracion.h"

t_log* logger;

int main(int argc, char *argv[]) {
	struct configuracionYama config;
	logger = log_create("logYama", "YAMA.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);
	//conectarseConFs();

	cargarConfiguracionYama(&config,argv[1]);

	levantarServidorYama(config.YAMA_IP,config.YAMA_PUERTO);

	return EXIT_SUCCESS;
}

void conectarseConFs(){
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 6000);
	conectarCon(direccion, socketFs, 1);
}
void levantarServidorYama(char* ip, int port){
	respuesta conexionNueva;
	int bufferPrueba = 2;
	servidor = crearServidorAsociado(ip, port);
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// añadir listener al conjunto maestro
	FD_SET(servidor, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = servidor; // por ahora es éste
	// bucle principal
	while(1){
		read_fds = master; // cópialo
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == servidor) {
					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((nuevoMaster = accept(servidor, (struct sockaddr *)&direccionCliente,
							&addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(nuevoMaster, &master); // añadir al conjunto maestro
						if (nuevoMaster > fdmax) {    // actualizar el máximo
							fdmax = nuevoMaster;
						}
						conexionNueva = desempaquetar(nuevoMaster);
						int idRecibido = *(int*)conexionNueva.envio;

						if (idRecibido == idMaster){
							log_trace(logger, "Conexion de Master\n");
							empaquetar(nuevoMaster,5,0,&bufferPrueba);//FIXME:SOLO A MODO DE PRUEBA
							//hacer algo despues del handshake
						}
					}
				} else {
					// gestionar datos de un cliente

				}
			}
		}
	}

}



