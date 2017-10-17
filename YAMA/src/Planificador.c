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


void planificar(job* job){
	//pedido lista de bloques de job->rutaDatos
	informacionArchivoFsYama* infoArchivo = malloc(sizeof(informacionArchivoFsYama));
	infoNodo* worker = NULL;

	infoArchivo = recibirInfoArchivo(job);
	void seleccionarWorkerConBloque(uint32_t bloque){
		seleccionarWorker(worker, bloque);
	}
	list_iterate(infoArchivo->informacionBloques, (void*)seleccionarWorkerConBloque);
}

informacionArchivoFsYama* recibirInfoArchivo(job* job) {
	solicitudInfoNodos* solTransf = malloc(sizeof(solicitudInfoNodos));

	solTransf->rutaDatos.cadena = strdup(job->rutaDatos.cadena);
	solTransf->rutaDatos.longitud = job->rutaDatos.longitud;
	solTransf->rutaResultado.cadena = strdup(job->rutaResultado.cadena);
	solTransf->rutaResultado.longitud = job->rutaResultado.longitud;

	return solicitarInformacionAFS(solTransf);
}

void asignarNodoA(job* unJob, infoNodo* worker){
	//empaquetar designar worker
}

void seleccionarWorker(infoNodo* worker, uint32_t numeroBloque){
	infoNodo* workerActual = buscarNodo(listaNodos, worker->nombre);

	if((worker == NULL || mayorDisponibilidad(workerActual, worker))){//&& estaActivo(workerActual)){
		worker = workerActual;
		//asignar bloque
	}
}

bool mayorDisponibilidad(infoNodo* worker, infoNodo* workerMasDisp){
	return calcularDisponibilidadWorker(workerMasDisp) > calcularDisponibilidadWorker(worker);
}

infoNodo* buscarNodo(t_list* nodos, char* nombreNodo){
	bool nodoConNombre(infoNodo* nodo){
		return string_equals_ignore_case(nodo->nombre, nombreNodo);
	}

	return list_find(nodos, (void*) nodoConNombre);
}

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
		return workLoadGlobal() - worker->carga;
	}
	else{
		return 0;
	}
}

uint32_t workLoadGlobal(){
	int i;
	uint32_t sum;
	for(i=0; i < list_size(listaNodos); i++){
		infoNodo* nodo = list_get(listaNodos, i);
		sum += nodo->carga;
	}
	return sum;
}

void agregarJobAPlanificar(job* jobAPlanificar){
	list_add(jobsAPlanificar,jobAPlanificar);
}

void agregarNodo(t_list* lista, infoNodo* nodo){
	list_add(lista,nodo);
}


/*t_list* obtenerNodosQueEstanEnLista(t_list* cargaNodos, t_list* listaNodos){
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
*/



