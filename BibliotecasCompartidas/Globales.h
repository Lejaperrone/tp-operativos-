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

sem_t pedidoFS;

typedef struct string{
	int longitud;
	char* cadena;
}string;

typedef enum {TRANSFORMACION, RED_LOCAL, RED_GLOBAL, ALM_FINAL}Etapa;

typedef enum {EN_EJECUCION, ERROR, FINALIZADO_OK}Estado;

typedef struct respuestaTransformacion{
	int nodo;
	char* ip;
	int puerto;
	int bloque;
	int byteOcupado;
	char* rutaArchivoTemp;
}respuestaTransformacion;

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

typedef struct solicitudTransformacion{
	string rutaDatos;
	string rutaResultado;
}solicitudTransformacion;

typedef struct{
	int sizeNodo;
	int bloquesOcupados;
	int numeroNodo;
	int socket;
} informacionNodo;

typedef struct{
	//YAMA recibe la lista de bloques que componen al archivo
	//con la ubicacion de sus dos copias y espacio ocupado en cada bloque
}informacionArchivoFsYama;
#endif /* GLOBALES_H_ */
