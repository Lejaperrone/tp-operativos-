/*
 ============================================================================
 Name        : FileSystem.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "FileSystem.h"

int sizeBloque = 1048576; // 1mb
int mostrarLoggerPorPantalla = 1;

void levantarServidorFS(int servidor, int cliente){
	char* buffer = malloc(300);
	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(direccionCliente);
	struct sockaddr_in direccionServidor = cargarDireccion("127.0.0.1",6000);
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	asociarSocketA(direccionServidor, servidor);

		cliente = accept(servidor, (struct sockaddr *) &direccionCliente, &tamanioDireccion);
	while(1){

	}

	//falta agregar el manejo de error cuando se desconecta el fs,
	//handshake y el protocolo de envio de mensajes
	free(buffer);
}

int main(void) {

	int sizeComando = 256;
	int clienteYama = 0;
	int servidorFS = crearSocket();

	t_log* logger = log_create("logFileSystem", "FileSystem.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	//levantarServidorFS(servidorFS, clienteYama);

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando);
		bzero(comando, sizeComando);
		comando = readline(">");
		if (comando)
			add_history(comando);

		log_trace(logger, "El usuario ingreso: %s", comando);

		if (string_starts_with(comando, "format")) {
			log_trace(logger, "File system formateado");
		}
		else if (string_starts_with(comando, "rm -d")) {
			log_trace(logger, "Directorio eliminado");
		}
		else if (string_starts_with(comando, "rm -b")) {
			log_trace(logger, "Bloque eliminado");
		}
		else if (string_starts_with(comando, "rm")) {
			log_trace(logger, "Archivo eliminado");
		}
		else if (string_starts_with(comando, "rename")) {
			log_trace(logger, "Archivo renombrado");
		}
		else if (string_starts_with(comando, "mv")) {
			log_trace(logger, "Archivo movido");
		}
		else if (string_starts_with(comando, "cat")) {
			log_trace(logger, "Archivo mostrado");
		}
		else if (string_starts_with(comando, "mkdir")) {
			log_trace(logger, "Directorio creado"); // avisar si ya existe
		}
		else if (string_starts_with(comando, "cpfrom")) {
			log_trace(logger, "Archivo copiado a yamafs");
		}
		else if (string_starts_with(comando, "cpto")) {
			log_trace(logger, "Archivo copiado desde yamafs");
		}
		else if (string_starts_with(comando, "cpblock")) {
			log_trace(logger, "Bloque copiado en el nodo");
		}
		else if (string_starts_with(comando, "md5")) {
			log_trace(logger, "MD5 del archivo");
		}
		else if (string_starts_with(comando, "ls")) {
			log_trace(logger, "Archivos listados");
		}
		else if (string_starts_with(comando, "info")) {
			log_trace(logger, "Mostrando informacion del archivo");
		}
		else {
			printf("Comando invalido\n");
			log_trace(logger, "Comando invalido");
		}
		free(comando);
	}
}

