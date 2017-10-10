/*
 * FuncionesFS.h
 *
 *  Created on: 11/9/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <commons/log.h>
#include "Globales.h"
#include <commons/collections/list.h>
#include <semaphore.h>
#include <pthread.h>
#include "Comandos.h"

typedef struct{
	int index;
	char nombre[255];
	int padre;
} t_directory;

typedef struct parametrosEnvioBloque {
    int socket;
    char* mapa;
    int sizeBloque;
    int offset;
    int bloque;
}parametrosEnvioBloque;

void* consolaFS();

void inicializarTablaDirectorios();

void guardarTablaDirectorios();

char* buscarRutaArchivo(char* ruta);

int getIndexDirectorio(char* ruta);

char* generarArrayNodos();

int levantarBitmapNodo(int numeroNodo, int sizeNodo);

int buscarPrimerBloqueLibre(int numeroNodo, int sizeNodo);

void actualizarArchivoNodos();

int nodoRepetido(informacionNodo info);

void atenderSolicitudYama(int socketYama, void* envio);

char* generarArrayBloque(int numeroNodo, int numeroBloque);

void guardarEnNodos(char* path, char* nombre, char* tipo, string* mapeoArchivo);

void setearBloqueOcupadoEnBitmap(int numeroNodo, int bloqueLibre);

void actualizarBitmapNodo(int numeroNodo);

void* enviarADataNode(void* parametros);
