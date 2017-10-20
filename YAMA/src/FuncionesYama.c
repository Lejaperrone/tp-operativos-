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

int disponibilidadBase;

int conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(config.FS_IP, config.FS_PUERTO);
	conectarCon(direccion, socketFs, 1);
	desempaquetar(socketFs);
	log_trace(logger, "Conexion exitosa con File System");
	return socketFs;
}

void levantarServidorYama(char* ip, int port) {
	respuesta conexionNueva;
	servidor = crearServidorAsociado(ip, port);
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// añadir listener al conjunto maestro
	FD_SET(servidor, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = servidor; // por ahora es éste
	// bucle principal
	while (1) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == servidor) {
					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((nuevoMaster = accept(servidor,
							(struct sockaddr *) &direccionCliente, &addrlen))
							== -1) {
						perror("accept");
					} else {
						FD_SET(nuevoMaster, &master); // añadir al conjunto maestro
						if (nuevoMaster > fdmax) {    // actualizar el máximo
							fdmax = nuevoMaster;
						}
						conexionNueva = desempaquetar(nuevoMaster);
						int idRecibido = *(int*) conexionNueva.envio;

						switch (idRecibido) {    //HANDSHAKE
						case idMaster:
							recibirContenidoMaster();
							break;

						}
					}
				} else {
					// gestionar datos de un cliente

				}
			}
		}
	}

}

void recibirContenidoMaster() {
	respuesta nuevoJob;

	iniciarListasPlanificacion();

	log_trace(logger, "Conexion de Master");
	nuevoJob = desempaquetar(nuevoMaster);
	job* jobAPlanificar =(job*) nuevoJob.envio;

	pthread_mutex_lock(&cantJobs_mutex);
	cantJobs++;
	pthread_mutex_unlock(&cantJobs_mutex);
	jobAPlanificar->id = cantJobs;
	jobAPlanificar->socketFd = nuevoMaster;
	log_trace(logger, "Job recibido pre-planificacion %i",jobAPlanificar->id);

	planificar(jobAPlanificar);

	empaquetar(nuevoMaster, mensajeOk, 0, 0);// logica con respuesta a Master
	empaquetar(nuevoMaster, mensajeDesignarWorker, 0, 0);

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

void recibirArchivo(){
	respuesta paquete;

	paquete = desempaquetar(nuevoMaster);
	string* archivo = (string*) paquete.envio;
	char* hola = archivo->cadena;

	printf("%s\n", hola);
}

char* dameUnNombreArchivoTemporal(){
	char* nombre;// = string_new();
	//string_from_format("Master-%i-temp%i",infoJob->id, inforBloque->bloque);
	return nombre;
}
void inicializarEstructuras(){
	pthread_mutex_init(&cantJobs_mutex, NULL);
}

bool** llenarMatrizNodosBloques(informacionArchivoFsYama* infoArchivo,int nodos,int bloques){
	bool** matriz = (bool**)malloc(nodos*sizeof(bool));

	int j,k;

	for(j=0;j<nodos;j++){
		matriz[j] = (bool*)malloc( bloques*sizeof(bool));
		for(k=0;k<bloques;k++){
			matriz[j][k]=false;
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

void calcularNodosYBloques(informacionArchivoFsYama* info,int* nodos){
	*nodos = list_size(info->informacionBloques);

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

void agregarBloqueANodo(t_list* listaNodos, ubicacionBloque ubicacion,int bloque){

	bool funcionFind(void *nodo) {
		return(((infoNodo*)nodo)->numero == ubicacion.numeroNodo);
	}

	if (list_find(listaNodos,funcionFind)) {
		infoNodo* nodo = (infoNodo*)list_find(listaNodos,funcionFind);
		list_add(nodo->bloques,&bloque);
	}
	else{
		infoNodo* nuevoNodo = malloc(sizeof(infoNodo));
		nuevoNodo->activo = true;
		list_add(nuevoNodo->bloques,&bloque);
		nuevoNodo->carga=0;
		nuevoNodo->ip.cadena = strdup(ubicacion.ip.cadena);
		nuevoNodo->ip.longitud = ubicacion.ip.longitud;
		nuevoNodo->numero = ubicacion.numeroNodo;
		nuevoNodo->puerto = ubicacion.puerto;
		list_add(listaNodos,nuevoNodo);
	}
}
