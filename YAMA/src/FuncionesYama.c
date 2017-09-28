/*
 * FuncionesYama.c
 *
 *  Created on: 24/9/2017
 *      Author: utnso
 */
#include "FuncionesYama.h"
#include <stdio.h>

int conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 7000);
	conectarCon(direccion, socketFs, 1);
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
	respuestaTransformacion* rtaTransf;

	log_trace(logger, "Conexion de Master\n");
	nuevoJob = desempaquetar(nuevoMaster);
	solicitudTransformacion* solTransf =(solicitudTransformacion*) nuevoJob.envio;
	log_trace(logger, "Me llego %d %d ", solTransf->rutaDatos.longitud,	solTransf->rutaResultado.longitud);

	rtaTransf  = solicitarInformacionAFS(solTransf);
	empaquetar(nuevoMaster, mensajeOk, 0, 0);
	// logica con respuesta a Master
	empaquetar(nuevoMaster, mensajeDesignarWorker, 0, 0);

}

respuestaTransformacion* solicitarInformacionAFS(solicitudTransformacion* solicitud){
	respuestaTransformacion* rtaTransf;
	respuesta respuestaFs;

	empaquetar(socketFs, mensajeSolicitudTransformacion, 0, solicitud);

	respuestaFs = desempaquetar(socketFs);

	if(respuestaFs.idMensaje == mensajeInfoArchivo){
		rtaTransf = (respuestaTransformacion*)respuestaFs.envio;
		log_trace(logger, "Me llego la informacion desde Fs correctamente");
	}
	else{
		log_error(logger, "Error al recibir la informacion del archivo desde FS");
		exit(1);
	}
	return rtaTransf;
}
