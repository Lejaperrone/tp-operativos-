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


void planificar(job* job){
	infoNodo* worker = malloc(sizeof(infoNodo));
	infoBloque* bloque = malloc(sizeof(infoBloque));

	informacionArchivoFsYama* infoArchivo = recibirInfoArchivo(job);//RECIBE BLOQUES Y TAMAÑO DE FS SOBRE EL ARCHIVO DEL JOB

	actualizarNodosConectados(infoArchivo);

	worker = posicionarClock(listaNodos);//POSICIONA EL CLOCK EN EL WORKER DE MAYOR DISPONIBILIDAD

	int nodos,bloques;

	calcularNodosYBloques(infoArchivo,&nodos,&bloques);

	bool** matrix = llenarMatrizNodosBloques(infoArchivo,nodos,bloques);


	//CHEQUEADO QUE RECIBE TODO OK

	//todo

}

informacionArchivoFsYama* recibirInfoArchivo(job* job) {
	solicitudInfoNodos* solTransf = malloc(sizeof(solicitudInfoNodos));

	solTransf->rutaDatos.cadena = strdup(job->rutaDatos.cadena);
	solTransf->rutaDatos.longitud = job->rutaDatos.longitud;
	solTransf->rutaResultado.cadena = strdup(job->rutaResultado.cadena);
	solTransf->rutaResultado.longitud = job->rutaResultado.longitud;

	return solicitarInformacionAFS(solTransf);
}

void bloqueEstaEnWorker(infoBloque* bloque, infoNodo* worker){
	//TODO
}
void seleccionarWorker(infoNodo* worker, infoBloque bloque){
	infoNodo* workerActual = buscarNodo(listaNodos, worker->nombre);

	if((worker == NULL || mayorDisponibilidad(workerActual, worker)) && estaActivo(workerActual)){
		worker = workerActual;
		worker->bloque = bloque;
	}
}

infoNodo* posicionarClock(t_list* listaWorkers){
	infoNodo* workerDesignado;
	list_sort(listaWorkers, (void*) mayorDisponibilidad);

	workerDesignado = list_get(listaWorkers, 0);//FIXME DESEMPATAR POR MENOS TAREAS HISTORICAMENTE

	return workerDesignado;
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

void calcularCargasDeWorkers(t_list* nodos){
	int i;
	infoNodo* worker;
	for(i = 0; i < list_size(nodos); i++){
		worker = list_get(nodos, i);
		//worker->carga+=;
	}
}
bool estaActivo(infoNodo* worker){
	return worker->activo == 1;
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



*/



