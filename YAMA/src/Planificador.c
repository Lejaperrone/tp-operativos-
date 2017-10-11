/*
 * Planificador.c
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#include "Planificador.h"

void iniciarListasPlanificacion(){
	listaNodos = list_create();
	jobsAPlanificar = list_create();
}

void planificar(job* job, infoNodo* nodo){
	t_list* listaWorkersConBloques = list_create();	//localizar los bloques en FS TOdo

	calcularCargasDeWorkers();

	//infoNodo* workerCandidato = obtenerNodoDisponible(listaNodos, listaWorkersConBloques);
	posicionarClock(listaWorkersConBloques);
	//comparar cargas
}

void posicionarClock(t_list* listaWorkersConBloques){
	int i;
	infoNodo* workerAsignado;
	for(i=0;i<list_size(listaWorkersConBloques);i++){
		//workerAsignado = obtenerNodosDisponibles(listaWorkersConBloques,i); FIXME PARA WCLOCK
		workerAsignado = list_get(listaWorkersConBloques, i);//CLOCK ELIJO CUALQUIERA

	}
}

void asignarNodoA(job* unJob, infoNodo* worker){
	//empaquetar designar worker
}

void calcularCargasDeWorkers(){
	int i;

	for(i = 0; i < list_size(listaNodos); i++){
		infoNodo* worker = list_get(listaNodos, i);
		worker->carga = calcularDisponibilidadWorker(worker);
	}
}

t_list* consultarDetallesBloqueArchivo(char *pathArchivo, int bloque){
	return 0; //FIXME
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

void agregarJobAPlanificar(job* jobAPlanificar){
	list_add(jobsAPlanificar,jobAPlanificar);
}

void agregarNodo(t_list* lista, infoNodo* nodo){
	list_add(lista,nodo);
}

t_list* obtenerNodosQueEstanEnLista(t_list* cargaNodos, t_list* listaNodos){
	t_list* resultado = list_create();
	int i;

	for(i = 0; i < list_size(listaNodos); i++){
		infoNodo* nodoEnLista = list_get(listaNodos, i);
		list_add(resultado, obtenerNodoConNombre(nodoEnLista->nombre));
	}

	return resultado;
}

infoNodo* obtenerNodoConNombre(char *nombreNodo){
	int i;
	for (i = 0; i < list_size(listaNodos); i++){
		infoNodo* nodo = list_get(listaNodos, i);
		if (strcmp(nodo->nombre, nombreNodo) == 0)
			return nodo;
	}

	return NULL;
}

infoNodo* obtenerNodoDisponible(t_list* cargaNodos, t_list* listaNodos){
	agregarNodos(cargaNodos, listaNodos);

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

void agregarNodos(t_list* cargaNodos, t_list* listaNodos){
	int i;
	for(i=0;i < list_size(listaNodos);i++)	{
		infoNodo* nodoEnLista = list_get(listaNodos, i);
		agregarNodo(cargaNodos, nodoEnLista);
	}
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

