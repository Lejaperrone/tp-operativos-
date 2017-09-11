/*
 * Comandos.h
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */

#ifndef FILESYSTEM_COMANDOS_H_
#define FILESYSTEM_COMANDOS_H_

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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool validarArchivo(char* path);

bool validarDirectorio(char* path);

int eliminarArchivo(char* comando, int longitudKey);//rm

int eliminarDirectorio(char* comando, int longitudKey);//rm -d

void listarArchivos(char* comando, int longitudKey);//ls

int crearDirectorio(char* comando, int longitudKey);//mkdir

int mostrarArchivo(char* comando, int longitudKey);//cat

int cambiarNombre(char* comando, int longitudKey);//rename

int mover(char* comando, int longitudKey);//mv

int informacion(char* comando, int longitudKey);//info

#endif /* FILESYSTEM_COMANDOS_H_ */
