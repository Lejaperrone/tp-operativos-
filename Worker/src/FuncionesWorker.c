#include "FuncionesWorker.h"

void handlerMaster() {
	respuesta instruccionMaster;
	log_trace(logger, "Esperando instruccion de Master");

	instruccionMaster = desempaquetar(socketMaster);

	char* destino;
	char* contenidoScript;
	char* command;
	char* archivoAReducir;
	t_list* listaArchivosTemporales;
	t_list* archivosAReducir;
	switch (instruccionMaster.idMensaje) {
	case mensajeProcesarTransformacion:
		log_trace(logger, "Iniciando Transformacion");
		//Recibir de master script, origen de datos (bloque y bytesRestantes) y destino (archivo temporal)
		contenidoScript = "contenidoScript transformador";
		int bloqueId = 1;
		int bytesRestantes = 50;
		destino = "/tmp/resultado";
		int offset = bloqueId * mb + bytesRestantes;
		crearScript(contenidoScript, mensajeProcesarTransformacion);
		free(contenidoScript);
		command =
				string_from_format(
						"head -c %d < %s | tail -c %d | ./%s/transformador.sh | sort > %s",
						offset, config.RUTA_DATABIN, bytesRestantes,
						config.RUTA_DATABIN, destino);
		ejecutarComando(command,socketMaster);
		free(command);
		empaquetar(socketMaster, mensajeOk, 0, 0);
		exit(1);
		break;
	case mensajeProcesarRedLocal:
		log_trace(logger, "Iniciando Reduccion Local");
		//Recibir script, origen de datos (archivos temporales del fs local) y destino (archivo temporal del fs local)
		contenidoScript = "contenidoScript reductorLocal";
		listaArchivosTemporales = list_create();
		destino = "/tmp/resultado";
		archivoAReducir = "preReduccion";
		crearScript(contenidoScript, mensajeProcesarRedLocal);
		apareoArchivosLocales(listaArchivosTemporales, archivoAReducir);
		FILE* archivoTemporalDeReduccionLocal = fopen(destino, "w+");
		command = string_from_format(" %s | ./%s/reductor.sh > %s ",
				archivoAReducir, config.RUTA_DATABIN,
				archivoTemporalDeReduccionLocal);
		ejecutarComando(command,socketMaster);
		free(command);
		empaquetar(socketMaster, mensajeOk, 0, 0);
		exit(1);
		break;
	case mensajeProcesarRedGlobal:
		log_trace(logger, "Iniciando Reduccion Global");
		//Recibir script, origen de datos (archivo temporal del fs local) y destino (archivo temporal del fs local)
		contenidoScript = "contenidoScript reductorGlobal";
		archivoAReducir = "preReduccion";
		destino = "/tmp/resultado";
		crearScript(contenidoScript, mensajeProcesarRedGlobal);
		archivosAReducir = crearListaParaReducir();
		apareoArchivosLocales(archivosAReducir, archivoAReducir);
		FILE* archivoTemporalDeReduccionGlobal = fopen(destino, "w+");
		command = string_from_format("%s | ./%s/reductorGlobal.sh > %s", archivoAReducir,
				config.RUTA_DATABIN, archivoTemporalDeReduccionGlobal);
		ejecutarComando(command,socketMaster);
		free(command);
		empaquetar(socketMaster, mensajeOk, 0, 0);
		exit(1);
		break;
	}
}

void ejecutarComando(char * command, int socketMaster) {
	int status;
	if ((status = system(command)) < 0) {
		log_error(logger,
				"NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA REDUCCION LOCAL");
		empaquetar(socketMaster, mensajeError, 0, 0);
		free(command);
		exit(1);
	}
}

t_list* crearListaParaReducir() {
	//Aca hay que conectarse al worker capitan, mandar el contenido del archivo reducido y que ese lo aparee
	t_list* archivosAReducir = list_create();
	return archivosAReducir;
}

void apareoArchivosLocales(t_list *sources, const char *target) {

}

void crearScript(char * bufferScript, int etapa) {
	int aux, auxChmod;
	char mode[] = "0777";
	FILE* script;
	aux = string_length(bufferScript);
	printf("size archivo:%d\n", aux);
	if (etapa == mensajeProcesarTransformacion) {
		printf("Se crea el archivo transformador\n");
		char* ruta = string_from_format("%s/transformador.sh",
				config.RUTA_DATABIN);
		script = fopen(ruta, "w+");
	} else if (etapa == mensajeProcesarRedLocal) {
		printf("Se crea el archivo reductor local\n");
		char* ruta = string_from_format("%s/reductor.sh", config.RUTA_DATABIN);
		script = fopen(ruta, "w+");
	} else {
		printf("Se crea el archivo reductor global\n");
		char* ruta = string_from_format("%s/reductorGlobal.sh",
				config.RUTA_DATABIN);
		script = fopen(ruta, "w+");
	}
	fwrite(bufferScript, sizeof(char), aux, script);
	auxChmod = strtol(mode, 0, 8);
	if (chmod(config.RUTA_DATABIN, auxChmod) < 0) {
		log_error(logger, "NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	fclose(script);
}

void handlerWorker() {

}

void levantarServidorWorker(char* ip, int port) {
	int sock;
	sock = crearServidorAsociado(ip, port);

	while (1) {
		struct sockaddr_in their_addr;
		socklen_t size = sizeof(struct sockaddr_in);
		socketMaster = accept(sock, (struct sockaddr*) &their_addr, &size);
		int pid;

		if (socketMaster == -1) {
			perror("accept");
		}

		printf("Got a connection from %s on port %d\n",
				inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));

		pid = fork();
		if (pid == 0) {
			/* Proceso hijo */
			close(sock);

			respuesta conexionNueva;
			conexionNueva = desempaquetar(socketMaster);

			if (conexionNueva.idMensaje == 1) {
				if (*(int*) conexionNueva.envio == 2) {
					log_trace(logger, "Conexion con Master establecida");
					handlerMaster();
				} else if (*(int*) conexionNueva.envio == 100) { //Averiguar cual es el id del worker
					log_trace(logger, "Conexion con Worker establecida");
					handlerWorker();
				}
			}
		} else {
			/* Proceso padre */
			if (pid == -1) {
				perror("fork");
			} else {
				close(socketMaster);
			}
		}
	}
	close(sock);
}
