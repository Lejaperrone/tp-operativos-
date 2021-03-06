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
	return worker;
}
void planificar(job* job){
	pthread_mutex_lock(&mutexJobs);
	infoNodo* worker = malloc(sizeof(infoNodo));
	pthread_mutex_unlock(&mutexJobs);
	t_list* listaNodos = list_create();
	int nodos,bloques;

	informacionArchivoFsYama* infoArchivo = recibirInfoArchivo(job);

	if(infoArchivo->tamanioTotal==-1){
		printf("\nFallo de ingreso de ruta datos o ruta resultado en job %d\n",job->id);
		finalizarJob(job,TRANSFORMACION,FALLO_INGRESO);
	}

	void cargas(infoNodo* nodo){
		log_trace(logger,"Nodo %d Carga %d",nodo->numero,nodo->carga);
	}

	list_iterate(nodosConectados,(void*)cargas);

	llenarListaNodos(listaNodos,infoArchivo,job->id);

	calcularDisponibilidadWorkers(listaNodos,job->id);

	worker = posicionarClock(listaNodos,job->id);

	calcularNodosYBloques(infoArchivo,&nodos,&bloques,job->id);

	bool** matrix = llenarMatrizNodosBloques(infoArchivo,nodos,bloques,job->id);

	respuestaSolicitudTransformacion* respuestaMaster = moverClock(worker, listaNodos, matrix, infoArchivo->informacionBloques,job->id,nodos);

	log_trace(logger,"Envio pre planificacion a Master para job %d",job->id);

	empaquetar(job->socketFd,mensajeRespuestaTransformacion,0,respuestaMaster);

	void reparticion(workerDesdeYama* worker){
		printf("Worker %d, tareas %d \n",worker->numeroWorker,list_size(worker->bloquesConSusArchivos));
	}

	list_iterate(respuestaMaster->workers,(void*)reparticion);

	planificarReduccionesLocales(job,matrix,respuestaMaster,nodos,bloques,listaNodos);

	int nodoEncargado = enviarReduccionGlobalAMaster(job);

	actualizarCargasNodos(nodoEncargado);

	esperarRespuestaReduccionDeMaster(job);

	realizarAlmacenamientoFinal(job);

	free(infoArchivo);
	free(respuestaMaster);
	free(listaNodos);
	free(worker);

	finalizarJob(job,ALM_FINAL,OK);

}


respuestaSolicitudTransformacion* moverClock(infoNodo* workerDesignado, t_list* listaNodos, bool** nodosPorBloque, t_list* infoBloques,int job,int nodos){
	int i;
	int cantidadBloques = list_size(infoBloques);
	infoBloque* bloque = malloc(sizeof(infoBloque));
	respuestaSolicitudTransformacion* respuestaAMaster = malloc(sizeof(respuestaSolicitudTransformacion));
	respuestaAMaster->workers = list_create();

	for(i=0;i<cantidadBloques;i++){
		bloque = (infoBloque*)list_get(infoBloques, i);

		usleep(config.RETARDO_PLANIFICACION*1000);

		if(bloqueEstaEn(workerDesignado,nodosPorBloque,bloque->numeroBloque)){
			modificarCargayDisponibilidad(workerDesignado);
			agregarBloqueANodoParaEnviar(bloque,workerDesignado,respuestaAMaster,job);
			log_trace(logger, "Bloque %i asignado al worker %i para job %d | Disp %i",bloque->numeroBloque, workerDesignado->numero,job, workerDesignado->disponibilidad);
			printf("Bloque %i asignado al worker %i para job %d\n",bloque->numeroBloque, workerDesignado->numero,job);


			workerDesignado = avanzarClock(workerDesignado, listaNodos);

			while(workerDesignado->disponibilidad <= 0){
				log_trace(logger, "Worker %i sin disponibilidad para job %d ",workerDesignado->numero,job);
				workerDesignado->disponibilidad = getDisponibilidadBase();
				workerDesignado = avanzarClock(workerDesignado, listaNodos);
			}

		}
		else {
			log_trace(logger,"El worker %d no tiene el bloque %d para job %d\n",workerDesignado->numero,bloque->numeroBloque,job);
			infoNodo* proximoWorker = obtenerProximoWorkerConBloque(listaNodos,bloque->numeroBloque,workerDesignado->numero,nodosPorBloque,nodos);
			modificarCargayDisponibilidad(proximoWorker);

			agregarBloqueANodoParaEnviar(bloque,proximoWorker,respuestaAMaster,job);
			printf("Bloque %i asignado al worker %i para job %d\n",bloque->numeroBloque, proximoWorker->numero,job);
			log_trace(logger, "Bloque %i asignado al worker %i para job %d | Disp %i",bloque->numeroBloque, proximoWorker->numero,job ,proximoWorker->disponibilidad);
		}

	}

	return respuestaAMaster;

}

void verificarValorDisponibilidad(infoNodo* nodo){
	if(nodo->disponibilidad == 0){
		restaurarDisponibilidad(nodo);
	}
}
void restaurarDisponibilidad(infoNodo* worker){
	worker->disponibilidad = getDisponibilidadBase();
}

infoNodo* avanzarClock(infoNodo* worker, t_list* listaNodos){
	bool nodoConNumero(infoNodo* nodo){
		return nodo->numero == worker->numero;
	}

	infoNodo* siguienteWorker = malloc(sizeof(infoNodo));
	list_remove_by_condition(listaNodos, (void*)nodoConNumero);
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
	informacionArchivoFsYama* infoArchivo = malloc(sizeof(informacionArchivoFsYama));

	solTransf->rutaDatos.cadena = strdup(job->rutaDatos.cadena);
	solTransf->rutaDatos.longitud = job->rutaDatos.longitud;
	solTransf->rutaResultado.cadena = strdup(job->rutaResultado.cadena);
	solTransf->rutaResultado.longitud = job->rutaResultado.longitud;

	log_trace(logger,"Envio de job %d a FS",job->id);

	infoArchivo = solicitarInformacionAFS(solTransf,job->id);
	return infoArchivo;
}

infoNodo* posicionarClock(t_list* listaWorkers,int job){
	infoNodo* workerDesignado = malloc(sizeof(infoNodo));
	list_sort(listaWorkers, (void*)mayorDisponibilidad);

	workerDesignado = list_get(listaWorkers, 0);

	log_trace(logger,"Posiciono clock en worker %d para job %d",workerDesignado->numero,job,workerDesignado->cantTareasHistoricas);


	int i=0;

	void ordenar(infoNodo* worker){
		i++;
		log_trace(logger,"Worker %d orden %d tareas %d",worker->numero,i,worker->cantTareasHistoricas);
	}

	list_iterate(listaWorkers,(void*)ordenar);

	return workerDesignado;
}

bool mayorDisponibilidad(infoNodo* workerMasDisp, infoNodo* worker){

	int maxDisponibilidad = obtenerDisponibilidadWorker(workerMasDisp);
	int disponibilidad =  obtenerDisponibilidadWorker(worker);
	if(maxDisponibilidad == disponibilidad){
		return workerMasDisp->cantTareasHistoricas < worker->cantTareasHistoricas;
	}
	return maxDisponibilidad > disponibilidad;

}

infoNodo* buscarNodo(t_list* nodos, int numNodo){
	bool nodoConNumero(infoNodo* nodo){
		return nodo->numero == numNodo;
	}

	return list_find(nodos, (void*) nodoConNumero);
}

bool estaActivo(infoNodo* worker){
	return worker->conectado == true;
}

void calcularDisponibilidadWorkers(t_list* nodos,int job){
	int max = calcularWorkLoadMaxima(nodos);

	void calcularDisponibilidadWorker(infoNodo* worker){
		worker->disponibilidad = getDisponibilidadBase() + calcularPWL(worker,max);
		log_trace(logger,"Disponibilidad %d para %d worker job %d",worker->disponibilidad,worker->numero,job);
	}


	list_iterate(nodos, (void*)calcularDisponibilidadWorker);
	log_trace(logger,"Disponibilidad calculada para job %d",job);
}

int obtenerDisponibilidadWorker(infoNodo* worker){//getter
	return worker->disponibilidad;
}

uint32_t calcularPWL(infoNodo* worker,int max){
	if(esClock() != 0){
		pthread_mutex_lock(&mutex_NodosConectados);
		infoNodo* nodo = obtenerNodo(worker->numero);
		int carga = nodo->carga;
		pthread_mutex_unlock(&mutex_NodosConectados);


		return max - carga;
	}
	else{
		return 0;
	}
}

int calcularWorkLoadMaxima(t_list* nodos){
	infoNodo* worker = malloc(sizeof(infoNodo));
	bool mayorCarga(infoNodo* nodoMasCarga, infoNodo* nodo){
		pthread_mutex_lock(&mutex_NodosConectados);
		infoNodo* nodoMas = obtenerNodo(nodoMasCarga->numero);
		infoNodo* nodoMenos = obtenerNodo(nodo->numero);
		int mas = nodoMas->carga;
		int menos = nodoMenos->carga;
		pthread_mutex_unlock(&mutex_NodosConectados);

		return mas > menos;
	}
	list_sort(nodos,(void*)mayorCarga);

	worker = list_get(nodos, 0);

	pthread_mutex_lock(&mutex_NodosConectados);
	infoNodo* nodoMax = obtenerNodo(worker->numero);
	int wlMax = nodoMax->carga;
	pthread_mutex_unlock(&mutex_NodosConectados);
	return wlMax;
}

void agregarJobAPlanificar(job* jobAPlanificar){
	list_add(jobsAPlanificar,jobAPlanificar);
}

void agregarNodo(t_list* lista, infoNodo* nodo){
	list_add(lista,nodo);
}

void modificarCargayDisponibilidad(infoNodo* worker){
	worker->disponibilidad--;

	pthread_mutex_lock(&mutex_NodosConectados);
	infoNodo* nodo = obtenerNodo(worker->numero);
	nodo->carga++;
	nodo->cantTareasHistoricas++;
	pthread_mutex_unlock(&mutex_NodosConectados);

}

infoNodo* obtenerProximoWorkerConBloque(t_list* listaNodos,int bloque,int numWorkerActual,bool** matrix,int nodos){

	void restaurarDisp(infoNodo* nodo){
		nodo->disponibilidad = getDisponibilidadBase();
	}

	infoNodo* info;
	int i;

	int inicio;
	if(numWorkerActual==nodos){
		inicio=0;
	}
	else{
		inicio=numWorkerActual+1;
	}

	for(i=inicio;i<=nodos;i++){
		bool nodoEnLista(infoNodo* nodo){
			return nodo->numero == i;
		}

		info = list_find(listaNodos,(void*)nodoEnLista);

		if(matrix[i][bloque] &&(info->disponibilidad > 0)){
			return info;
		}

		if(i==numWorkerActual){
			list_iterate(listaNodos,(void*)restaurarDisp);
		}

		if(i==nodos){
			i=0;
		}


	}
	return info;
}

void agregarBloqueANodoParaEnviar(infoBloque* bloque,infoNodo* nodo,respuestaSolicitudTransformacion* respuestaMaster,int job){
	workerDesdeYama* worker;
	bloquesConSusArchivosTransformacion* bloquesArchivos = malloc(sizeof(bloquesConSusArchivosTransformacion));

	bool nodoConNumero(workerDesdeYama* worker){
		return worker->numeroWorker == nodo->numero;
	}

	if( list_find(respuestaMaster->workers, (void*) nodoConNumero)){
		worker = list_find(respuestaMaster->workers, (void*) nodoConNumero);
	}
	else{
		worker = malloc(sizeof(workerDesdeYama));
		worker->numeroWorker = nodo->numero;
		worker->puerto = nodo->puerto;
		worker->ip.longitud = nodo->ip.longitud;
		worker->ip.cadena = strdup(nodo->ip.cadena);
		worker->bloquesConSusArchivos = list_create();
		list_add(respuestaMaster->workers,worker);
	}

	char* rutaTemporal = dameUnNombreArchivoTemporal(job,bloque->numeroBloque,TRANSFORMACION,worker->numeroWorker);

	bloquesArchivos->numBloque = bloque->numeroBloque;
	bloquesArchivos->bytesOcupados = bloque->bytesOcupados;
	bloquesArchivos->archivoTemporal.cadena = strdup(rutaTemporal);
	bloquesArchivos->archivoTemporal.longitud = string_length(rutaTemporal);

	bool nodoEsta(ubicacionBloque* ubi){
		return ubi->numeroNodo == nodo->numero;
	}

	ubicacionBloque* subi = list_find(bloque->ubicaciones,(void*)nodoEsta);
	bloquesArchivos->numBloqueEnNodo = subi->numeroBloqueEnNodo;

	list_add(worker->bloquesConSusArchivos,bloquesArchivos);

	registroTablaEstados* registro = malloc(sizeof(registroTablaEstados));
	registro->bloque= bloquesArchivos->numBloque;
	registro->estado=EN_EJECUCION;
	registro->etapa= TRANSFORMACION;
	registro->job = job;
	registro->nodo= nodo->numero;
	registro->rutaArchivoTemp = strdup(bloquesArchivos->archivoTemporal.cadena);

	pthread_mutex_lock(&mutexTablaEstados);
	list_add(tablaDeEstados,registro);
	pthread_mutex_unlock(&mutexTablaEstados);

}

void replanificar(int paraReplanificar,job* jobi,respuestaSolicitudTransformacion* respuestaArchivo, bool** matrix,int bloques,int nodos,t_list* listaNodos){
	respuestaSolicitudTransformacion* respuestaTransfromacion = malloc(sizeof(respuestaSolicitudTransformacion));
	respuestaTransfromacion->workers = list_create();

	bool nodoConNumero(workerDesdeYama* worker){
		return worker->numeroWorker == paraReplanificar;
	}

	workerDesdeYama* worker = list_find(respuestaArchivo->workers, (void*) nodoConNumero);

	void agregarBloque(bloquesConSusArchivosTransformacion* bloque){
		bloquesConSusArchivosTransformacion* bloqueNuevo = malloc(sizeof(bloquesConSusArchivosTransformacion));

		bloqueYNodo* nodoBloque = malloc(sizeof(bloqueYNodo));
		nodoBloque->bloque= bloque->numBloque;
		nodoBloque->workerId = paraReplanificar;

		int nodoNuevo = nodoConOtraCopia(nodoBloque,matrix,nodos,bloques,listaNodos);

		if(nodoNuevo == -1){
			log_trace(logger,"Fallo en replanificar nodo %d", paraReplanificar);
			finalizarJob(jobi,TRANSFORMACION,FALLO_TRANSFORMACION);
		}

		bool nodoYaCargado(workerDesdeYama* workerCargado){
			return workerCargado->numeroWorker == nodoNuevo;
		}

		workerDesdeYama* nuevoWorker;
		if(list_any_satisfy(respuestaTransfromacion->workers,(void*)nodoYaCargado)){
			nuevoWorker=list_find(respuestaTransfromacion->workers,(void*)nodoYaCargado);
		}
		else{
			nuevoWorker = malloc(sizeof(workerDesdeYama));
			nuevoWorker->numeroWorker = nodoNuevo;

			pthread_mutex_lock(&mutex_NodosConectados);
			infoNodo* infoNod = obtenerNodo(nodoNuevo);

			nuevoWorker->puerto= infoNod->puerto;
			nuevoWorker->ip.cadena = strdup(infoNod->ip.cadena);
			nuevoWorker->ip.longitud = infoNod->ip.longitud;

			pthread_mutex_unlock(&mutex_NodosConectados);
			list_add(respuestaTransfromacion->workers,nuevoWorker);
			nuevoWorker->bloquesConSusArchivos =list_create();
		}

		bloqueNuevo->archivoTemporal.cadena = dameUnNombreArchivoTemporal(jobi->id,bloque->numBloque,TRANSFORMACION,nodoNuevo);
		bloqueNuevo->archivoTemporal.longitud = string_length(bloqueNuevo->archivoTemporal.cadena);

		bloqueNuevo->bytesOcupados = bloque->bytesOcupados;
		bloqueNuevo->numBloque = bloque->numBloque;
		bloqueNuevo->numBloqueEnNodo = bloque->numBloqueEnNodo;

		list_add(nuevoWorker->bloquesConSusArchivos,bloqueNuevo);

		log_trace(logger, "Replanifico bloque %i asignado al worker %i ",bloque->numBloque, nodoNuevo);


		pthread_mutex_lock(&mutexTablaEstados);

		bool regEnTabla(registroTablaEstados* reg){
			return reg->job == jobi->id && reg->etapa == TRANSFORMACION && reg->bloque==bloque->numBloque;
		}

		registroTablaEstados* registroViejo =list_find(tablaDeEstados,(void*)regEnTabla);
		registroViejo->estado = ERROR;

		registroTablaEstados* registro = malloc(sizeof(registroTablaEstados));
		registro->bloque= bloque->numBloque;
		registro->estado=EN_EJECUCION;
		registro->etapa= TRANSFORMACION;
		registro->job= jobi->id;
		registro->nodo= nodoNuevo;
		registro->rutaArchivoTemp = strdup(bloqueNuevo->archivoTemporal.cadena);


		list_add(tablaDeEstados,registro);
		pthread_mutex_unlock(&mutexTablaEstados);

		pthread_mutex_lock(&mutex_NodosConectados);
		infoNodo* nodo = obtenerNodo(paraReplanificar);
		nodo->carga--;

		nodo = obtenerNodo(nodoNuevo);
		nodo->carga++;
		nodo->cantTareasHistoricas++;
		pthread_mutex_unlock(&mutex_NodosConectados);
	}

	list_iterate(worker->bloquesConSusArchivos,(void*)agregarBloque);

	empaquetar(jobi->socketFd,mensajeReplanificacion,0,respuestaTransfromacion);


}

void enviarReduccionLocalAMaster(job* job,int nodo){
	nodosRedLocal* respuestaTodos = malloc(sizeof(nodosRedLocal));
	registroTablaEstados* registro = malloc(sizeof(registroTablaEstados));

	bool encontrarEnTablaEstados(registroTablaEstados* reg) {
		return reg->job == job->id && reg->etapa == TRANSFORMACION && reg->nodo==nodo;
	}

	pthread_mutex_lock(&mutexTablaEstados);
	t_list* registrosRedLocal = list_filter(tablaDeEstados,(void*)encontrarEnTablaEstados);
	pthread_mutex_unlock(&mutexTablaEstados);

	pthread_mutex_lock(&mutex_NodosConectados);

	infoNodo* infoNod = obtenerNodo(nodo);
	char* archivo = dameUnNombreArchivoTemporal(job->id,0,RED_LOCAL,nodo);

	respuestaTodos->numeroNodo=nodo;
	respuestaTodos->puerto = infoNod->puerto;
	respuestaTodos->ip.longitud = infoNod->ip.longitud;
	respuestaTodos->ip.cadena = strdup(infoNod->ip.cadena);
	respuestaTodos->archivoTemporal.longitud = string_length(archivo);
	respuestaTodos->archivoTemporal.cadena =  strdup(archivo);
	respuestaTodos->archivos = list_create();

	void meterEnRespuestaRedLocal(registroTablaEstados* reg){
		bool encontrar(int* nodo){
			return *nodo == reg->nodo;
		}

		bool encontrartabla(registroTablaEstados* registro){
			return registro->nodo == reg->nodo;
		}

		string* strArchivo = malloc(sizeof(string));
		strArchivo->longitud = string_length(reg->rutaArchivoTemp);
		strArchivo->cadena = strdup(reg->rutaArchivoTemp);
		list_add(respuestaTodos->archivos,strArchivo);
	}

	registro->bloque=0;
	registro->estado=EN_EJECUCION;
	registro->etapa=RED_LOCAL;
	registro->job= job->id;
	registro->nodo= nodo;
	registro->rutaArchivoTemp = strdup(archivo);

	pthread_mutex_lock(&mutexTablaEstados);
	list_add(tablaDeEstados,registro);
	pthread_mutex_unlock(&mutexTablaEstados);

	//pthread_mutex_lock(&mutexTablaEstados);
	list_iterate(registrosRedLocal,(void*)meterEnRespuestaRedLocal);
	//pthread_mutex_unlock(&mutexTablaEstados);
	pthread_mutex_unlock(&mutex_NodosConectados);

	empaquetar(job->socketFd,mensajeRespuestaRedLocal,0,respuestaTodos);
}

int enviarReduccionGlobalAMaster(job* job){
	int cantArchivosTemp =0;
	respuestaReduccionGlobal* respuesta = malloc(sizeof(respuestaReduccionGlobal));
	respuesta->parametros = malloc(sizeof(parametrosReduccionGlobal));
	respuesta->parametros->infoWorkers = list_create();

	bool encontrarEnTablaEstados(void *registro) {
		registroTablaEstados* reg =(registroTablaEstados*)registro;
		return reg->job == job->id && reg->etapa== RED_LOCAL;
	}
	void meterEnRespuestaRedGlobal(registroTablaEstados* reg){
		infoWorker* info = malloc(sizeof(infoWorker));

		infoNodo* infoNod = obtenerNodo(reg->nodo);

		bool nodoConNumero(workerDesdeYama* worker){
			return worker->numeroWorker == infoNod->numero;
		}

		info->puerto = infoNod->puerto;;
		info->ip.longitud = infoNod->ip.longitud;
		info->ip.cadena = strdup(infoNod->ip.cadena);
		info->nombreArchivoReducido.longitud = string_length(reg->rutaArchivoTemp);
		info->nombreArchivoReducido.cadena = strdup(reg->rutaArchivoTemp);
		cantArchivosTemp++;
		list_add(respuesta->parametros->infoWorkers,info);

	}

	pthread_mutex_lock(&mutexTablaEstados);
	t_list* registrosRedGlobal = list_filter(tablaDeEstados,(void*)encontrarEnTablaEstados);
	pthread_mutex_unlock(&mutexTablaEstados);

	infoNodo* nodoEncargado = obtenerNodo(calcularNodoEncargado(registrosRedGlobal));

	char* archivoReduccionGlobal = dameUnNombreArchivoTemporal(job->id,0,RED_GLOBAL,nodoEncargado->numero);

	respuesta->archivoTemporal.longitud = string_length(archivoReduccionGlobal);
	respuesta->archivoTemporal.cadena = strdup(archivoReduccionGlobal);

	respuesta->ip.cadena = strdup(nodoEncargado->ip.cadena);
	respuesta->ip.longitud = nodoEncargado->ip.longitud;

	respuesta->puerto=nodoEncargado->puerto;

	respuesta->numero = nodoEncargado->numero;
	respuesta->job = job->id;

	list_iterate(registrosRedGlobal,(void*)meterEnRespuestaRedGlobal);

	pthread_mutex_lock(&mutex_NodosConectados);
	infoNodo* nodo = obtenerNodo(nodoEncargado->numero);

	nodo->carga++;
	nodo->cantTareasHistoricas++;

	pthread_mutex_unlock(&mutex_NodosConectados);

	empaquetar(job->socketFd,mensajeRespuestaRedGlobal,0,respuesta);

	registroTablaEstados* registro = malloc(sizeof(registroTablaEstados));
	registro->bloque=0;
	registro->estado=EN_EJECUCION;
	registro->etapa=RED_GLOBAL;
	registro->job= job->id;
	registro->nodo= nodoEncargado->numero;
	registro->rutaArchivoTemp = strdup(respuesta->archivoTemporal.cadena);


	pthread_mutex_lock(&mutexTablaEstados);
	list_add(tablaDeEstados,registro);
	pthread_mutex_unlock(&mutexTablaEstados);

	pthread_mutex_lock(&mutex_NodosConectados);
	infoNodo* nodos = obtenerNodo(nodoEncargado->numero);

	nodos->carga--;

	pthread_mutex_unlock(&mutex_NodosConectados);

	log_trace(logger,"Envio de Reduccion Global de job %d",job->id);
	printf("\nEnvio de Reduccion Global de job %d\n",job->id);

	return nodoEncargado->numero;
}

int calcularNodoEncargado(t_list* registrosRedGlobal){
	int menorCarga,numeroNodo;

	registroTablaEstados* reg = list_get(registrosRedGlobal,0);

	pthread_mutex_lock(&mutex_NodosConectados);
	infoNodo* nodo = obtenerNodo(reg->nodo);
	numeroNodo = reg->nodo;
	menorCarga=nodo->carga;


	int i;
	for(i=1;i<list_size(registrosRedGlobal);i++){
		reg = list_get(registrosRedGlobal,i);
		nodo = obtenerNodo(reg->nodo);

		if(nodo->carga < menorCarga ){
			numeroNodo = reg->nodo;
			menorCarga=nodo->carga;
		}
	}
	pthread_mutex_unlock(&mutex_NodosConectados);
	return numeroNodo;

}

t_list* buscarCopiasBloques(t_list* listaBloques,t_list* listaNodos,informacionArchivoFsYama* infoArchivo){
	t_list* listaNodosActivos = list_create();

	bool nodosConectadosConBloques(infoNodo* worker){
		return worker->conectado == true;
	}

	listaNodosActivos = list_filter(listaNodos, (void*)nodosConectadosConBloques);

	return listaNodosActivos;
}

void realizarAlmacenamientoFinal(job* job){
	bool encontrarEnTablaEstados(void *registro) {
		registroTablaEstados* reg =(registroTablaEstados*)registro;
		return reg->job == job->id && reg->etapa== RED_GLOBAL;
	}

	pthread_mutex_lock(&mutex_NodosConectados);

	registroTablaEstados* reg = list_find(tablaDeEstados,(void*)encontrarEnTablaEstados);

	respuestaAlmacenamiento* respuestaAlm = malloc(sizeof(respuestaAlmacenamiento));
	infoNodo* infoNod = obtenerNodo(reg->nodo);
	infoNod->cantTareasHistoricas++;
	infoNod->carga++;

	pthread_mutex_unlock(&mutex_NodosConectados);

	respuestaAlm->ip.longitud = infoNod->ip.longitud;
	respuestaAlm->ip.cadena = strdup(infoNod->ip.cadena);

	respuestaAlm->nodo = reg->nodo;
	respuestaAlm->puerto = infoNod->puerto;
	respuestaAlm->archivo.longitud = string_length(reg->rutaArchivoTemp);
	respuestaAlm->archivo.cadena = strdup(reg->rutaArchivoTemp);

	empaquetar(job->socketFd,mensajeRespuestaAlmacenamiento,0,respuestaAlm);
	respuesta almac = desempaquetar(job->socketFd);

	if(almac.idMensaje == mensajeAlmacenamientoCompleto){
		printf("\nAlmacenamiento correcto de job %d en FS\n",job->id);
		log_trace(logger, "Almacenamiento correcto de job %d en FS",job->id);

		pthread_mutex_lock(&mutex_NodosConectados);
		infoNodo* nodo = obtenerNodo(reg->nodo);
		nodo->carga--;

		pthread_mutex_unlock(&mutex_NodosConectados);
	}
	else{
		printf("\nAlmacenamiento fallido de job %d en FS\n",job->id);
		log_trace(logger, "Almacenamiento fallido de job %d en FS",job->id);

		pthread_mutex_lock(&mutex_NodosConectados);
		infoNodo* nodo = obtenerNodo(reg->nodo);
		nodo->carga--;

		pthread_mutex_unlock(&mutex_NodosConectados);

		finalizarJob(job,ALM_FINAL,FALLO_ALMACENAMIENTO);
	}
}

void planificarReduccionesLocales(job* job,bool** matrix,respuestaSolicitudTransformacion* respuestaMaster,int nodos,int bloques,t_list* listaNodos){
	bool redLocalIncompleta= true;
	bloqueYNodo* bloqueNodo;
	int numNodo;
	bool* nodosReplanificados = malloc(nodos * sizeof(bool));

	int i;
	for(i=0;i<nodos;i++){
		nodosReplanificados[i] = true;
	}

	log_trace(logger,"Espero respuesta de tareas de Master para job %d",job->id);

	while(redLocalIncompleta){
		respuesta respuestaPlanificacionMaster = desempaquetar(job->socketFd);

		if(respuestaPlanificacionMaster.idMensaje == mensajeTransformacionCompleta){
			bloqueNodo = (bloqueYNodo*) respuestaPlanificacionMaster.envio;
			log_trace(logger,"Finalizada tarea transformacion en bloque %d para job %d", bloqueNodo->bloque,job->id);
			printf("\nFinalizada tarea transformacion en bloque %d para job %d\n", bloqueNodo->bloque,job->id);

			agregarBloqueTerminadoATablaEstados(bloqueNodo->bloque,job->id,TRANSFORMACION);

			if(!faltanBloquesTransformacionParaNodo(job->id,bloqueNodo->workerId)){
				log_trace(logger,"Envio reduccion local de nodo %d para job %d", bloqueNodo->workerId,job->id);
				printf("\nEnvio reduccion local de nodo %d para job %d\n", bloqueNodo->workerId,job->id);
				enviarReduccionLocalAMaster(job,bloqueNodo->workerId);
				actualizarCargasNodosRedLocal(bloqueNodo->workerId);
			}

		}
		else if(respuestaPlanificacionMaster.idMensaje == mensajeFalloTransformacion){
			bloqueNodo = (bloqueYNodo*) respuestaPlanificacionMaster.envio;
			log_trace(logger,"Entro a replanificar se desconecto un worker %d para job %d", bloqueNodo->workerId,job->id);
			printf("\nEntro a replanificar se desconecto un worker %d para job %d\n", bloqueNodo->workerId,job->id);
			if(nodosReplanificados[bloqueNodo->workerId]){
				replanificar(bloqueNodo->workerId,job,respuestaMaster,matrix,bloques,nodos,listaNodos);
				nodosReplanificados[bloqueNodo->workerId] = false;
			}

		}
		else if(respuestaPlanificacionMaster.idMensaje == mensajeRedLocalCompleta){
			numNodo = *(int*) respuestaPlanificacionMaster.envio;
			log_trace(logger,"Finalizada tarea reduccion nodo %d para job %d", numNodo,job->id);
			printf("\nFinalizada tarea reduccion nodo %d para job %d\n", numNodo,job->id);
			agregarBloqueTerminadoATablaEstadosRedLocal(numNodo,job->id,RED_LOCAL);
			redLocalIncompleta= faltanMasTareas(job->id,RED_LOCAL) || faltanMasTareas(job->id,TRANSFORMACION);

		}

		else if(respuestaPlanificacionMaster.idMensaje == mensajeFalloReduccion){
			log_trace(logger,"Fallo en reduccion local del job %d", job->id);
			printf("\nFallo en reduccion local del job %d\n", job->id);
			finalizarJob(job,RED_LOCAL,FALLO_RED_LOCAL);
		}

		else if(respuestaPlanificacionMaster.idMensaje == mensajeDesconexion){
			log_error(logger, "Error en Proceso Master.");
			printf("\nError en Proceso Master\n");
			reestablecerEstadoYama(job);
		}
	}
}
