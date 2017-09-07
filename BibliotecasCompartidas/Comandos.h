/*
 * Comandos.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef COMANDOS_H_
#define COMANDOS_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "Sockets.h"
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

bool validarArchivo(char* path);

int eliminarArchivo(char* comando, int longitudKey);

void listarArchivos(char* comando, int longitudKey);

int crearDirectorio(char* comando, int longitudKey);

int mostrarArchivo(char* comando, int longitudKey);


#endif /* COMANDOS_H_ */
