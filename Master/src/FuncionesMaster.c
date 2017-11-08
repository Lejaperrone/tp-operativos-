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
	int i;

	for(i=0 ; i<list_size(rtaYama->workers);i++){
		workerDesdeYama* worker = list_get(rtaYama->workers, i);
		crearHilosPorBloque(worker);
	}
}


void crearHilosPorBloque(workerDesdeYama* worker){
	parametrosTransformacion* parametrosConexion = malloc(sizeof(parametrosTransformacion));

	parametrosConexion->ip.cadena = worker->ip.cadena;
	parametrosConexion->ip.longitud = worker->ip.longitud;
	parametrosConexion->numero = worker->numeroWorker;
	parametrosConexion->puerto = worker->puerto;

	int j;
	for(j=0 ; j<list_size(worker->bloquesConSusArchivos);j++){
		pthread_t hiloConexion;
		bloquesConSusArchivosTransformacion* bloque = list_get(worker->bloquesConSusArchivos, j);
		parametrosConexion->bloquesConSusArchivos = *bloque;

		if (pthread_create(&hiloConexion, NULL, (void *) conectarseConWorkers, parametrosConexion) != 0) {
			log_error(loggerMaster, "No se pudo crear el thread de conexion");
			exit(-1);
		}
		pthread_join(hiloConexion, NULL);
	}
}

void* conectarseConWorkers(void* params) {
	parametrosTransformacion* infoTransformacion= (parametrosTransformacion*)params;
	respuesta confirmacionWorker;
	bloqueAReplanificar* bloqueReplanificar=malloc(sizeof(bloqueAReplanificar));
	int numeroBloque;

	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(infoTransformacion->ip.cadena,infoTransformacion->puerto);
	conectarCon(direccion, socketWorker, 2); //2 id master

	log_trace(loggerMaster, "Conexion con Worker en %s:%i", infoTransformacion->ip.cadena, infoTransformacion->puerto);

	struct stat fileStat;
	if(stat(miJob->rutaTransformador.cadena,&fileStat) < 0){
		printf("No se pudo abrir el archivo\n");
		return 0;
	}

	int fd = open(miJob->rutaTransformador.cadena,O_RDWR);
	int size = fileStat.st_size;

	infoTransformacion->contenidoScript.cadena = mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	infoTransformacion->contenidoScript.longitud = size;

	empaquetar(socketWorker, mensajeProcesarTransformacion, 0, infoTransformacion);

	confirmacionWorker = desempaquetar(socketWorker);

	if (munmap(infoTransformacion->contenidoScript.cadena, infoTransformacion->contenidoScript.longitud) == -1){
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);

	switch(confirmacionWorker.idMensaje){

		case mensajeTransformacionCompleta:
			numeroBloque = *(int*)confirmacionWorker.envio;
			printf("enviado a yama%d\n",numeroBloque);
			empaquetar(socketYama, mensajeTransformacionCompleta, 0 , &numeroBloque);
			break;
		//case mensajeFalloTransformacion:
		case mensajeDesconexion:
			bloqueReplanificar->workerId = infoTransformacion->numero;
			bloqueReplanificar->bloque = infoTransformacion->bloquesConSusArchivos.numBloque;
			empaquetar(socketYama, mensajeFalloTransformacion, 0 , bloqueReplanificar);
			printf("desconccceecion\n\n");
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
	workerDesdeYama* worker;

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

			case mensajeReplanificacion:
				worker= instruccionesYama.envio;
				crearHilosPorBloque(worker);
				printf("soy un capo y replanifique\n");

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

