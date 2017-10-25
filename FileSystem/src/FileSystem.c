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

#include "FuncionesHilosFs.h"

#define cantDataNodes 10

int cantBloques = 10;
int sizeBloque = 1048576; // 1mb
int mostrarLoggerPorPantalla = 1;

char* rutaBitmaps = "../metadata/Bitmaps/";
char* rutaArchivos = "../metadata/Archivos/";

int cantidadDirectorios = 100;
int numeroCopiasBloque = 2;
//t_bitarray* bitmap[cantDataNodes];
t_log* loggerFS;
int sizeTotalNodos = 0, nodosLibres = 0;
t_list* nodosConectados;
t_list* bitmapsNodos;
extern sem_t pedidoFS;
sem_t pedidoLecturaFS[];
extern sem_t actualizarNodos;
int clienteYama;
int servidorFS;

int main(void) {
	limpiarPantalla();
	sem_init(&pedidoFS,0,0);
	sem_init(&actualizarNodos,1,0);
	nodosConectados = list_create();
	bitmapsNodos = list_create();

	pthread_t hiloConsolaFS;

	//inicializarBitmaps();

	loggerFS = log_create("logFileSystem", "FileSystem.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	inicializarTablaDirectorios();

	servidorFS = crearSocket();

	establecerServidor(servidorFS);

	levantarServidorFS();

	pthread_create(&hiloConsolaFS,NULL,consolaFS ,NULL);
	pthread_join(hiloConsolaFS, NULL);

	return 0;

}

