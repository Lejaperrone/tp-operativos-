#ifndef FUNCIONESMASTER_H_
#define FUNCIONESMASTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <Configuracion.h>
#include <commons/string.h>
#include <Globales.h>
#include <Serializacion.h>
#include <pthread.h>
#include <commons/log.h>

#define mensajeArchivo 2
#define mensajeSolicitudTransformacion 4

typedef struct{
	void* scriptTransformacion;//FIXMe PUEDE CAMBIAR POR EL TEMA DE MMAP
	int bloque;
	int bytesOcupados;
	string archivoTemporal;
}procesarTransformacion;

/*----VARIABLES GLOBALES----*/
t_log* loggerMaster;
int socketYama;
estadisticaProceso* estadisticas;
job* miJob;
bool hayReplanificacion;
bool esperoReplanificacion;
pthread_mutex_t mutexReplanificar;
/*--------------------------*/


void conectarseConYama(char* ip, int port);

void* conectarseConWorkers(void* parametros);

void enviarJobAYama(job* job);

void esperarInstruccionesDeYama();

char* recibirRuta(char* mensaje);

string* contenidoArchivo(char* rutaArchivo);

job* crearJob(char* argv[]);

estadisticaProceso* crearEstadisticasProceso();

void setearTiempo(t_list* tiempos);

void finalizarJob();

void crearHilosConexionTransformacion(respuestaSolicitudTransformacion* rtaYama);

void crearHilosPorBloqueTransformacion(workerDesdeYama* worker);

void esperarReplanificaciones();

double calcularDuracionPromedio(t_list* tiemposInicio,t_list* tiemposFin,int etapa);


#endif /* FUNCIONESMASTER_H_ */
