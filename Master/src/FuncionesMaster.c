/*
 * FuncionesMaster.c
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#include "FuncionesMaster.h"

void conectarseConYama(char* ip, int port) {
	int socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketYama, idMaster);
}

void conectarseConWorkers(char* ip, int port){
	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketWorker, idMaster);
}

char* recibirRuta(char* mensaje){
	printf("%s\n",mensaje);
	char* comando = malloc(sizeof(char)*256);
	bzero(comando,256);
	fgets(comando,256,stdin);
	string_trim(&comando);
	return comando;
}

void enviarArchivo(int socketPrograma, char* rutaArchivo) {
	FILE *archivo;
	char* stringArchivo;

	archivo = fopen(rutaArchivo, "r");
	fseek(archivo, 0L, SEEK_SET);

	char c = fgetc(archivo);
	stringArchivo = malloc(sizeof(char));
	int i = 0;

	while (!feof(archivo)) {
		 stringArchivo[i] = c;
		 ++i;
		 stringArchivo = realloc(stringArchivo, (i + 1) * sizeof(char));
		 c= fgetc(archivo);
	}
	stringArchivo[i] = '\0';

	fclose(archivo);

	string* archivoEnviar = malloc(sizeof(string));
	archivoEnviar->longitud = i;
	archivoEnviar->cadena = malloc(i+1);
	strcpy(archivoEnviar->cadena,stringArchivo);

	empaquetar(socketPrograma, mensajeArchivo,sizeof(int)+i,archivo);

	free(stringArchivo);
}
