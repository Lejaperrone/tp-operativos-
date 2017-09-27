/*
 * FuncionesMaster.c
 *
 *  Created on: 10/9/2017
 *      Author: utnso
 */

#include "FuncionesMaster.h"
#include <sys/mman.h>
#include <Globales.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void conectarseConYama(char* ip, int port) {
	socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketYama, 2); //2 id master
	log_trace(loggerMaster, "Conexion con Yama establecida"); //FIXME: FALTA CONTROL DE ERROR
}
void crearHilosConexion() {
	pthread_t hiloConexion;

	parametrosConexionMaster* parametrosConexion = malloc(sizeof(parametrosConexionMaster));
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
	procesarTransformacion solicitud;
	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(parametrosConexion->ip,parametrosConexion->port);
	conectarCon(direccion, socketWorker, parametrosConexion->id); //2 id master
	log_trace(loggerMaster, "Conexion con Worker \n");


	//empaquetar la solicitud de procesamiento
	return 0;
}

void enviarJobAYama() {
	solicitudTransformacion* nuevaSol = malloc(sizeof(solicitudTransformacion));

	nuevaSol->rutaDatos.cadena = string_duplicate(miJob->rutaDatos);
	nuevaSol->rutaDatos.longitud = string_length(miJob->rutaDatos);
	nuevaSol->rutaResultado.cadena = string_duplicate(miJob->rutaResultado);
	nuevaSol->rutaResultado.longitud = string_length(miJob->rutaResultado);

	empaquetar(socketYama,mensajeSolicitudTransformacion,0,nuevaSol);//
	log_trace(loggerMaster,"Enviando solicitud de etapa Transformacion a YAMA");

	respuesta respuestaYama = desempaquetar(socketYama);

	if(respuestaYama.idMensaje != mensajeOk){
		log_error(loggerMaster,"No se pudo iniciar correctamente el job");
		exit(1);
	}

	log_trace(loggerMaster, "Envio correcto de datos a Yama.");

}

void esperarInstruccionesDeYama() {
	respuesta instruccionesYama ;//= malloc(sizeof(respuesta));
	while (1) {
		instruccionesYama = desempaquetar(socketYama);

		switch (instruccionesYama.idMensaje) {

		case mensajeDesignarWorker:
			crearHilosConexion();
			break;
		}
	}
//recibir de yama todos los parametros conexion IP PUERTO
//ver que tipo de etapa es, los hilos de cada etapa son DISTINTOS!!!
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

job* crearJob(char* argv[]){
	job* nuevo = (job*)malloc(sizeof(job));
	nuevo->rutaTransformador= argv[2];
	nuevo->rutaReductor= argv[3];
	nuevo->rutaDatos= argv[4];
	nuevo->rutaResultado= argv[5];

	return nuevo;
}

int dameUnID(){
	return 1;//FIXME
}
void controlarParametros(int cantParams){
	if(cantParams < 6){
		log_error(loggerMaster, "Parametros insuficientes");
	}
}

void enviarArchivoo(int socketPrograma, char* pathArchivo){
	struct stat fileStat;
	if(stat(pathArchivo,&fileStat) < 0)
		exit(1);
	int fd = open(pathArchivo,O_RDWR);
	FILE* paquete = mmap(0,fileStat.st_size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	empaquetar(socketPrograma, mensajeArchivo, fileStat.st_size, (void*)paquete);
	close(fd);
	if (munmap(paquete,fileStat.st_size) == 0)
		printf("%s\n", "todo joya");
	else
		printf("%s\n", "todo no joya");


}
