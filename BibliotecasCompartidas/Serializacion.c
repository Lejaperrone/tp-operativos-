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

		case mensajeInfoArchivo:
		case mensajeProcesarTransformacion:
		case mensajeDesignarWorker:

		case mensajeOk:
			tamanio =1;
			bloque = malloc(1);
			char a = 'a';
			memcpy(bloque,&a,1);
			break;

		case mensajeError:
			tamanio =1;
			bloque = malloc(1);
			char b = 'b';
			memcpy(bloque,&b,1);
			break;

		case mensajeArchivo:
			bloque = serializarString(paquete,&tamanio);
			break;

		case mensajeSolicitudTransformacion:
			bloque = serializarJob(paquete, &tamanio);
			break;


		case mensajeInformacionNodo:
			bloque = serializarInformacionNodos(paquete, &tamanio);
			break;

		case mensajeEnvioBloqueANodo:

		case mensajeRespuestaGetBloque:
			tamanio = tamanioS;
			bloque = malloc(tamanio);
			printf("size envio %d\n", tamanio);
			memcpy(bloque,paquete,tamanio);
			break;

		case mensajeRespuestaEnvioBloqueANodo:
		case mensajeNumeroCopiaBloqueANodo:

		case mensajeNumeroLecturaBloqueANodo:
			tamanio = sizeof(int);
			bloque = malloc(tamanio);
			memcpy(bloque,paquete,tamanio);
			break;

		case mensajeSolicitudInfoNodos:
			bloque = serializarSolicitudInfoNodos(paquete,&tamanio);
			break;

		case mensajeRespuestaInfoNodos:
			bloque = serializarRespuestaInfoNodos(paquete,&tamanio);
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
	header* cabecera = malloc(sizeof(header));
	int bytesRecibidos;

	if ((bytesRecibidos = recv(socket, cabecera, sizeof(header), 0)) == 0) {
		miRespuesta.idMensaje = -1;
	}
	else {
		miRespuesta.size = cabecera->tamanio;
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
				miRespuesta.envio = deserializarString(socket,cabecera->tamanio);
				break;

			case mensajeInfoArchivo://todo
			case mensajeProcesarTransformacion://todo
			case mensajeDesignarWorker://todo
			case mensajeOk:
				bufferOk = malloc(sizeof(char));
				recv(socket,bufferOk,sizeof(char),0);
				free(bufferOk);
				break;

			case mensajeError:
				bufferOk = malloc(sizeof(char));
				recv(socket,bufferOk,sizeof(char),0);
				free(bufferOk);
				break;

			case mensajeSolicitudTransformacion:
				miRespuesta.envio = deserializarJob(socket, cabecera->tamanio);//FIXME
				break;

			case mensajeInformacionNodo:
				miRespuesta.envio = deserializarInformacionNodos(socket, cabecera->tamanio);
				break;

			case mensajeRespuestaEnvioBloqueANodo:
			case mensajeNumeroCopiaBloqueANodo:
			case mensajeNumeroLecturaBloqueANodo:
				bufferOk = malloc(sizeof(int));
				recv(socket,bufferOk,sizeof(int),MSG_WAITALL);
				miRespuesta.envio = malloc(sizeof(int));
				memcpy(miRespuesta.envio, bufferOk, sizeof(int));
				free(bufferOk);
				break;

			case mensajeEnvioBloqueANodo:
			case mensajeRespuestaGetBloque:
				bufferOk = malloc(cabecera->tamanio);
				printf("espero %d\n", cabecera->tamanio);
				recv(socket,bufferOk,cabecera->tamanio,MSG_WAITALL);
				miRespuesta.envio = malloc(cabecera->tamanio);
				memcpy(miRespuesta.envio, bufferOk, cabecera->tamanio);
				free(bufferOk);
				break;

			case mensajeSolicitudInfoNodos:
				miRespuesta.envio = deserializarSolicitudInfoNodos(socket,cabecera->tamanio);
				break;

			case mensajeRespuestaInfoNodos:
				miRespuesta.envio = deserializarRespuestaInfoNodos(socket,cabecera->tamanio);
				break;


		}
	}
	return miRespuesta;
}
//------SERIALIZACIONES PARTICULARES------//
void* serializarString(void* paquete,int *tamanio){
 	string* cadena = (string*)paquete;
 	int longitudInt = sizeof(int);
 	*tamanio = sizeof(int)+strlen(cadena->cadena)+1;
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

 	return cadena;
 }

void* serializarJob(void* paquete, int* tamanio){
	job* unJob = (job*)paquete;
	int desplazamiento = 0;

	*tamanio = sizeof(job) -(4*sizeof(int)) + unJob->rutaDatos.longitud + unJob->rutaResultado.longitud
			+ unJob->rutaTransformador.longitud + unJob->rutaReductor.longitud + 4;
	void* buffer = malloc(*tamanio);

	memcpy(buffer + desplazamiento, &(unJob->id), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(unJob->socketFd), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(unJob->rutaDatos.longitud), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unJob->rutaDatos.cadena, unJob->rutaDatos.longitud+1);
	desplazamiento += unJob->rutaDatos.longitud;

	memcpy(buffer + desplazamiento, &(unJob->rutaResultado.longitud), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unJob->rutaResultado.cadena, unJob->rutaResultado.longitud+1);
	desplazamiento += unJob->rutaResultado.longitud;

	memcpy(buffer + desplazamiento, &(unJob->rutaTransformador.longitud), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unJob->rutaTransformador.cadena, unJob->rutaTransformador.longitud+1);
	desplazamiento += unJob->rutaTransformador.longitud;

	memcpy(buffer + desplazamiento, &(unJob->rutaReductor.longitud), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unJob->rutaReductor.cadena, unJob->rutaReductor.longitud+1);


	return buffer;
}

job* deserializarJob(int socket, int tamanio){
	int desplazamiento = 0;
	job* unJob = malloc(sizeof(job));

	void* buffer = malloc(tamanio);
	recv(socket,buffer,tamanio,0);

	memcpy(&unJob->id, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&unJob->socketFd, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&unJob->rutaDatos.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	unJob->rutaDatos.cadena = calloc(1,unJob->rutaDatos.longitud+1);
	memcpy(unJob->rutaDatos.cadena, buffer + desplazamiento, unJob->rutaDatos.longitud);
	desplazamiento += unJob->rutaDatos.longitud;

	memcpy(&unJob->rutaResultado.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	unJob->rutaResultado.cadena = calloc(1,unJob->rutaResultado.longitud+1);
	memcpy(unJob->rutaResultado.cadena, buffer + desplazamiento, unJob->rutaResultado.longitud);
	desplazamiento += unJob->rutaResultado.longitud;

	memcpy(&unJob->rutaTransformador.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	unJob->rutaTransformador.cadena = calloc(1,unJob->rutaTransformador.longitud+1);
	memcpy(unJob->rutaTransformador.cadena, buffer + desplazamiento, unJob->rutaTransformador.longitud);
	desplazamiento += unJob->rutaTransformador.longitud;

	memcpy(&unJob->rutaReductor.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	unJob->rutaReductor.cadena = calloc(1,unJob->rutaReductor.longitud+1);
	memcpy(unJob->rutaReductor.cadena, buffer + desplazamiento, unJob->rutaReductor.longitud);

	return unJob;
}

void* serializarSolicitudInfoNodos(void* paquete,int* tamanio){

	solicitudInfoNodos* unaSolicitud = (solicitudInfoNodos*)paquete;
	int desplazamiento = 0;

	*tamanio = 2+ sizeof(solicitudInfoNodos) - (2* sizeof(int)) + unaSolicitud->rutaDatos.longitud + unaSolicitud->rutaResultado.longitud;
	void * buffer = malloc(*tamanio);

	memcpy(buffer + desplazamiento, &(unaSolicitud->rutaDatos.longitud), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unaSolicitud->rutaDatos.cadena, unaSolicitud->rutaDatos.longitud+1);
	desplazamiento += unaSolicitud->rutaDatos.longitud;

	memcpy(buffer + desplazamiento, &(unaSolicitud->rutaResultado.longitud), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unaSolicitud->rutaResultado.cadena, unaSolicitud->rutaResultado.longitud+1);
	desplazamiento += unaSolicitud->rutaResultado.longitud;

	return buffer;
}

solicitudInfoNodos* deserializarSolicitudInfoNodos(int socket,int tamanio){
	int desplazamiento = 0;
	solicitudInfoNodos* unaSolicitud = malloc(sizeof(solicitudInfoNodos));

	void* buffer = malloc(tamanio);
	recv(socket,buffer,tamanio,0);

	memcpy(&unaSolicitud->rutaDatos.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	unaSolicitud->rutaDatos.cadena = calloc(1,unaSolicitud->rutaDatos.longitud+1);
	memcpy(unaSolicitud->rutaDatos.cadena, buffer + desplazamiento, unaSolicitud->rutaDatos.longitud);
	desplazamiento += unaSolicitud->rutaDatos.longitud;

	memcpy(&unaSolicitud->rutaResultado.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	unaSolicitud->rutaResultado.cadena = calloc(1,unaSolicitud->rutaResultado.longitud+1);
	memcpy(unaSolicitud->rutaResultado.cadena, buffer + desplazamiento, unaSolicitud->rutaResultado.longitud);
	desplazamiento += unaSolicitud->rutaResultado.longitud;

	return unaSolicitud;
}

void* serializarInformacionNodos(void* paquete,int* tamanio){
	informacionNodo* info= (informacionNodo*)paquete;
	int desplazamiento = 0;

	*tamanio = sizeof(informacionNodo)-sizeof(int) +info->ip.longitud;
	void * buffer = malloc(*tamanio);

	memcpy(buffer + desplazamiento, &(info->bloquesOcupados), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(info->numeroNodo), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(info->puerto), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(info->sizeNodo), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(info->socket), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, &(info->ip.longitud), sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer + desplazamiento, info->ip.cadena, info->ip.longitud+1);
	desplazamiento += info->ip.longitud;

	return buffer;
}

informacionNodo* deserializarInformacionNodos(int socket,int tamanio){
	int desplazamiento = 0;
	informacionNodo* info= malloc(sizeof(informacionNodo));

	void* buffer = malloc(tamanio);
	recv(socket,buffer,tamanio,0);

	memcpy(&info->bloquesOcupados, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	memcpy(&info->numeroNodo, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	memcpy(&info->puerto, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	memcpy(&info->sizeNodo, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	memcpy(&info->socket, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	memcpy(&info->ip.longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	info->ip.cadena = calloc(1,info->ip.longitud+1);
	memcpy(info->ip.cadena, buffer + desplazamiento, info->ip.longitud);
	desplazamiento += info->ip.longitud;

	return info;
}

void* serializarRespuestaInfoNodos(void* paquete,int* tamanio){
	informacionArchivoFsYama* respuesta = (informacionArchivoFsYama*)paquete;
	int desplazamiento = 0;

	*tamanio = sizeof(int);
	void * buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento, &(respuesta->tamanioTotal), sizeof(int));
	desplazamiento += sizeof(int);

	*tamanio += sizeof(int);
	buffer = realloc(buffer, *tamanio);
	int longitud = list_size(respuesta->informacionBloques);
	memcpy(buffer + desplazamiento, &longitud, sizeof(int));
	desplazamiento += sizeof(int);

	int j;
	for (j = 0; j < list_size(respuesta->informacionBloques); ++j) {
		infoBloque* infoBloq = (infoBloque*)list_get(respuesta->informacionBloques, j);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->bytesOcupados, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->numeroBloque, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia0.numeroBloqueEnNodo, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia0.numeroNodo, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia0.puerto, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia0.ip.longitud, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += infoBloq->ubicacionCopia0.ip.longitud;
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, infoBloq->ubicacionCopia0.ip.cadena, infoBloq->ubicacionCopia0.ip.longitud);
		desplazamiento += infoBloq->ubicacionCopia0.ip.longitud;

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia1.numeroBloqueEnNodo, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia1.numeroNodo, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia1.puerto, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += sizeof(int);
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, &infoBloq->ubicacionCopia1.ip.longitud, sizeof(int));
		desplazamiento += sizeof(int);

		*tamanio += infoBloq->ubicacionCopia1.ip.longitud;
		buffer = realloc(buffer, *tamanio);
		memcpy(buffer + desplazamiento, infoBloq->ubicacionCopia1.ip.cadena, infoBloq->ubicacionCopia1.ip.longitud);
		desplazamiento += infoBloq->ubicacionCopia1.ip.longitud;

	}

	return buffer;
}

informacionArchivoFsYama* deserializarRespuestaInfoNodos(int socket,int tamanio){
	int desplazamiento = 0;
	informacionArchivoFsYama* info= malloc(sizeof(informacionArchivoFsYama));

	void* buffer = malloc(tamanio);
	recv(socket,buffer,tamanio,0);

	memcpy(&info->tamanioTotal, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	info->informacionBloques= list_create();
	int longitud = 0;
	memcpy(&longitud, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	int j;
	for (j = 0; j < longitud; ++j) {
		infoBloque* infoBloq = malloc(sizeof(infoBloque));

		memcpy(&infoBloq->bytesOcupados, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->numeroBloque, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia0.numeroBloqueEnNodo, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia0.numeroNodo, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia0.puerto, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia0.ip.longitud, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		infoBloq->ubicacionCopia0.ip.cadena = calloc(1,infoBloq->ubicacionCopia0.ip.longitud+1);
		memcpy(infoBloq->ubicacionCopia0.ip.cadena, buffer + desplazamiento, infoBloq->ubicacionCopia0.ip.longitud);
		desplazamiento += infoBloq->ubicacionCopia0.ip.longitud;

		memcpy(&infoBloq->ubicacionCopia1.numeroBloqueEnNodo, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia1.numeroNodo, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia1.puerto, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&infoBloq->ubicacionCopia1.ip.longitud, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		infoBloq->ubicacionCopia1.ip.cadena = calloc(1,infoBloq->ubicacionCopia1.ip.longitud+1);
		memcpy(infoBloq->ubicacionCopia1.ip.cadena, buffer + desplazamiento, infoBloq->ubicacionCopia1.ip.longitud);
		desplazamiento += infoBloq->ubicacionCopia1.ip.longitud;

		list_add(info->informacionBloques, infoBloq);
	}

	return info;
}
