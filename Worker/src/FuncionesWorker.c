#include "FuncionesWorker.h"

void esperarConexionesMaster(char* ip, int port) {
	levantarServidorWorker(ip, port);
}

void esperarJobDeMaster() {
	respuesta instruccionMaster;
	log_trace(logger, "Esperando instruccion de Master");

	instruccionMaster = desempaquetar(socketMaster);

	switch (instruccionMaster.idMensaje) {
	case mensajeProcesarTransformacion:

		log_trace(logger, "Iniciando Transformacion");
		//Recibir script, origen de datos (porcion databin) y destino (archivo temporal)
		char* destino = "/tmp/resultado";
		char* contenidoScript = "transformador";
		int offset = 100;
		int bytesOcupados = 50;

		crearScript(contenidoScript, mensajeProcesarTransformacion);
		free(contenidoScript);

		char* command = string_from_format("head -c %d < %s | tail -c %d | sh %s/transformador.sh | sort > %s",
				offset, config.RUTA_DATABIN, bytesOcupados,
				config.RUTA_DATABIN, destino);

		int status;
		if ((status = system(command)) < 0) {
			log_error(logger,"NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA TRANSFORMACION");
			empaquetar(socketMaster,mensajeError,0,&status);
		}
		free(command);
		log_trace(logger,"Status transformacion:%d", status);
		empaquetar(socketMaster,mensajeOk,0,&status);

		break;
	case mensajeProcesarRedLocal:

		log_trace(logger, "Iniciando Reduccion Local");

		//Recibir script, origen de datos (archivo temporal del fs local) y destino (archivo temporal del fs local)

		break;
	case mensajeProcesarRedGlobal:

		log_trace(logger, "Iniciando Reduccion Global");

		//Recibir script, origen de datos (archivo temporal del fs local) y destino (archivo temporal del fs local)
		break;
	}
}

void crearScript(char * bufferScript, int etapa) {
	int aux, auxChmod;
	char mode[] = "0777";
	FILE* script;
	aux = string_length(bufferScript);
	printf("size archivo:%d\n", aux);
	if (etapa == mensajeProcesarTransformacion) {
		printf("Se crea el archivo transformador\n");
		char* ruta = string_from_format("%s/transformador.sh",config.RUTA_DATABIN);
		script = fopen(ruta, "w+");
	} else {
		printf("Se crea el archivo reductor\n");
		char* ruta = string_from_format("%s/reductor.sh",config.RUTA_DATABIN);
		script = fopen(ruta, "w+");
	}
	fwrite(bufferScript, sizeof(char), aux, script);
	auxChmod = strtol(mode, 0, 8);
	if (chmod(config.RUTA_DATABIN, auxChmod) < 0) {
		log_error(logger,"NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	fclose(script);
}

void ejecutarTransformacion() {		//PARA PROBAR
	/*pid_t pid;
	 pid = fork();
	 if(pid == -1){
	 log_error(logger,"Error al crear el hijo");
	 }else if(pid == 0){
	 //logica del hijo
	 }else{
	 //logica del padre
	 }*/
	//system("echo Ejecutando Transformacion | ./script_prueba > /tmp/resultado" );
	system("echo Ejecutando Transformacion");
}

void levantarServidorWorker(char* ip, int port) {
	struct sockaddr_in direccionCliente;
	int server;
	fd_set master;
	fd_set read_fds;
	int fdmax;
	int i, j;
	int tamanioDireccion;
	respuesta conexionNueva;
	server = crearServidorAsociado(ip, port);
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(server, &master);

	fdmax = server;

	while (1) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == server) {
					// gestionar nuevas conexiones
					tamanioDireccion = sizeof(direccionCliente);
					if ((socketMaster = accept(server,
							(struct sockaddr *) &direccionCliente,
							&tamanioDireccion)) == -1) {
						perror("accept");
					} else {
						FD_SET(socketMaster, &master);
						if (socketMaster > fdmax) {
							fdmax = socketMaster;
						}
						realizarHandshake(socketMaster);

						esperarJobDeMaster();
					}
				} else {
					// gestionar datos de un cliente

				}
			}
		}
	}
}

void realizarHandshake(int socket) {
	respuesta conexionNueva;
	conexionNueva = desempaquetar(socket);

	if (conexionNueva.idMensaje == 1) {
		if (*(int*) conexionNueva.envio == 2) {
			log_trace(logger, "Conexion con Master establecida");
		}
	}
}

