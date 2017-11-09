#include "FuncionesWorker.h"

void ejecutarComando(char * command, int socketAceptado) {
	int status;
	log_trace(logger, "COMANDO:%s\n", command);
	system(command);
	if ((status = system(command)) < 0) {
		log_error(logger, "NO SE PUDO EJECTUAR EL COMANDO EN SYSTEM, FALLA REDUCCION LOCAL");
		free(command);
		empaquetar(socketAceptado, mensajeError, 0, 0);
		exit(1);
	}
}

t_list* crearListaParaReducir() {
	//Aca hay que conectarse al worker capitan, mandar el contenido del archivo reducido y que ese lo aparee
	t_list* archivosAReducir = list_create();
	return archivosAReducir;
}

char *get_line(FILE *fp) {
	char *line = NULL;
	size_t len = 0;
	int r = getline(&line, &len, fp);
	return r != -1 ? line : NULL;
}

void traverse_nodes(t_list* list, void funcion(void*)) {

	t_link_element* next, *t_link_element = list->head;

	while (t_link_element != NULL) {
		next = t_link_element->next;
		funcion(t_link_element->data);
		t_link_element = next;
	}
}

void apareoArchivosLocales(t_list *sources, const char *target) {

	typedef struct {
		FILE *file;
		char *line;
	} t_cont;

	t_cont *map_cont(const char *source) {
		t_cont *cont = malloc(sizeof(cont));
		cont->file = fopen(source, "r");
		cont->line = NULL;
		return cont;
	}

	bool line_set(t_cont *cont) {
		return cont->line != NULL;
	}

	//TODO Corregir que pasa con el fin de archivo
	void read_file(t_cont *cont) {
		if (!line_set(cont)) {
			char *aux = get_line(cont->file);
			free(cont->line);
			cont->line = aux;
		}
	}

	bool compare_lines(t_cont *cont1, t_cont *cont2) {
		return strcmp(cont1->line, cont2->line) <= 0;
	}

	t_list* listaArchivos = list_map(sources, (void*) map_cont);
	FILE* resultado = fopen(target, "w+");

	t_list* list = list_create();
	list_add_all(list, listaArchivos);

	traverse_nodes(list, (void*) read_file);

	while (true) {

		traverse_nodes(list, (void*) read_file);
		t_list* aux = list_filter(list, (void*) line_set);

		list = aux;
		if (list_is_empty(list))
			break;
		list_sort(list, (void*) compare_lines);

		t_cont* cont = list_get(list, 0);

		fputs(cont->line, resultado);
		free(cont->line);
		cont->line = NULL;
	}

	void free_cont(t_cont *cont) {
		fclose(cont->file);
		free(cont);
	}

	list_map(listaArchivos, (void*) free_cont);
	fclose(resultado);
}

void crearScript(char* bufferScript, int etapa) {
	log_trace(logger, "Iniciando creacion de script");
	char mode[] = "0777";
	FILE* script;
	int aux = string_length(bufferScript);
	int auxChmod = strtol(mode, 0, 8);
	char* nombreArchivo;

	if (etapa == mensajeProcesarTransformacion)
		nombreArchivo = "transformador.sh";
	else if (etapa == mensajeProcesarRedLocal)
		nombreArchivo = "reductorLocal.pl";
	else if (etapa == mensajeProcesarRedGlobal)
		nombreArchivo = "reductorGlobal.pl";

	char* ruta = string_from_format("../scripts/%s", nombreArchivo);
	script = fopen(ruta, "w+");
	fwrite(bufferScript, sizeof(char), aux, script);
	auxChmod = strtol(mode, 0, 8);
	if (chmod(ruta, auxChmod) < 0) {
		log_error(logger, "NO SE PUDO DAR PERMISOS DE EJECUCION AL ARCHIVO");
	}
	log_trace(logger, "Script creado con permisos de ejecucion");
	fclose(script);
}

void handlerMaster(int clientSocket) {
	respuesta paquete;
	parametrosTransformacion* transformacion;
	char* destino;
	char* contenidoScript;
	char* command;
	char* rutaArchivoApareado;
	t_list* listaArchivosTemporales;
	t_list* archivosAReducir;

	paquete = desempaquetar(clientSocket);
	switch (paquete.idMensaje) {
	case mensajeProcesarTransformacion:
		transformacion = (parametrosTransformacion*)paquete.envio;
		log_trace(logger, "Iniciando Transformacion");
		contenidoScript = transformacion->contenidoScript.cadena;
		int numeroBloqueTransformado = transformacion->bloquesConSusArchivos.numBloque;
		int bloqueId = transformacion->bloquesConSusArchivos.numBloqueEnNodo;
		int bytesRestantes = transformacion->bloquesConSusArchivos.bytesOcupados;
		destino = transformacion->bloquesConSusArchivos.archivoTemporal.cadena;
		int offset = bloqueId * mb + bytesRestantes;
		crearScript(contenidoScript, mensajeProcesarTransformacion);
		log_trace(logger, "Aplicar transformacion en %i bytes del bloque %i",
				bytesRestantes, numeroBloqueTransformado);
		command =
				string_from_format(
						"head -c %d < %s | tail -c %d | sh %s | sort > /home/utnso/tp-2017-2c-PEQL/Worker/tmp/%s",
						offset, config.RUTA_DATABIN, bytesRestantes, "../scripts/transformador.sh", destino);
		ejecutarComando(command, clientSocket);
		log_trace(logger, "Transformacion realizada correctamente");
		empaquetar(clientSocket, mensajeTransformacionCompleta, 0, &numeroBloqueTransformado);
		free(transformacion);
		exit(1);
		break;
	case mensajeProcesarRedLocal:
		log_trace(logger, "Iniciando Reduccion Local");
		contenidoScript = "contenidoScript reductorLocal";
		listaArchivosTemporales = list_create(); //Recibir por socket la lista
		destino = "/tmp/resultado";
		rutaArchivoApareado = "/resultadoApareoLocal";
		crearScript(contenidoScript, mensajeProcesarRedLocal);
		apareoArchivosLocales(listaArchivosTemporales, rutaArchivoApareado);
		FILE* archivoTemporalDeReduccionLocal = fopen(destino, "w+");
		command = string_from_format(" %s | ./%s/reductor.sh > %s ",
				rutaArchivoApareado, config.RUTA_DATABIN,
				archivoTemporalDeReduccionLocal);
		ejecutarComando(command, clientSocket);
		log_trace(logger, "Reduccion local realizada correctamente");
		empaquetar(clientSocket, mensajeOk, 0, 0);
		exit(1);
		break;
	case mensajeProcesarRedGlobal:
		log_trace(logger, "Iniciando Reduccion Global");
		contenidoScript = "contenidoScript reductorGlobal";
		rutaArchivoApareado = "/resultadoApareoGlobal";
		destino = "/tmp/resultado";
		crearScript(contenidoScript, mensajeProcesarRedGlobal);
		archivosAReducir = crearListaParaReducir();
		apareoArchivosLocales(archivosAReducir, rutaArchivoApareado);
		FILE* archivoTemporalDeReduccionGlobal = fopen(destino, "w+");
		command = string_from_format("%s | ./%s/reductorGlobal.sh > %s",
				rutaArchivoApareado, config.RUTA_DATABIN,
				archivoTemporalDeReduccionGlobal);
		ejecutarComando(command, clientSocket);
		log_trace(logger, "Reduccion global realizada correctamente");
		empaquetar(clientSocket, mensajeOk, 0, 0);
		exit(1);
		break;
	default:
		break;
	}
}

void handlerWorker() {

}

void levantarServidorWorker(char* ip, int port) {
	int sock;
	sock = crearServidorAsociado(ip, port);

	while (1) {
		struct sockaddr_in their_addr;
		socklen_t size = sizeof(struct sockaddr_in);
		clientSocket = accept(sock, (struct sockaddr*) &their_addr, &size);
		int pid;

		if (clientSocket == -1) {
			close(clientSocket);
			perror("accept");
		}

		respuesta conexionNueva;
		conexionNueva = desempaquetar(clientSocket);

		if (conexionNueva.idMensaje == 1) {
			if (*(int*) conexionNueva.envio == 2) {
				log_trace(logger, "Conexion con Master establecida");
				if ((pid = fork()) == 0) {
					log_trace(logger, "Proceso hijo:%d", pid);
					log_trace(logger, "Esperando instruccion de Master");
					handlerMaster(clientSocket);
				} else if (pid > 0) {
					log_trace(logger, "Proceso Padre:%d", pid);
					//close(clientSocket);
					continue;
				} else if (pid < 0) {
					log_error(logger, "NO SE PUDO HACER EL FORK");
				}
			} else if (*(int*) conexionNueva.envio == 100) { //Averiguar cual es el id del worker
				log_trace(logger, "Conexion con Worker establecida");
				if ((pid = fork()) == 0) {
					log_trace(logger, "Proceso hijo de worker:%d", pid);
					handlerWorker();
				} else if (pid > 0) {
					log_trace(logger, "Proceso Worker Padre:%d", pid);
				} else if (pid < 0) {
					log_error(logger, "NO SE PUDO HACER EL FORK");
				}
			}
		}
	}
}
