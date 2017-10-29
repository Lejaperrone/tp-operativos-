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
		crearScript(contenidoScript);
		command =
				string_from_format(
						"head -c %d < %s | tail -c %d | ./home/utnso/scripts/script.sh | sort > %s",
						offset, config.RUTA_DATABIN, bytesRestantes, destino);
		ejecutarComando(command, socketMaster);
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
		crearScript(contenidoScript);
		apareoArchivosLocales(listaArchivosTemporales, archivoAReducir);
		FILE* archivoTemporalDeReduccionLocal = fopen(destino, "w+");
		command = string_from_format(" %s | ./%s/reductor.sh > %s ",
				archivoAReducir, config.RUTA_DATABIN,
				archivoTemporalDeReduccionLocal);
		ejecutarComando(command, socketMaster);
		empaquetar(socketMaster, mensajeOk, 0, 0);
		exit(1);
		break;
	case mensajeProcesarRedGlobal:
		log_trace(logger, "Iniciando Reduccion Global");
		//Recibir script, origen de datos (archivo temporal del fs local) y destino (archivo temporal del fs local)
		contenidoScript = "contenidoScript reductorGlobal";
		archivoAReducir = "preReduccion";
		destino = "/tmp/resultado";
		crearScript(contenidoScript);
		archivosAReducir = crearListaParaReducir();
		apareoArchivosLocales(archivosAReducir, archivoAReducir);
		FILE* archivoTemporalDeReduccionGlobal = fopen(destino, "w+");
		command = string_from_format("%s | ./%s/reductorGlobal.sh > %s",
				archivoAReducir, config.RUTA_DATABIN,
				archivoTemporalDeReduccionGlobal);
		ejecutarComando(command, socketMaster);
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

	list_add(sources, "/home/utnso/pruebaApareo/temp1");
	list_add(sources, "/home/utnso/pruebaApareo/temp2");
	list_add(sources, "/home/utnso/pruebaApareo/temp3");

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

void crearScript(char * bufferScript) {
	int aux, auxChmod;
	char mode[] = "0777";
	FILE* script;
	aux = string_length(bufferScript);
	printf("Size Archivo:%d\n", aux);
	printf("Se crea el script\n");
	script = fopen("/home/utnso/scripts/script.sh", "w+");
	fwrite(bufferScript, sizeof(char), aux, script);
	auxChmod = strtol(mode, 0, 8);
	if (chmod("/home/utnso/scripts/script.sh", auxChmod) < 0) {
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
