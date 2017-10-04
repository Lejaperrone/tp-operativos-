/*
 * Planificador.h
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "FuncionesYama.h"

//#define cargaMaxima 10;

typedef struct{
	int idJob;
	char* pathOrigen;
	char* pathResultado;
}infoJob;

typedef struct {
	char* nombre;
	char* ip;
	char* puerto;
	uint32_t carga;
	int bloque;
}infoNodo;

void planificar(infoJob* job, infoNodo nodo);

#endif /* PLANIFICADOR_H_ */
