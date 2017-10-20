/*
 * Planificador.c
 *
 *  Created on: 3/10/2017
 *      Author: utnso
 */

#include "Planificador.h"

void iniciarListasPlanificacion(){

	jobsAPlanificar = list_create();
}


void planificar(job* job){
	infoNodo* worker = malloc(sizeof(infoNodo));
	t_list* listaNodos = list_create();

	informacionArchivoFsYama* infoArchivo = recibirInfoArchivo(job);//RECIBE BLOQUES Y TAMAÑO DE FS SOBRE EL ARCHIVO DEL JOB

	//llenarListaNodos(listaNodos,infoArchivo);

	calcularDisponibilidadWorkers(listaNodos);

	worker = posicionarClock(listaNodos);//POSICIONA EL CLOCK EN EL WORKER DE MAYOR DISPONIBILIDAD

	int nodos,bloques;

	calcularNodosYBloques(infoArchivo,&nodos);

	bool** matrix = llenarMatrizNodosBloques(infoArchivo,nodos,bloques);

	//moverClock(worker, listaNodos);

	pthread_mutex_lock(&cantTareasHistoricas_mutex);
	worker->cantTareasHistoricas++;
	pthread_mutex_unlock(&cantTareasHistoricas_mutex);

}

void moverClock(infoNodo* worker, t_list* listaNodos){

}

informacionArchivoFsYama* recibirInfoArchivo(job* job) {
	solicitudInfoNodos* solTransf = malloc(sizeof(solicitudInfoNodos));

	solTransf->rutaDatos.cadena = strdup(job->rutaDatos.cadena);
	solTransf->rutaDatos.longitud = job->rutaDatos.longitud;
	solTransf->rutaResultado.cadena = strdup(job->rutaResultado.cadena);
	solTransf->rutaResultado.longitud = job->rutaResultado.longitud;

	return solicitarInformacionAFS(solTransf);
}

infoNodo* posicionarClock(t_list* listaWorkers){
	infoNodo* workerDesignado;
	list_sort(listaWorkers, (void*) mayorDisponibilidad);

	workerDesignado = list_get(listaWorkers, 0);//Ya desempata por cantidad de tareas historicas (PROBAR)

	return workerDesignado;
}

bool mayorDisponibilidad(infoNodo* worker, infoNodo* workerMasDisp){

	int maxDisponibilidad = obtenerDisponibilidadWorker(workerMasDisp);
	int dispoinbilidad =  obtenerDisponibilidadWorker(worker);

	if(maxDisponibilidad == dispoinbilidad){
		return workerMasDisp->cantTareasHistoricas < worker->cantTareasHistoricas;
	}

	return maxDisponibilidad > dispoinbilidad;

}

infoNodo* buscarNodo(t_list* nodos, int numNodo){
	bool nodoConNombre(infoNodo* nodo){
		return nodo->numero == numNodo;
	}

	return list_find(nodos, (void*) nodoConNombre);
}


bool estaActivo(infoNodo* worker){
	return worker->activo == 1;
}

void calcularDisponibilidadWorkers(t_list* nodos){
	calcularWorkLoadMaxima(nodos);
	list_iterate(nodos, (void*)calcularDisponibilidadWorker);
}

void calcularDisponibilidadWorker(infoNodo* worker){
	worker->disponibilidad = getDisponibilidadBase() + calcularPWL(worker);
}

int obtenerDisponibilidadWorker(infoNodo* worker){
	return worker->disponibilidad;
}
uint32_t calcularPWL(infoNodo* worker){
	if(esClock() != 0){
		return workLoadMaxima() - worker->carga;
	}
	else{
		return 0;
	}
}

void calcularWorkLoadMaxima(t_list* nodos){
	infoNodo* worker;
	bool mayorCarga(infoNodo* nodoMasCarga, infoNodo* nodo){
		return nodoMasCarga->carga > nodo->carga;
	}
	list_sort(nodos,(void*)mayorCarga);

	worker = list_get(nodos, 0);
	wlMax = worker->carga;
}

uint32_t workLoadMaxima(){
	return wlMax;
}
void agregarJobAPlanificar(job* jobAPlanificar){
	list_add(jobsAPlanificar,jobAPlanificar);
}

void agregarNodo(t_list* lista, infoNodo* nodo){
	list_add(lista,nodo);
}




