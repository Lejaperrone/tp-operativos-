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
#include "Sockets.h"
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "Comandos.h"
#include <Configuracion.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "FuncionesFS.h"
#include <pthread.h>

#define cantDataNodes 10

int cantBloques = 10;
int sizeBloque = 1048576; // 1mb
int mostrarLoggerPorPantalla = 1;
t_directory tablaDeDirectorios[100];
char* rutaArchivos = "metadata/Archivos/";
int cantidadDirectorios = 100;
//t_bitarray* bitmap[cantDataNodes];
t_log* loggerFS;
int sizeTotalNodos = 0, nodosLibres = 0;

int main(void) {

	int clienteYama = 0;
	int servidorFS = crearSocket();
	pthread_t hiloServidorFS, hiloConsolaFS;
	parametrosServidorHilo parametrosServidorFS;

	parametrosServidorFS.cliente = clienteYama;
	parametrosServidorFS.servidor = servidorFS;

	//inicializarBitmaps();

	/*tablaDeDirectorios[0].index = 0;
	tablaDeDirectorios[1].index = 1;
	tablaDeDirectorios[2].index = 2;		//para probar cosas
	tablaDeDirectorios[3].index = 3;
	tablaDeDirectorios[0].padre = -1;
	tablaDeDirectorios[1].padre = 0;
	tablaDeDirectorios[2].padre = 1;
	tablaDeDirectorios[3].padre = 0;
	memcpy(tablaDeDirectorios[0].nombre,"hola",5);
	memcpy(tablaDeDirectorios[1].nombre,"chau",5);
	memcpy(tablaDeDirectorios[2].nombre,"bla",4);
	memcpy(tablaDeDirectorios[3].nombre,"bla",4);*/

	loggerFS = log_create("logFileSystem", "FileSystem.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	inicializarTablaDirectorios();
	//printf("\n\n %s", tablaDeDirectorios[0].nombre);

	pthread_create(&hiloServidorFS,NULL,levantarServidorFS ,(void*)&parametrosServidorFS);
	pthread_create(&hiloConsolaFS,NULL,consolaFS ,NULL);
	//levantarServidorFS(servidorFS, clienteYama);
	pthread_join(hiloServidorFS, NULL);
	pthread_join(hiloConsolaFS, NULL);


}

