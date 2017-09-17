/*
 * FuncionesDN.c
 *
 *  Created on: 12/9/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "FuncionesDN.h"
#include "Serializacion.h"
#include "Sockets.h"

void enviarBloqueAFS(int numeroBloque){

}

void setearBloque(int numeroBloque, void* datos){

}

void conectarseConFs(){
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion("127.0.0.1", 7000);
	conectarCon(direccion, socketFs, 3);
}
