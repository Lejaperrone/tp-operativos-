/*
 * Configuracion.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <commons/config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct configuracionYama{
	int FS_PUERTO;
	char* FS_IP;
	int RETARDO_PLANIFICACION;
	char* ALGORITMO_BALANCEO;
}configuracionYama;

struct configuracionNodo{
	int PUERTO_FILESYSTEM;
	char* IP_FILESYSTEM;
	int PUERTO_WORKER;
	char* NOMBRE_NODO;
	int PUERTO_DATANODE;
	char* RUTA_DATABIN;
}configuracionNodo;

struct configuracionMaster{
	int YAMA_PUERTO;
	char* YAMA_IP;
}configuracionMaster;

#endif /* CONFIGURACION_H_ */
