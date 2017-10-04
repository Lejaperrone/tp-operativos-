/*
 * Planificador.c
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#include "Planificador.h"
t_list* listaNodos;
t_list* jobsAPlanificar;

void planificar(infoJob* job, infoNodo* nodo){
	//localizar los bloques en FS

	//calcular carga/score de cada nodo. Recomendable aplicar funcion de Availability

	//comparar cargas
}
int calcularDisponibilidadWorker(infoNodo* worker){
	if(esClock() == 0){
		return getDisponibilidadBase();
	}
	else{
		return getDisponibilidadBase() + calcularCarga(worker);
	}
}
uint32_t calcularCarga(infoNodo* worker){
	return workLoadGlobal() - worker->carga;
}
uint32_t workLoadGlobal(){
	return 0; //FIXME
}

t_list* nuevaCargaWorkers(){
	t_list* cargaWorkers = malloc(sizeof(t_list));
	cargaWorkers = list_create();
	return cargaWorkers;
}

void agregarJobAPlanificar(infoJob* jobAPlanificar){
	list_add(jobsAPlanificar, (void*)jobAPlanificar);
}

void agregarNodo(t_list* cargaNodos,infoNodo* nodo){
	int i;
	for(i = 0; i < list_size(cargaNodos); i++){
		infoNodo* nodoEnLista = list_get(listaNodos, i);
		if(strcmp(nodoEnLista->nombre, nodo->nombre) == 0)
			return;
	}

	list_add(listaNodos,nodo);
}

t_list* obtenerNodosQueEstanEnLista(t_list* cargaNodos, t_list* listaNodos){
	t_list* resultado = list_create();
	int i;

	for(i = 0; i < list_size(cargaNodos); i++){
		infoNodo* nodoEnLista = list_get(listaNodos, i);
		list_add(resultado, obtenerNodoConNombre(cargaNodos, nodoEnLista->nombre));

	}

	return resultado;
}
infoNodo* obtenerNodoConNombre(t_list *listaNodos, char *nombreNodo){
	int i;
	for (i = 0; i < list_size(listaNodos); i++){
		infoNodo* nodo = list_get(listaNodos, i);
		if (strcmp(nodo->nombre, nombreNodo) == 0)
			return nodo;
	}

	return NULL;
}
infoNodo* obtenerNodoDisponible(t_list* cargaNodos, t_list* listaNodos){
	//agregarNodosQueNoEstanEnListaInterna(cargaNodos, listaNodosParaMap);

	t_list* listaDeNodosCandidatos = obtenerNodosQueEstanEnLista(cargaNodos, listaNodos);
	list_sort(listaDeNodosCandidatos, nodoConMenorCargaPrimero);

	infoNodo* nodo =  list_get(listaDeNodosCandidatos, 0);

	if(nodo->carga + calcularCarga(nodo) > cargaMaxima())
		return NULL;

	infoNodo* nodoAUsar;
	int i;
	for(i = 0; i < list_size(cargaNodos); i++){

		nodoAUsar = list_get(listaNodos, i);
		if (strcmp(nodoAUsar->nombre, nodo->nombre) == 0)
			break;

	}

	nodo->bloque = nodoAUsar->bloque;
	return nodo;
}
uint32_t cargaMaxima(){
	int i;
	uint32_t sum;
	for(i=0; i < list_size(listaNodos); i++){
		infoNodo* nodo = list_get(listaNodos, i);
		sum += nodo->carga;
	}
	return sum;
}
bool nodoConMenorCargaPrimero(void* argNodo1, void* argNodo2){
	return ((infoNodo*)argNodo1)->carga <=  ((infoNodo*)argNodo2)->carga;
}

