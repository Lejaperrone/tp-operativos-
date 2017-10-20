/*
 * FuncionesYama.h
 *
 *  Created on: 24/9/2017
 *      Author: utnso
 */

#ifndef FUNCIONESYAMA_H_
#define FUNCIONESYAMA_H_

#include "Sockets.h"
#include "Configuracion.h"
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
#include <commons/log.h>
#include "Serializacion.h"
#include <Configuracion.h>
#include "Globales.h"
#include <commons/collections/list.h>
#include "Planificador.h"

#define idMaster 2
#define idDataNodes 3

t_list* nodosConectados;
pthread_mutex_t mutex_NodosConectados;
pthread_mutex_t mutexLog;

pthread_mutex_t cantJobs_mutex;

int servidor,socketFs;

struct sockaddr_in direccionCliente;

t_log* logger;
struct configuracionYama config;

uint32_t cantJobs;

int conectarseConFs();

void *manejarConexionMaster(void *cliente);

void levantarServidorYama(char* ip, int port);

void recibirContenidoMaster();

informacionArchivoFsYama* solicitarInformacionAFS(solicitudInfoNodos* solicitud);

int getDisponibilidadBase();

char* obtenerNombreArchivoResultadoTemporal();

int esClock();

void inicializarEstructuras();

bool** llenarMatrizNodosBloques(informacionArchivoFsYama* infoArchivo,int nodos,int bloques);

void calcularNodosYBloques(informacionArchivoFsYama* info,int* nodos);

void llenarListaNodos(t_list* listaNodos,informacionArchivoFsYama* infoArchivo);

void agregarBloqueANodo(t_list* listaNodos,ubicacionBloque ubicacion,int bloque);

#endif /* FUNCIONESYAMA_H_ */
