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
		break;
	case mensajeEtapaReduccionLocal:
		break;
	case mensajeEtapaReduccionGlobal:
		break;
	}
	//forkear por cada tarea recibida por el master
}
int levantarServidorWorker(char* ip, int port){
	struct sockaddr_in direccionCliente;
	int tamanioDireccion = sizeof(direccionCliente);
	int server = crearServidorAsociado(ip,port);
	return accept(server, (void*) &direccionCliente, &tamanioDireccion);


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

