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
infoNodo* inicializarWorker(){
	infoNodo* worker = malloc(sizeof(infoNodo));

	worker->bloques  = list_create();
	worker->carga = 0;
	worker->disponibilidad = 0;
	worker->cantTareasHistoricas = 0;
	wlMax = 1;
	return worker;
}
void planificar(job* job){
	infoNodo* worker = inicializarWorker();
	t_list* listaNodos = list_create();

	informacionArchivoFsYama* infoArchivo = recibirInfoArchivo(job);//RECIBE BLOQUES Y TAMAÑO DE FS SOBRE EL ARCHIVO DEL JOB

	llenarListaNodos(listaNodos,infoArchivo);

	calcularDisponibilidadWorkers(listaNodos);

	worker = posicionarClock(listaNodos);//POSICIONA EL CLOCK EN EL WORKER DE MAYOR DISPONIBILIDAD

	int nodos,bloques;

	calcularNodosYBloques(infoArchivo,&nodos,&bloques);

	bool** matrix = llenarMatrizNodosBloques(infoArchivo,nodos,bloques);

	moverClock(worker, listaNodos, matrix, infoArchivo);

	pthread_mutex_lock(&cantTareasHistoricas_mutex);
	worker->cantTareasHistoricas++;
	pthread_mutex_unlock(&cantTareasHistoricas_mutex);

}


void moverClock(infoNodo* workerDesignado, t_list* listaNodos, bool** nodosPorBloque, informacionArchivoFsYama* infoArchivo){
	int i;
	int cantidadBloques = list_size(infoArchivo->informacionBloques);
	infoNodo* worker= malloc(sizeof(infoNodo));
	infoBloque* bloque = malloc(sizeof(infoBloque));
	respuestaSolicitudTransformacion* respuestaAMaster = malloc(sizeof(respuestaSolicitudTransformacion));

	respuestaAMaster->ip = workerDesignado->ip;
	respuestaAMaster->puerto = workerDesignado->puerto;
	respuestaAMaster->bloques = list_create();
	int j;

	for(i=0;i<cantidadBloques;i++){
		bloque = list_get(infoArchivo->informacionBloques, i);

			if(bloqueEstaEn(workerDesignado,nodosPorBloque,i) && (workerDesignado->disponibilidad > 0)){
				modificarCargayDisponibilidad(workerDesignado);
				list_add(respuestaAMaster->bloques, bloque);

				log_trace(logger, "Bloque %i asignado al worker %i | Disponibilidad %i",bloque->numeroBloque, workerDesignado->numero, workerDesignado->disponibilidad);

				workerDesignado = avanzarClock(workerDesignado, listaNodos);
			}
			else if(!bloqueEstaEn(workerDesignado,nodosPorBloque,i) && (workerDesignado->disponibilidad > 0)){
				list_add(respuestaAMaster->bloques,bloque);

				modificarCargayDisponibilidad(workerDesignado);
				log_trace(logger, "Bloque %i asignado al worker %i | Disponibilidad %i",bloque->numeroBloque, workerDesignado->numero, workerDesignado->disponibilidad);
			}
			else{
				restaurarDisponibilidad(workerDesignado);//POSIBLE SOLUCION PORQUE NO ENTRA A LOS OTROS 2 IF
			}
			/*SI EL CLOCK VUELVE A CAER EN EL WORKER DE MAYOR DISPONIBILIDAD QUE ELEGIS AL PRINCIPIO
			 *OSEA QUE DA TODA UNA VUELTA HAY QUE SUMARLE A TODOS LOS WORKERS EL VALOR DE LA DISPONIBILIDAD
			 *BASE TODO
			 *tambien si cualquier worker se queda sin disponibilidad hay que restaurarle
			 **/

	}

}
void restaurarDisponibilidad(infoNodo* worker){
	worker->disponibilidad += getDisponibilidadBase();
}
infoNodo* encontrarWorkerDisponible(t_list* listaNodos){
	bool disponiblidadMayorA0(infoNodo* worker){
		return worker->disponibilidad > 0;
	}

	return list_find(listaNodos, disponiblidadMayorA0);
}

infoNodo* avanzarClock(infoNodo* worker, t_list* listaNodos){
	bool nodoConNumero(infoNodo* nodo){
		return nodo->numero == worker->numero;
	}
	infoNodo* siguienteWorker = malloc(sizeof(infoNodo));
	list_remove_by_condition(listaNodos, nodoConNumero);
	list_add_in_index(listaNodos, list_size(listaNodos), worker);

	siguienteWorker = list_get(listaNodos, 0);

	return siguienteWorker;
}

bool bloqueEstaEn(infoNodo* nodo,bool** nodoXbloque, int bloque){
	int posNodo = nodo->numero;
	return nodoXbloque[posNodo][bloque];
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
	infoNodo* workerDesignado = malloc(sizeof(infoNodo));
	list_sort(listaWorkers, (void*) mayorDisponibilidad);

	workerDesignado = list_get(listaWorkers, 0);//Ya desempata por cantidad de tareas historicas (PROBAR)

	return workerDesignado;
}

bool mayorDisponibilidad(infoNodo* worker, infoNodo* workerMasDisp){

	int maxDisponibilidad = obtenerDisponibilidadWorker(workerMasDisp);
	int disponibilidad =  obtenerDisponibilidadWorker(worker);

	//if(maxDisponibilidad == disponibilidad){
		//return workerMasDisp->cantTareasHistoricas < worker->cantTareasHistoricas;
		//FIXME FALLA VALGRIND ACA PORQUE NUNCA SE INICIALIZA
//	}

	return maxDisponibilidad > disponibilidad;

}

infoNodo* buscarNodo(t_list* nodos, int numNodo){
	bool nodoConNumero(infoNodo* nodo){
		return nodo->numero == numNodo;
	}

	return list_find(nodos, (void*) nodoConNumero);
}


bool estaActivo(infoNodo* worker){
	return worker->activo == 1;
}

void calcularDisponibilidadWorkers(t_list* nodos){
	//calcularWorkLoadMaxima(nodos);
	list_iterate(nodos, (void*)calcularDisponibilidadWorker);
}

void calcularDisponibilidadWorker(infoNodo* worker){
	worker->disponibilidad = getDisponibilidadBase() + calcularPWL(worker);
}

int obtenerDisponibilidadWorker(infoNodo* worker){//getter
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

uint32_t workLoadMaxima(){//getter
	return wlMax = 0;
}
void agregarJobAPlanificar(job* jobAPlanificar){
	list_add(jobsAPlanificar,jobAPlanificar);
}

void agregarNodo(t_list* lista, infoNodo* nodo){
	list_add(lista,nodo);
}

void modificarCargayDisponibilidad(infoNodo* worker){
	worker->disponibilidad--;
	worker->carga++;
}

