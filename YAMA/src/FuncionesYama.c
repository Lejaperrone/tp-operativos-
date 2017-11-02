/*
 * FuncionesYama.c
 *
 *  Created on: 24/9/2017
 *      Author: utnso
 */
#include "FuncionesYama.h"
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

int disponibilidadBase;

int conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(config.FS_IP, config.FS_PUERTO);
	conectarCon(direccion, socketFs, 1);
	desempaquetar(socketFs);
	log_trace(logger, "Conexion exitosa con File System");
	return socketFs;
}

void *manejarConexionMaster(void *cliente) {
	int nuevoMaster;
	memcpy(&nuevoMaster,cliente,sizeof(int));

	recibirContenidoMaster(nuevoMaster);
	return 0;
}

void manejarConfig(){
	while (1) {
		verCambiosConfig();
	}
}

void levantarServidorYama(char* ip, int port){
	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(direccionCliente);
	respuesta respuestaId;
	void* cliente;

	servidor = crearServidorAsociado(ip, port);

	while (1) {
		int nuevoCliente;
		if ((nuevoCliente = accept(servidor,(struct sockaddr *) &direccionCliente, &tamanioDireccion))!= -1) {

			pthread_t nuevoHilo;
			pthread_attr_t attr;

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			respuestaId = desempaquetar(nuevoCliente);
			int id = *(int*)respuestaId.envio;

			if(id == idMaster){
				pthread_mutex_lock(&mutexLog);
				log_trace(logger, "Nueva conexion de un Master\n");
				pthread_mutex_unlock(&mutexLog);

				empaquetar(nuevoCliente,mensajeOk,0,0);

				cliente= malloc(sizeof(int));
				memcpy(cliente,&nuevoCliente,sizeof(int));

				if (pthread_create(&nuevoHilo, &attr, &manejarConexionMaster,cliente) == -1) {
					log_error(logger, "could not create thread");
					perror("could not create thread");
					log_destroy(logger);
					exit(1);
				}
			}
			else{
				pthread_mutex_lock(&mutexLog);
				log_trace(logger, "Conexion invalida\n");
				pthread_mutex_unlock(&mutexLog);
			}
		}
	}
}

void verCambiosConfig(){
	char *pathCarpetaConfig = string_new();
	char  cwdCarpeta[1024];

	string_append(&pathCarpetaConfig, getcwd(cwdCarpeta, sizeof(cwdCarpeta)));

	char buffer[BUF_LEN];

	int file_descriptor = inotify_init();

	if (file_descriptor < 0) {
		log_error(logger, "<inotify> No se pudo iniciar.");
	}

	int watch_descriptor = inotify_add_watch(file_descriptor, pathCarpetaConfig, IN_MODIFY | IN_DELETE);

	int length = read(file_descriptor, buffer, BUF_LEN);

	if (length < 0) {
		log_error(logger, "<inotify> No se pudo leer archivo de configuracion.");
	}

	int offset = 0;

	while (offset < length) {
		struct inotify_event *event = (struct inotify_event *)&buffer[offset];

		if (event->len) {
			if (strcmp(event->name, "YAMA.cfg") == 0) {
				if (event->mask & IN_DELETE) {
					if (event->mask & IN_ISDIR) {
						log_error(logger, "<inotify> El directorio %s ha sido eliminado.", event->name);
					}
					else {
						log_error(logger, "<inotify> El archivo %s ha sido eliminado.", event->name);
					}
				}
				else if (event->mask & IN_MODIFY) {
					if (event->mask & IN_ISDIR) {
						log_info(logger, "<inotify> El directorio %s ha sido modificado.", event->name);
					}
					else {
						log_info(logger, "<inotify> El archivo %s ha sido modificado.", event->name);
						validarCambiosConfig();
					}
	            }
	         }
	      }
	      offset += sizeof(struct inotify_event) + event->len;
	   }

	   inotify_rm_watch(file_descriptor, watch_descriptor);
	   close(file_descriptor);

}

void validarCambiosConfig(){
	int nuevoRetardo,nuevaDisponibilidad;
	char* nuevoAlgoritmo;

	char *pathArchConfig = string_new(); // String que va a tener el path absoluto para pasarle al config_create

	char cwd[1024];                      // Variable donde voy a guardar el path absoluto hasta el /Debug

	string_append(&pathArchConfig, getcwd(cwd, sizeof(cwd)));

	string_append(&pathArchConfig, "/YAMA.cfg");     // Le concateno el archivo de configuración

	t_config *archivoConfig = config_create(pathArchConfig);

	if (archivoConfig == NULL) {
		log_info(logger, "<inotify> config_create retorno NULL.");
		return;
	}

	if (config_has_property(archivoConfig, "RETARDO_PLANIFICACION")) {
		nuevoRetardo= config_get_int_value(archivoConfig, "RETARDO_PLANIFICACION");
	}
	else{
		log_info(logger, "<inotify> RETARDO PLANIFICACION retorno NULL.");
		return;
	}

	if (config_has_property(archivoConfig, "DISPONIBILIDAD_BASE")) {
		nuevaDisponibilidad= config_get_int_value(archivoConfig, "DISPONIBILIDAD_BASE");
	}
	else{
		log_info(logger, "<inotify> DISPONIBILIDAD_BASE retorno NULL.");
		return;
	}

	if (config_has_property(archivoConfig, "ALGORITMO_BALANCEO")) {
		nuevoAlgoritmo= config_get_string_value(archivoConfig, "ALGORITMO_BALANCEO");
	}
	else{
		log_info(logger, "<inotify> ALGORITMO_BALANCEO retorno NULL.");
		return;
	}

	//config_destroy(archivoConfig);

	if (config.RETARDO_PLANIFICACION != nuevoRetardo) {
		log_info(logger, "<inotify> RETARDO_PLANIFICACION modificado. Anterior: %d || Actual: %d", config.RETARDO_PLANIFICACION , nuevoRetardo);

		if (nuevoRetardo<= 0) {
			log_error(logger, "El RETARDO_PLANIFICACION no puede ser < = 0. Se deja el anterior: %d.", config.RETARDO_PLANIFICACION);
		}
		else{
			log_trace(logger,"[Inotify] RETARDO_PLANIFICACION modificado. Anterior: %d || Actual: %d\n", config.RETARDO_PLANIFICACION, nuevoRetardo);
			config.RETARDO_PLANIFICACION = nuevoRetardo;
		}
	}

	if (config.DISPONIBILIDAD_BASE != nuevaDisponibilidad) {
		log_info(logger, "<inotify> DISPONIBILIDAD_BASE modificada. Anterior: %d || Actual: %d", config.DISPONIBILIDAD_BASE , nuevaDisponibilidad);

		if (nuevoRetardo<= 0) {
			log_error(logger, "La DISPONIBILIDAD_BASE no puede ser < = 0. Se deja la anterior: %d.", config.DISPONIBILIDAD_BASE);
		}
		else{
			log_trace(logger,"[Inotify] DISPONIBILIDAD_BASE modificada. Anterior: %d || Actual: %d\n", config.DISPONIBILIDAD_BASE, nuevaDisponibilidad);
			config.DISPONIBILIDAD_BASE = nuevoRetardo;
		}
	}

	if (strcmp(config.ALGORITMO_BALANCEO ,nuevoAlgoritmo)) {
		log_info(logger, "<inotify> ALGORITMO_BALANCEO modificada. Anterior: %s || Actual: %s", config.ALGORITMO_BALANCEO , nuevoAlgoritmo);

		if (!strcmp(nuevoAlgoritmo,"WCLOCK") || !strcmp(nuevoAlgoritmo,"CLOCK")) {
			log_error(logger, "El ALGORITMO_BALANCEO no puede ser diferente a CLOCK o WCLOCK. Se deja el anterior: %s.", config.ALGORITMO_BALANCEO);
		}
		else{
			log_trace(logger,"[Inotify] ALGORITMO_BALANCEO modificada. Anterior: %s || Actual: %s\n", config.ALGORITMO_BALANCEO, nuevoAlgoritmo);
			config.ALGORITMO_BALANCEO = strdup(nuevoAlgoritmo);
		}
	}
	config_destroy(archivoConfig);
}

void recibirContenidoMaster(int nuevoMaster) {
	respuesta nuevoJob;

	iniciarListasPlanificacion();

	nuevoJob = desempaquetar(nuevoMaster);
	job* jobAPlanificar =(job*) nuevoJob.envio;

	pthread_mutex_lock(&cantJobs_mutex);
	cantJobs++;
	pthread_mutex_unlock(&cantJobs_mutex);
	jobAPlanificar->id = cantJobs;
	jobAPlanificar->socketFd = nuevoMaster;
	log_trace(logger, "Job recibido para pre-planificacion %i",jobAPlanificar->id);

	empaquetar(nuevoMaster, mensajeOk, 0,0);

	planificar(jobAPlanificar);


}

informacionArchivoFsYama* solicitarInformacionAFS(solicitudInfoNodos* solicitud){
	informacionArchivoFsYama* rtaFs = malloc(sizeof(informacionArchivoFsYama));
	respuesta respuestaFs;

	empaquetar(socketFs, mensajeSolicitudInfoNodos, 0, solicitud);

	respuestaFs = desempaquetar(socketFs);

	if(respuestaFs.idMensaje == mensajeRespuestaInfoNodos){
		rtaFs = (informacionArchivoFsYama*)respuestaFs.envio;
		log_trace(logger, "Me llego la informacion desde Fs correctamente");
	}
	else{
		log_error(logger, "Error al recibir la informacion del archivo desde FS");
		exit(1);
	}
	return rtaFs;
}

int getDisponibilidadBase(){
	return config.DISPONIBILIDAD_BASE;
}

int esClock(){
	return strcmp("CLOCK" ,config.ALGORITMO_BALANCEO);
}

void recibirArchivo(int nuevoMaster){
	respuesta paquete;

	paquete = desempaquetar(nuevoMaster);
	string* archivo = (string*) paquete.envio;
	char* hola = archivo->cadena;

	printf("%s\n", hola);
}

char* dameUnNombreArchivoTemporal(int jobId,int numBloque,int etapa,int nodo){
	char* nombre= string_from_format("j%dn%db%de%d",jobId, nodo,numBloque,etapa);
	return nombre;
}
void inicializarEstructuras(){
	pthread_mutex_init(&cantJobs_mutex, NULL);
}

bool** llenarMatrizNodosBloques(informacionArchivoFsYama* infoArchivo,int nodos,int bloques){
	bool** matriz = (bool**)malloc((nodos+1)*sizeof(bool*));

	int j,k;
	for(j=0;j<=nodos;j++){
		matriz[j]= malloc(bloques * sizeof(bool));
		for(k=0;k<bloques;k++){
			matriz[j][k] = false;
		}
	}

	int i;
	for(i=0;i< list_size(infoArchivo->informacionBloques);i++){
		infoBloque* info = list_get(infoArchivo->informacionBloques,i);
		matriz[info->ubicacionCopia0.numeroNodo][info->numeroBloque] = true;
		matriz[info->ubicacionCopia1.numeroNodo][info->numeroBloque] = true;
	}
	return matriz;
}

void calcularNodosYBloques(informacionArchivoFsYama* info,int* nodos,int*bloques){
	*bloques = list_size(info->informacionBloques);

	int max =0;
	int i;
	for(i=0;i<list_size(info->informacionBloques);i++){
		infoBloque* infoBlo = list_get(info->informacionBloques,i);
		if(infoBlo->ubicacionCopia0.numeroNodo > max){
			max = infoBlo->ubicacionCopia0.numeroNodo;
		}
		if(infoBlo->ubicacionCopia1.numeroNodo > max){
			max = infoBlo->ubicacionCopia1.numeroNodo;
		}
	}
	*nodos = max;
}

void llenarListaNodos(t_list* listaNodos,informacionArchivoFsYama* infoArchivo){
	int i;
	for(i=0;i<list_size(infoArchivo->informacionBloques);i++){
		infoBloque* infoBlo = list_get(infoArchivo->informacionBloques,i);
		agregarBloqueANodo(listaNodos,infoBlo->ubicacionCopia0,infoBlo->numeroBloque);
		agregarBloqueANodo(listaNodos,infoBlo->ubicacionCopia1,infoBlo->numeroBloque);
	}
}

void agregarBloqueANodo(t_list* listaNodos,ubicacionBloque ubicacion,int bloque){
	infoNodo* nodoAPlanificar = malloc(sizeof(infoNodo));

	bool funcionFind(void *nodo) {
		return(((infoNodo*)nodo)->numero == ubicacion.numeroNodo);
	}

	pthread_mutex_lock(&mutex_NodosConectados);

	if(list_find(listaNodos,funcionFind)){
		infoNodo* nodo = (infoNodo*)list_find(listaNodos,funcionFind);
		list_add(nodo->bloques,&bloque);
	}
	else if (list_find(nodosConectados,funcionFind)) {
		infoNodo* nodo = (infoNodo*)list_find(nodosConectados,funcionFind);
		memcpy(nodoAPlanificar,nodo,sizeof(infoNodo));
		list_add(nodoAPlanificar->bloques,&bloque);
		list_add(listaNodos,nodoAPlanificar);
	}
	else{
		infoNodo* nuevoNodo = malloc(sizeof(infoNodo));
		nuevoNodo->carga=0;
		nuevoNodo->cantTareasHistoricas=0;
		nuevoNodo->activo = true;
		nuevoNodo->ip.cadena = strdup(ubicacion.ip.cadena);
		nuevoNodo->ip.longitud = ubicacion.ip.longitud;
		nuevoNodo->numero = ubicacion.numeroNodo;
		nuevoNodo->puerto = ubicacion.puerto;
		list_add(nodosConectados,nuevoNodo);

		memcpy(nodoAPlanificar,nuevoNodo,sizeof(infoNodo));
		nodoAPlanificar->bloques = list_create();
		list_add(nodoAPlanificar->bloques,&bloque);
		list_add(listaNodos,nodoAPlanificar);

	}
	pthread_mutex_unlock(&mutex_NodosConectados);

}

void agregarBloqueTerminadoATablaEstados(int bloque,int jobId,int etapa){
	bool encontrarEnTablaEstados(void *registro) {
		registroTablaEstados* reg =(registroTablaEstados*)registro;
		return reg->job == jobId && reg->etapa == etapa && reg->etapa == etapa && reg->estado == EN_EJECUCION;
	}

	pthread_mutex_lock(&mutexTablaEstados);
	registroTablaEstados* reg = list_find(tablaDeEstados,(void*)faltanMasTareas);
	pthread_mutex_lock(&mutexTablaEstados);

	reg->estado=FINALIZADO_OK;
}

bool faltanMasTareas(int jobid,int etapa){
	bool encontrarEnTablaEstados(void *registro) {
		registroTablaEstados* reg =(registroTablaEstados*)registro;
		return reg->job == jobid && reg->etapa== etapa;
	}

	pthread_mutex_lock(&mutexTablaEstados);
	registroTablaEstados* reg = list_find(tablaDeEstados,(void*)faltanMasTareas);
	pthread_mutex_lock(&mutexTablaEstados);

	return reg;
}

void finalizarJob(job* job){
	borrarEntradasDeJob(job->id);
	empaquetar(job->socketFd,mensajeFinJob,0,0);
}

void borrarEntradasDeJob(int jobid){
	bool encontrarEnTablaEstados(void *registro) {
		return(((registroTablaEstados*)registro)->job== jobid);
	}

	pthread_mutex_lock(&mutexTablaEstados);
	while(list_find(tablaDeEstados,(void*)encontrarEnTablaEstados)){
		list_remove_by_condition(tablaDeEstados,(void*)borrarEntradasDeJob);
	}
	pthread_mutex_unlock(&mutexTablaEstados);

}

void esperarRespuestaReduccionDeMaster(job* job,int etapa){
	int mensajeOkRedu,mensajeFalloRedu;

	if(etapa ==  RED_LOCAL){
		mensajeOkRedu=mensajeRedLocalComlpleta;
		mensajeFalloRedu=mensajeFalloRedLocal;
	}
	else{
		mensajeOkRedu=mensajeRedGlobalComlpleta;
		mensajeFalloRedu=mensajeFalloRedGlobal;
	}


	int bloque;
	bool reduccionIncompleta = true;
	while(reduccionIncompleta){
		respuesta respuestaMaster=  desempaquetar(job->socketFd);

		if(respuestaMaster.idMensaje == mensajeOkRedu){
			bloque = (int)respuestaMaster.envio;
			agregarBloqueTerminadoATablaEstados(bloque,job->id,etapa);
			reduccionIncompleta = faltanMasTareas(job->id,etapa);
		}
		else if(respuestaMaster.idMensaje == mensajeFalloRedu){
			finalizarJob(job);
		}
	}
}

infoNodo* obtenerNodo(int numero){
	bool encontrarNodo(void *nodo){
		return ((infoNodo*)nodo)->numero == numero;
	}

	return list_find(nodosConectados,(void*)encontrarNodo);
}
