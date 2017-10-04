/*
 * Planificador.h
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "FuncionesYama.h"

//#define cargaMaxima 10;

typedef struct{
	int idJob;
	char* pathOrigen;
	char* pathResultado;
}infoJob;

typedef struct {
	char* nombre;
	char* ip;
	char* puerto;
	uint32_t carga;
	int bloque;
}infoNodo;

void planificar(infoJob* job, infoNodo* nodo);
uint32_t calcularCarga(infoNodo* worker);
uint32_t workLoadGlobal();
int calcularDisponibilidadWorker(infoNodo* worker);
void agregarNodo(t_list* cargaNodo,infoNodo* nodo);
void agregarJobAPlanificar(infoJob* jobAPlanificar);
infoNodo* obtenerNodoDisponible(t_list* cargaNodos, t_list* listaNodosParaMap);
t_list* obtenerNodosQueEstanEnLista(t_list* cargaNodos, t_list* listaNodos);
infoNodo* obtenerNodoConNombre(char *nombreNodo);
bool nodoConMenorCargaPrimero(void* argNodo1, void* argNodo2);
uint32_t cargaMaxima();
void agregarNodos(t_list* cargaNodos, t_list* listaNodos);
void iniciarListasPlanificacion();
t_list* consultarDetallesBloqueArchivo(char *pathArchivo, int bloque);
void calcularCargasDeWorkers();

#endif /* PLANIFICADOR_H_ */
