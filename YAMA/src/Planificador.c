/*
 * Planificador.c
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#include "Planificador.h"
t_list* listaNodos;
t_list* jobsAPlanificar;

void planificar(infoJob* job, infoNodo nodo){
	//localizar los bloques en FS

	//calcular carga/score de cada nodo. Recomendable aplicar funcion de Availability

	//comparar cargas
}
int calcularDisponibilidadWorker(infoNodo worker){
	if(esClock()){
		return disponibilidadBase;
	}
	else{
		return disponibilidadBase + calcularWorkLoad(worker);
	}
}
uint32_t calcularWorkLoad(infoNodo* worker){
	return workLoadGlobal() - worker->carga;
}
uint32_t workLoadGlobal(){
	return 0; //FIXME
}
int getDisponibilidadBase(){
	return disponibilidadBase;
}
t_list* nuevaCargaWorkers(){
	t_list* cargaWorkers = malloc(sizeof(t_list));
	cargaWorkers = list_create();
	return cargaWorkers;
}

void agregarJobAPlanificar(infoJob* jobAPlanificar){
	list_add(jobsAPlanificar, (void*)jobAPlanificar);
}

void agregarNodo(t_list* cargaNodo,infoNodo* nodo){
	int contador = 0;
	while(contador < list_size(cargaNodo))
	{
		infoNodo* nodoEnLista = list_get(listaNodos, contador);
		if(strcmp(nodoEnLista->nombre, nodo->nombre) == 0)
			return;

		contador++;
	}

	list_add(listaNodos,nodo);
}

t_list* obtenerNodosQueEstanEnLista(t_list* cargaNodos, t_list* listaNodos){
	t_list* resultado = list_create();

	int contador = 0;
	while (contador < list_size(listaNodos))
	{
		infoNodo* nodoEnLista = list_get(listaNodos, contador);
		list_add(resultado, obtenerNodoConNombre(cargaNodos, nodoEnLista->nombre));

		contador ++;
	}

	return resultado;
}

infoNodo* obtenerNodoDisponible(t_list* cargaNodos, t_list* listaNodosParaMap){
	agregarNodosQueNoEstanEnListaInterna(cargaNodos, listaNodosParaMap);

	t_list* listaDeNodosCandidatos = obtenerNodosQueEstanEnLista(cargaNodos, listaNodosParaMap);
	list_sort(listaDeNodosCandidatos, nodoConMenorCargaPrimero);

	infoNodo* nodo =  list_get(listaDeNodosCandidatos, 0);

	if(nodo->carga + getCargaMap() > getCargaMaxima())
		return NULL;

	int contador = 0;
	infoNodo * nodoAUsar;
	while (contador < list_size(listaNodosParaMap))
	{
		nodoAUsar = list_get(listaNodosParaMap, contador);
		if (strcmp(nodoAUsar->nombre, nodo->nombre) == 0)
			break;

		contador ++;
	}

	nodo->bloque = nodoAUsar->bloque;
	return nodo;
}

bool nodoConMenorCargaPrimero(void* argNodo1, void* argNodo2){
	return ((infoNodo*)argNodo1)->carga <=  ((infoNodo*)argNodo2)->carga;
}
void nodoAsignarCarga(infoNodo* nodo){
	nodo->carga += cargaDefault;
}
