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

typedef struct{
	int index;
	char nombre[255];
	int padre;
} t_directory;

int maxDatanodes;
int nuevoDataNode;

void* consolaFS();

char* buscarRutaArchivo(char* ruta);

int getIndexDirectorio(char* ruta);
