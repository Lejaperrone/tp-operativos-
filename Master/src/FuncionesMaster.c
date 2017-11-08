#include "FuncionesMaster.h"
#include <sys/mman.h>
#include <Globales.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void conectarseConYama(char* ip, int port) {
	socketYama = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(ip, port);
	conectarCon(direccion, socketYama, 2); //2 id master
	respuesta respuestaHandShake = desempaquetar(socketYama);

	if(respuestaHandShake.idMensaje != mensajeOk){
		log_error(loggerMaster, "Conexion fallida con YAMA");
		perror("Conexion fallida con YAMA");
		log_destroy(loggerMaster);
		exit(1);
	}
	log_trace(loggerMaster, "Conexion con Yama establecida");
}
void crearHilosConexion(respuestaSolicitudTransformacion* rtaYama) {
	pthread_t hiloConexion;
	int i,j;
	//worker =list_get(rtaYama->workers, 0);
	//para probar conexion por hilos
	for(i=0 ; i<list_size(rtaYama->workers);i++){

		workerDesdeYama* worker = list_get(rtaYama->workers, i);
		parametrosTransformacion* parametrosConexion = malloc(sizeof(parametrosTransformacion));

		for(j=0 ; j<list_size(worker->bloquesConSusArchivos);j++){
			bloquesConSusArchivosTransformacion* bloque = list_get(worker->bloquesConSusArchivos, j);
			parametrosConexion->ip.cadena = worker->ip.cadena;
			parametrosConexion->ip.longitud = worker->ip.longitud;
			parametrosConexion->numero = worker->numeroWorker;
			parametrosConexion->puerto = worker->puerto;
			parametrosConexion->bloquesConSusArchivos = *bloque;

			log_trace(loggerMaster, "Me tengo que conectar a %s:%i", parametrosConexion->ip.cadena, parametrosConexion->puerto);

			if (pthread_create(&hiloConexion, NULL, (void *) conectarseConWorkers, parametrosConexion) != 0) {
				log_error(loggerMaster, "No se pudo crear el thread de conexion");
				exit(-1);
			}
		}

	}
}

void* conectarseConWorkers(void* params) {
	parametrosTransformacion* infoTransformacion = malloc(sizeof(parametrosTransformacion));
	respuesta confirmacionWorker;
	bloquesAReplanificar* bloquesAReplanificar = malloc(sizeof(bloquesAReplanificar));
	bloquesAReplanificar->bloques = list_create();

	infoTransformacion = (parametrosTransformacion*)params;
	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(infoTransformacion->ip.cadena,infoTransformacion->puerto);
	conectarCon(direccion, socketWorker, 2); //2 id master

	log_trace(loggerMaster, "Conexion con Worker en %s:%i", infoTransformacion->ip.cadena, infoTransformacion->puerto);

	empaquetar(socketWorker, mensajeProcesarTransformacion, 0, infoTransformacion);

	confirmacionWorker = desempaquetar(socketWorker);

		switch(confirmacionWorker.idMensaje){

		case mensajeOk:
			empaquetar(socketYama, mensajeTransformacionCompleta, 0 , 0);
			break;
		//case mensajeFalloTransformacion:
		case mensajeDesconexion:
			//list_add_all(bloquesAReplanificar->bloques,infoTransformacion->bloquesConSusArchivos);
			bloquesAReplanificar->workerId = infoTransformacion->numero;
			empaquetar(socketYama, mensajeFalloTransformacion, 0 , bloquesAReplanificar);
			break;

		}

	return 0;
}

void enviarJobAYama(job* miJob) {
	empaquetar(socketYama,mensajeSolicitudTransformacion, 0 ,miJob);
	log_trace(loggerMaster,"Enviando solicitud de etapa Transformacion a YAMA");

	respuesta respuestaYama = desempaquetar(socketYama);

	if(respuestaYama.idMensaje != mensajeOk){
		log_error(loggerMaster,"No se pudo iniciar correctamente el job");
		exit(1);
	}

	log_trace(loggerMaster, "Envio correcto de datos a Yama.");

}

void esperarInstruccionesDeYama() {
	respuesta instruccionesYama;//= malloc(sizeof(respuesta));
	respuestaSolicitudTransformacion* infoTransformacion = malloc(sizeof(respuestaSolicitudTransformacion));

	while (1) {
		instruccionesYama = desempaquetar(socketYama);

		switch (instruccionesYama.idMensaje) {

			case mensajeRespuestaTransformacion:
				infoTransformacion = (respuestaSolicitudTransformacion*)instruccionesYama.envio;
				crearHilosConexion(infoTransformacion);
				break;
			case mensajeDesconexion:
				log_error(loggerMaster, "Error inesperado al recibir instrucciones de YAMA.");
				exit(1);
		}
	}
}

char* recibirRuta(char* mensaje) {
	printf("%s\n", mensaje);
	char* comando = malloc(sizeof(char) * 256);
	bzero(comando, 256);
	fgets(comando, 256, stdin);
	string_trim(&comando);
	return comando;
}


job* crearJob(char* argv[]){
	job* nuevo = (job*)malloc(sizeof(job));

	nuevo->id = 0;
	nuevo->socketFd = 0;

	nuevo->rutaTransformador.cadena = string_duplicate(argv[2]);
	nuevo->rutaTransformador.longitud = string_length(nuevo->rutaTransformador.cadena);

	nuevo->rutaReductor.cadena = string_duplicate(argv[3]);
	nuevo->rutaReductor.longitud = string_length(nuevo->rutaReductor.cadena);

	nuevo->rutaDatos.cadena= string_duplicate(argv[4]);
	nuevo->rutaDatos.longitud = string_length(nuevo->rutaDatos.cadena);

	nuevo->rutaResultado.cadena = string_duplicate(argv[5]);
	nuevo->rutaResultado.longitud = string_length(nuevo->rutaResultado.cadena);

	estadisticas = crearEstadisticasProceso();
	return nuevo;
}

/*int dameUnID(){
	return ultimoIdMaster++;//FIXME
}*/

string* contenidoArchivo(char* pathArchivo){
	struct stat fileStat;
	if(stat(pathArchivo,&fileStat) < 0)
		exit(1);
	int fd = open(pathArchivo,O_RDWR);
	string* paquete = malloc(sizeof(string));
	paquete->cadena = mmap(0,fileStat.st_size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	paquete->longitud = fileStat.st_size;

	close(fd);
	return paquete;
}

estadisticaProceso* crearEstadisticasProceso(){
	estadisticaProceso* estadisticas = malloc(sizeof(estadisticaProceso));
	estadisticas->cantImpresiones = 0;
	estadisticas->tiempoInicio= time(NULL);
	estadisticas->cantFallos=0;
	estadisticas->cantTareas=0;
	return estadisticas;
}

void setearFechaTransfromacion(){
	estadisticas->tiempoInicioTrans = time(NULL);
}

void setearFechaReduccionGlobal(){
	estadisticas->tiempoInicioRedGlobal = time(NULL);
}

void setearFechaReduccionLoacl(){
	estadisticas->tiempoInicioRedLocal = time(NULL);
}

void calcularYMostrarEstadisticas(){
	time_t fin= time(NULL);


}
