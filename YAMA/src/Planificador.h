/*
 * Planificador.h
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "FuncionesYama.h"
#include "../BibliotecasCompartidas/Globales.h"
#include <pthread.h>
#include <semaphore.h>
#include <../commons/string.h>

t_list* jobsAPlanificar;

typedef struct {
	int numero;
	string ip;
	int puerto;
	uint32_t carga;
	t_list* bloques;
	bool activo;
	int cantTareasHistoricas;
	int disponibilidad;
}infoNodo;

pthread_mutex_t cantTareasHistoricas_mutex;
uint32_t wlMax;


void planificar(job* job);
infoNodo* inicializarWorker();
void iniciarListasPlanificacion();
void seleccionarWorker(infoNodo* worker, infoBloque bloque);
bool mayorDisponibilidad(infoNodo* worker, infoNodo* workerMasDisp);
infoNodo* buscarNodo(t_list* nodos, int numNodo);
uint32_t calcularPWL(infoNodo* worker);
uint32_t workLoadMaxima();
void calcularWorkLoadMaxima(t_list* nodos);
void calcularDisponibilidadWorkers(t_list* nodos);
void calcularDisponibilidadWorker(infoNodo* worker);
int obtenerDisponibilidadWorker(infoNodo* worker);
void agregarNodo(t_list* cargaNodo,infoNodo* nodo);
void agregarJobAPlanificar(job* jobAPlanificar);
uint32_t cargaMaxima();
void agregarNodos(t_list* cargaNodos, t_list* listaNodos);
void iniciarListasPlanificacion();
t_list* consultarDetallesBloqueArchivo(char *pathArchivo, int bloque);
void calcularCargasDeWorkers(t_list* listaNodos);
void bloqueEstaEnWorker(infoBloque* bloque, infoNodo* worker);
informacionArchivoFsYama* recibirInfoArchivo(job* job) ;
bool estaActivo(infoNodo* worker);
infoNodo* posicionarClock(t_list* listaWorkersConBloques);
bool bloqueEstaEn(infoNodo* nodo,bool** nodoXbloque, int bloque);
void moverClock(infoNodo* workerDesignado, t_list* listaNodos, bool** nodosPorBloque, informacionArchivoFsYama* infoArchivo);
void avanzarClock(infoNodo* worker, t_list* listaNodos);
infoNodo* encontrarWorkerDisponible(t_list* listaNodos);

#endif /* PLANIFICADOR_H_ */
