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
#include <sys/mman.h>
#include <ctype.h>

char* devolverRuta(char* comando, int cantidadDeComandos);

bool validarDirectorio(char* path);

bool isDirectoryEmpty(char *dirname);

int eliminarArchivo(char* comando);//rm

int eliminarDirectorio(char* comando);//rm -d

int eliminarBloque(char* comando);//rm -b

int listarArchivos(char* comando);//ls

int crearDirectorio(char* comando);//mkdir

int mostrarArchivo(char* comando);//cat

int cambiarNombre(char* comando);//rename

int mover(char* comando);//mv

int informacion(char* comando);//info

int copiarArchivo(char* comando);//cpfrom

int copiarArchivoAFs(char* comando);//cpto

int generarArchivoMD5(char* comando);//md5

int formatearFS(char* comando);//format

#endif /* FILESYSTEM_COMANDOS_H_ */
