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
	pthread_mutex_init(&listaNodos_mutex, NULL);
}

void planificar(job* job, infoNodo* nodo){
	t_list* listaWorkersConBloques = list_create();	//localizar los bloques en FS TOdo
	pthread_mutex_lock(&listaNodos_mutex);
	calcularCargasDeWorkers(listaNodos);
	pthread_mutex_unlock(&listaNodos_mutex);

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


//ELIMINA ELEMENTOS DE LA LISTA DE NODOS Y LA LISTA DE COPIAS DE CADA NODO A MODO QUE AL FINALIZAR SOLO QUEDEN NODOS CON COPIAS DE
//BLOQUES AUN NO ASIGNADOS Y LOS NODOS SOLO POSEEN COPIAS SIN ASIGNAR
/*void actualizarNodo(infoNodo* ultimoNodoSeleccionado, t_list* nodos){
    pthread_mutex_lock( &listaNodos_mutex );
    int indexNodos = 0;
    int indexCopias = 0;
    infoNodo* nodoAux;
    infoNodo* copiaAux;
    char* ipNodoSeleccionado = malloc( strlen( inet_ntoa(ultimoNodoSeleccionado->ip)) + 1);
    strcpy( ipNodoSeleccionado, inet_ntoa(ultimoNodoSeleccionado->ip));

    while(indexNodos < list_size(nodos))
    {
        nodoAux = list_get(nodos, indexNodos);
        if(strcmp(ipNodoSeleccionado, inet_ntoa(nodoAux->ip)) == 0)
        {
           //SI EL NODO OBTENIDO DE LA LISTA ES EL MISMO QUE EL ULTIMO NODO SELECCIONADO, LO ELIMINO DE LA LISTA
            list_remove(nodos, indexNodos);
        }
        else
        {
          /* while(indexCopias < list_size(nodoAux->copias))
            {
              copiaAux = (t_copiasPorNodo*)list_get(nodoAux->copias,indexCopias);
              if(bitarray_test_bit(bitarray, copiaAux->bloqueDeArchivo) == true)

                //SI LA COPIA DEL NODO OBTENIDA DE LA LISTA YA FUE ELEGIDA PARA OTRO NODO, BORRO LA COPIA DE LA LISTA DEL NODO

                list_remove(nodoAux->copias, indexCopias);
                else
                    indexCopias++;

            }
        if(list_size(nodoAux->copias) == 0)
        //SI EL NODO YA NO TIENE COPIAS SIN ASIGNAR ENTONCES LO BORRO DE LA LISTA
            list_remove(nodos, indexNodos);
        else
            indexNodos++;

        indexCopias = 0;
        }

    }
    return;
    pthread_mutex_unlock( &listaNodos_mutex );


}*/

void calcularCargasDeWorkers(t_list* listaNodos){
	int i;
	infoNodo* worker;
	for(i = 0; i < list_size(listaNodos); i++){
		worker = list_get(listaNodos, i);
		//worker->carga+=;
	}
}

t_list* consultarDetallesBloqueArchivo(char *pathArchivo, int bloque){
	return 0; //FIXME
}

int calcularDisponibilidadWorker(infoNodo* worker){
		return getDisponibilidadBase() + calcularPWL(worker);
}

uint32_t calcularPWL(infoNodo* worker){
	if(esClock() != 0){
		return worker->carga + workLoadGlobal();
	}
	else{
		return 0;
	}
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

	if(nodo->carga > cargaMaxima())
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

