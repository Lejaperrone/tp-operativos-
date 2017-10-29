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
#include <math.h>

int redondearHaciaArriba(int num,int num2){
	if(num%num2 ==0){
		return (int)num/num2;
	}
	else{
		return (int)(num/num2)+1;
	}
}

bool validarArchivo(char* path) {
	if (access(path, R_OK) == -1) {
		printf("No existe el archivo %s en el FileSystem\n", path);
		return 0;
	} else {
		printf("Existe el archivo %s en el FileSystem\n", path);
		return 1;
	}
}

void obtenerNumeroNodo(t_config* archivo,char* claveCopia,ubicacionBloque* ubi){
	char* copia = config_get_string_value(archivo,claveCopia);
	char* numeroNodo = calloc(1,3);
	char* numeroBloque= calloc(1,3);
	int i =5;

	while(copia[i]!= ','){
		char cop[2];
		cop[0] = copia[i];
		cop[1] = '\0';
		string_append(&numeroNodo,cop);
		i++;
	}
	i++;
	while(copia[i]!=']'){
		char cop[2];
		cop[0] = copia[i];
		cop[1] = '\0';
		string_append(&numeroBloque,cop);
		i++;
	}

	ubi->numeroBloqueEnNodo = atoi(numeroBloque);
	ubi->numeroNodo = atoi(numeroNodo);
}

void limpiarPantalla(){
	system("clear");
}
