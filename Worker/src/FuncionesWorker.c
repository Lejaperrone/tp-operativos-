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

}
int levantarServidorWorker(char* ip, int port){
	unsigned int tamanioDireccion;
	int server = crearServidorAsociado(ip,port);
	return accept(server, (void*) &direccionCliente, &tamanioDireccion);

}

void realizarHandshake(int socket){
	respuesta conexion;
	conexion = desempaquetar(socketMaster);
	if (conexion.idMensaje == 0){ //que sea mensaje handshake
		int idProceso = *(int*) conexion.envio;
		if (idProceso == 2){//HANDSHAKE, RECONOCIA A MASTER con id master =2
			log_trace(logger, "Conexion de Master establecida\n");
			//logica con el master
		}

		close(socket);

	}
}

