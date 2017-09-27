/*
 * FuncionesWorker.c
 *
 *  Created on: 24/9/2017
 *      Author: utnso
 */
#include "FuncionesWorker.h"

void esperarConexionesMaster(char* ip, int port){
	socketMaster = levantarServidorWorker(ip,port);//FIXME: CAMBIAR ARCHIVO CONFIGURACION CON IP NODO
	realizarHandshake(socketMaster);
}

void esperarJobDeMaster(){
	respuesta instruccionMaster;
	log_trace(logger,"Esperando instruccion de Master");

	instruccionMaster = desempaquetar(socketMaster);
	switch(instruccionMaster.idMensaje){
	case mensajeEtapaTransformacion:
		ejecutarTransformacion();//LE TIENE QUE LLEGAR EL ARCHIVO DE ORIGEN EL DE DESTINO Y EL SCRIPT A EJECUTAR
		break;
	case mensajeEtapaReduccionLocal:
		break;
	case mensajeEtapaReduccionGlobal:
		break;
	}
	//forkear por cada tarea recibida por el master
}

void ejecutarTransformacion(){//PARA PROBAR
	/*pid_t pid;
	pid = fork();
	if(pid == -1){
		log_error(logger,"Error al crear el hijo");
	}else if(pid == 0){
		//logica del hijo
	}else{
		//logica del padre
	}*/
	system("echo Ejecutando Transformacion | ./script_prueba > /tmp/resultado" );
}
int levantarServidorWorker(char* ip, int port){
	struct sockaddr_in direccionCliente;
	int server;
	fd_set master;
	fd_set read_fds;
	int fdmax;
	int nuevoMaster;
	char buf[256];
	int nbytes;
	int i, j;
	int tamanioDireccion;
	respuesta conexionNueva;
	server = crearServidorAsociado(ip, port);
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_SET(server, &master);

	fdmax = server;

	while (1) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == server) {
					// gestionar nuevas conexiones
					tamanioDireccion = sizeof(direccionCliente);
					if ((nuevoMaster = accept(server,(struct sockaddr *) &direccionCliente, &tamanioDireccion))
							== -1) {
						perror("accept");
					} else {
						FD_SET(nuevoMaster, &master);
						if (nuevoMaster > fdmax) {
							fdmax = nuevoMaster;
						}
						realizarHandshake(nuevoMaster);

						esperarJobDeMaster();
					}
				} else {
					// gestionar datos de un cliente

				}
			}
		}
	}
}

void realizarHandshake(int socket){
	respuesta conexionNueva;
	conexionNueva = desempaquetar(socket);

	if(conexionNueva.idMensaje == 1){
		if(*(int*)conexionNueva.envio == 2){
			log_trace(logger, "Conexion con Master establecida");
		}
	}
}

