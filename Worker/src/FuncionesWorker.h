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
#include "Globales.h"

#define mb 1048576

/*------VARIABLES-------------*/
struct configuracionNodo config;
int socketMaster;
t_log* logger;


/*----PROTOTIPOS--------------------*/
void handlerMaster();
void handlerWorker();
void levantarServidorWorker(char* ip, int port);
void ejecutarTransformacion();
void crearScript(char * bufferScript);
void apareoArchivosLocales();
t_list* crearListaParaReducir();
void ejecutarComando(char* command, int socketMaster);
void testWorker();

#endif /* FUNCIONESWORKER_H_ */
