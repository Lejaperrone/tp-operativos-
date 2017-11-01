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
sem_t pedidoFS;
t_list* pedidosFS;
extern sem_t actualizarNodos;
int clienteYama;
int servidorFS;
pthread_mutex_t logger_mutex;

int main(void) {
	limpiarPantalla();
	sem_init(&pedidoFS,0,0);
	sem_init(&actualizarNodos,1,0);
	nodosConectados = list_create();
	bitmapsNodos = list_create();
	pedidosFS = list_create();

	pthread_mutex_init(&logger_mutex, NULL);
	pthread_t hiloServidorFS, hiloConsolaFS, hiloConexionYama;
	parametrosServidorHilo parametrosServidorFS;
	loggerFS = log_create("logFileSystem", "FileSystem.c", mostrarLoggerPorPantalla, LOG_LEVEL_TRACE);

	parametrosServidorFS.cliente = clienteYama;


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


	inicializarTablaDirectorios();

	servidorFS = crearSocket();

	establecerServidor(servidorFS);

	clienteYama = recibirConexionYama(servidorFS);

	parametrosServidorFS.servidor = servidorFS;

	pthread_create(&hiloServidorFS,NULL,levantarServidorFS ,(void*)&parametrosServidorFS);
	pthread_create(&hiloConexionYama,NULL,manejarConexionYama ,NULL);
	pthread_create(&hiloConsolaFS,NULL,consolaFS ,NULL);

	pthread_join(hiloServidorFS, NULL);
	pthread_join(hiloConexionYama, NULL);
	pthread_join(hiloConsolaFS, NULL);

	return 0;

}

