/*
 * FuncionesMaster.c
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#include "FuncionesMaster.h"

void conectarseConYama(char* ip, int port) {
	socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketYama, 2); //2 id master
	log_info(loggerMaster, "Conexion con Yama establecida"); //FIXME: FALTA CONTROL DE ERROR
}
void crearHilosConexion() {
	pthread_t hiloConexion;
	parametrosConexionMaster* parametrosConexion = malloc(
			sizeof(parametrosConexionMaster));
	//para probar conexion por hilos
	parametrosConexion->ip = "127.0.0.1";
	parametrosConexion->id = 2;
	parametrosConexion->port = 5000;
	if (pthread_create(&hiloConexion, NULL, (void *) conectarseConWorkers,parametrosConexion) != 0) {
		log_error(loggerMaster, "No se pudo crear el thread de conexion");
		exit(-1);
	}

}
void* conectarseConWorkers(parametrosConexionMaster* parametrosConexion) {
	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(parametrosConexion->ip,parametrosConexion->port);
	conectarCon(direccion, socketWorker, parametrosConexion->id); //2 id master
	log_trace(loggerMaster, "Conexion con Worker\n");
	return 0;
}

void enviarJobAYama() {
	//empaquetar los archivos y setear un job a enviar (crear struct job)
}

void esperarInstruccionesDeYama() {
	respuesta instruccionesYama = malloc(sizeof(respuesta));
	while (1) {
		instruccionesYama = desempaquetar(socketYama);
		switch (instruccionesYama.idMensaje) {

		case mensajeEtapaTransformacion:
			crearHilosConexion();
			//logica en etapa de transformacion
			break;
		}
	}
//recibir de yama todos los parametros conexion IP PUERTO
//ver que tipo de etapa es! los hilos de cada etapa son DISTINTOS!!!
}

char* recibirRuta(char* mensaje) {
	printf("%s\n", mensaje);
	char* comando = malloc(sizeof(char) * 256);
	bzero(comando, 256);
	fgets(comando, 256, stdin);
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
		c = fgetc(archivo);
	}
	stringArchivo[i] = '\0';

	fclose(archivo);

	string* archivoEnviar = malloc(sizeof(string));
	archivoEnviar->longitud = i;
	archivoEnviar->cadena = malloc(i + 1);
	strcpy(archivoEnviar->cadena, stringArchivo);

	empaquetar(socketPrograma, mensajeArchivo, sizeof(int) + i, archivo);

	free(stringArchivo);
}
