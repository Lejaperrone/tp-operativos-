/*
 * FuncionesDN.h
 *
 *  Created on: 12/9/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>

void conectarseConFs();

int levantarBitmap(char* nombreNodo);

void escucharAlFS(int socketFs);

int setBloque(int numeroBloque, char* datos);
