/*
 * Globales.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */
#include "commons/string.h"
#include "commons/config.h"
#include "Globales.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

int redondearHaciaArriba(double num){
	 int inum = (int)num;

	 if (num == (float)inum) {
		 return inum;
	 }
	 return inum + 1;
}

void obtenerNumeroNodo(t_config* archivo,char* claveCopia,ubicacionBloque* ubi){
	char* copia = config_get_string_value(archivo,claveCopia);
	char* numeroNodo = calloc(1,3);
	char* numeroBloque= calloc(1,3);
	int i =5;

	while(copia[i]== ','){
		snprintf(numeroNodo,string_length(numeroNodo),"%s%c",numeroNodo,copia[i]);
		i++;
	}
	i++;
	while(copia[i]==']'){
		snprintf(numeroBloque,string_length(numeroBloque),"%s%c",numeroBloque,copia[i]);
		i++;
	}

	printf("%s %s\n",numeroBloque,numeroNodo);

	ubi->numeroBloque = atoi(numeroBloque);
	ubi->numeroNodo = atoi(numeroNodo);

	printf("%d %d\n",ubi->numeroBloque,ubi->numeroNodo);
}

void limpiarPantalla(){
	system("clear");
}
