/*
 * Serializacion.c
 *
 *  Created on: 7/9/2017
 *      Author: utnso
 */
#include "Serializacion.h"

void empaquetar(int socket, int idMensaje,int tamanioS, void* paquete){
	header cabecera;
	cabecera.idMensaje = idMensaje;
	int tamanio;
	void* bloque;

	switch(idMensaje){
		case mensajeHandshake:
			tamanio = sizeof(int);
			bloque = malloc(sizeof(int));
			memcpy(bloque,paquete,sizeof(int));
			break;

		case mensajeArchivo:
			serializarString(paquete,&tamanio);
			break;
	}

	cabecera.tamanio = tamanio;
	int desplazamiento =0;
	int tamanioTotal = 2* sizeof(int) + tamanio;
    void* buffer = malloc(tamanioTotal);

	memcpy(buffer , &cabecera, sizeof(header));
	desplazamiento += sizeof(header);
	memcpy(buffer + desplazamiento, bloque, tamanio);
	send(socket,buffer,tamanioTotal,0);
	free(bloque);
	free(buffer);

}

respuesta desempaquetar(int socket){
	void* bufferOk;
	respuesta miRespuesta;
	header *cabecera = malloc(sizeof(header));
	int bytesRecibidos;

	if ((bytesRecibidos = recv(socket, cabecera, sizeof(header), 0)) == 0) {
		miRespuesta.idMensaje = -1;
	}

	else {
		miRespuesta.idMensaje = cabecera->idMensaje;

		switch (miRespuesta.idMensaje) {

		case mensajeHandshake:
			bufferOk = malloc(sizeof(int));
			recv(socket, bufferOk, sizeof(int), 0);
			miRespuesta.envio = malloc(sizeof(int));
			memcpy(miRespuesta.envio, bufferOk, sizeof(int));
			free(bufferOk);
			break;

		case mensajeArchivo:
			deserializarString(socket,cabecera->tamanio);
		}


	}
	return miRespuesta;
}
//------SERIALIZACIONES PARTICULARES------//
void* serializarString(void* paquete,int* tamanio){
	string* cadena = (string*)paquete;
	int longitudInt = sizeof(int);
	*tamanio = sizeof(int)+cadena->longitud+1;
	void* bloque = malloc(*tamanio);
	int desplazamiento =0;

	memcpy(bloque, &cadena->longitud, longitudInt);
	desplazamiento += longitudInt;

	memcpy(bloque+desplazamiento, cadena->cadena, cadena->longitud+1);

	return bloque;
}

string* deserializarString(int socket,int tamanio){
	void* paquete = malloc(tamanio+1);
	recv(socket,paquete,tamanio+1,0);
	int longitudInt = sizeof(int);
	string* cadena = malloc(sizeof(string));
	int desplazamiento =0;

	memcpy(&cadena->longitud,paquete, longitudInt);
	desplazamiento += longitudInt;

	cadena->cadena = malloc(cadena->longitud+1);

	memcpy(cadena->cadena,paquete+desplazamiento, cadena->longitud+1);

	return cadena;
}
