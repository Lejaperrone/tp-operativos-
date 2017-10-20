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
t_list* listaNodos;
pthread_mutex_t listaNodos_mutex;

typedef struct {
	char* nombre;
	char* ip;
	char* puerto;
	uint32_t carga;
	infoBloque bloque;
	int activo;//1 activo 0 no activo;
	int cantTareasHistoricas;
}infoNodo;



/*typedef struct {
	int numero;
	char* ip;
	char* puerto;
	uint32_t carga;
	int* bloques;
	bool activo;
}infoNodo;*/

pthread_mutex_t cantTareasHistoricas_mutex;


void planificar(job* job);
void seleccionarWorker(infoNodo* worker, infoBloque bloque);
bool mayorDisponibilidad(infoNodo* worker, infoNodo* workerMasDisp);
infoNodo* buscarNodo(t_list* nodos, char* nombreNodo);
uint32_t calcularPWL(infoNodo* worker);
uint32_t workLoadGlobal();
int calcularDisponibilidadWorker(infoNodo* worker);
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


#endif /* PLANIFICADOR_H_ */
