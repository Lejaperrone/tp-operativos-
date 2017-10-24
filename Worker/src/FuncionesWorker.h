#ifndef FUNCIONESWORKER_H_
#define FUNCIONESWORKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <Sockets.h>
#include <Configuracion.h>
#include <commons/log.h>
#include "Serializacion.h"
#include <commons/log.h>
#include "FuncionesWorker.h"
#include <commons/string.h>
#include <sys/stat.h>

/*------VARIABLES-------------*/
struct configuracionNodo config;
int socketMaster;
t_log* logger;

/*----PROTOTIPOS--------------------*/
void esperarConexionesMaster(char* ip, int port);
void esperarJobDeMaster();
void levantarServidorWorker(char* ip, int port);
void realizarHandshake(int socket);
void ejecutarTransformacion();
FILE* crearScript(char * bufferScript, int etapa);

#endif /* FUNCIONESWORKER_H_ */
