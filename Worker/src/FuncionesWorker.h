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
 #include <sys/mman.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Globales.h"

#define mb 1048576
#define idWorker 100

/*------VARIABLES-------------*/
struct configuracionNodo config;
int clientSocket;
t_log* logger;


/*----PROTOTIPOS--------------------*/
void handlerMaster(int clientSocket);
void handlerWorker(int clientSocket);
void levantarServidorWorker(char* ip, int port);
void ejecutarTransformacion();
void crearScript(char* bufferScript, int etapa,int numero);
void apareoArchivosLocales();
t_list* crearListaParaReducir();
void ejecutarComando(char* command, int socketMaster);
void traverse_nodes(t_list* list, void funcion(void*));
char *get_line(FILE *fp);
char* crearRutaArchivoAReducir(t_list* listaWorkers);
char* obtenerPathActual();
int conectarseConFS();

#endif /* FUNCIONESWORKER_H_ */
