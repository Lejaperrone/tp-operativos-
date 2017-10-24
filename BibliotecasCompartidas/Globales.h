/*
 * Globales.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <stdbool.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/collections/list.h>

sem_t pedidoFS;
sem_t actualizarNodos;

typedef struct string{
	int longitud;
	char* cadena;
}string;

typedef struct job{
	int id;
	int socketFd;
	string rutaTransformador;
	string rutaReductor;
	string rutaDatos;
	string rutaResultado;
}job;

typedef enum {TRANSFORMACION, RED_LOCAL, RED_GLOBAL, ALM_FINAL}Etapa;

typedef enum {EN_EJECUCION, ERROR, FINALIZADO_OK}Estado;

typedef struct {
	int nodo;
	string ip;
	int puerto;
	t_list* bloques;
	int bytesOcupado;//FIXME
	string rutaArchivoTemp;
}respuestaSolicitudTransformacion;

typedef struct {
	int nodo;
	char* ip;
	int puerto;
	int bloque;
	int byteOcupado;
	char* rutaArchivoTemp;
}respuestaInfoNodos;

typedef struct respuestaReduccionLocal{
	int nodo;
	char* ip;
	int puerto;
	char* archivoTransformacion;
	char* archivoReduccion;
}respuestaReduccionLocal;

typedef struct respuestaReduccionGlobal{
	int nodo;
	char* ip;
	int puerto;
	char* archivoReduccionLocal;
	char* archivoReduccionGlobal;
	bool encargado;
}respuestaReduccionGlobal;

typedef struct registroTablaEstados{
	int job;
	int master;
	int nodo;
	int bloque;
	Etapa etapa;
	char* rutaArchivoTemp;
	Estado estado;
}registroTablaEstados;

typedef struct {
	string rutaDatos;
	string rutaResultado;
}solicitudInfoNodos;

typedef struct{
	int sizeNodo;
	int bloquesOcupados;
	int numeroNodo;
	int socket;
	int puerto;
	string ip;
} informacionNodo;

typedef struct{
	string ip;
	int puerto;
	int numeroBloqueEnNodo;
	int numeroNodo;
}ubicacionBloque;

typedef struct {
	int bytesOcupados;
	int numeroBloque;
	ubicacionBloque ubicacionCopia0;
	ubicacionBloque ubicacionCopia1;
}infoBloque;

typedef struct{
	int tamanioTotal;
	t_list* informacionBloques;
}informacionArchivoFsYama;

typedef struct{
	int index;
	char nombre[255];
	int padre;
} t_directory;

typedef struct parametrosEnvioBloque{
    int socket;
    char* mapa;
    int sizeBloque;
    int offset;
    int bloque;
    int restanteAnterior;
}parametrosEnvioBloque;

typedef struct parametrosLecturaBloque{
    int socket;
    int bloque;
    char* contenidoBloque;
}parametrosLecturaBloque;

int redondearHaciaArriba(double num);

bool validarArchivo(char* path);

void obtenerNumeroNodo(t_config* archivo,char* claveCopia,ubicacionBloque* ubi);

void limpiarPantalla();

#endif /* GLOBALES_H_ */
