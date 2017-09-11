/*
 * Globales.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <stdbool.h>

typedef struct respuestaTransformacion{
	int nodo;
	char* ip;
	int puerto;
	int bloque;
	int byteOcupado;
	char* rutaArchivoTemp;
};

typedef struct respuestaReduccionLocal{
	int nodo;
	char* ip;
	int puerto;
	char* archivoTransformacion;
	char* archivoReduccion;
};

typedef struct respuestaReduccionGlobal{
	int nodo;
	char* ip;
	int puerto;
	char* archivoReduccionLocal;
	char* archivoReduccionGlobal;
	bool encargado;
};

typedef struct registroTablaEstados{
	int job;
	int master;
	int nodo;
	int bloque;
	int estapa;
	char* rutaArchivoTemp;
	int estado;
};

#endif /* GLOBALES_H_ */
