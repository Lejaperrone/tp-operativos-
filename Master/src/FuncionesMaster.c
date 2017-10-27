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
void crearHilosConexion() {
	pthread_t hiloConexion;
	parametrosConexionMaster* parametrosConexion = malloc(sizeof(parametrosConexionMaster));
	//para probar conexion por hilos
	parametrosConexion->ip = "127.0.0.1";
	parametrosConexion->id = 2;
	parametrosConexion->port = 5000;
	if (pthread_create(&hiloConexion, NULL, (void *) conectarseConWorkers,parametrosConexion) != 0) {
		log_error(loggerMaster, "No se pudo crear el thread de conexion");
		exit(-1);
	}
}
void* conectarseConWorkers(parametrosConexionMaster* parametrosConexion) {
	procesarTransformacion solicitud;
	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(parametrosConexion->ip,parametrosConexion->port);
	conectarCon(direccion, socketWorker, parametrosConexion->id); //2 id master
	log_trace(loggerMaster, "Conexion con Worker \n");

	//empaquetar(socketWorker,mensajeProcesarTransformacion, 0,0);
	//empaquetar la solicitud de procesamiento
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
	while (1) {
		instruccionesYama = desempaquetar(socketYama);

		switch (instruccionesYama.idMensaje) {

		case mensajeRespuestaTransformacion:
			//ACA RECIBE LO QUE YAMA PLANIFICA Y SE CONECTA A LOS WORKERS SELECCIONADOS
			crearHilosConexion();
			break;
		case mensajeDesconexion:
			log_error(loggerMaster, "Error inesperado al recibir instrucciones de YAMA.");
			exit(1);
		}
	}
//recibir de yama todos los parametros conexion IP PUERTO
//ver que tipo de etapa es, los hilos de cada etapa son DISTINTOS!!!
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
	nuevo->socketFd = NULL;

	nuevo->rutaTransformador.cadena = string_duplicate(argv[2]);
	nuevo->rutaTransformador.longitud = string_length(nuevo->rutaTransformador.cadena);

	nuevo->rutaReductor.cadena = string_duplicate(argv[3]);
	nuevo->rutaReductor.longitud = string_length(nuevo->rutaReductor.cadena);

	nuevo->rutaDatos.cadena= string_duplicate(argv[4]);
	nuevo->rutaDatos.longitud = string_length(nuevo->rutaDatos.cadena);

	nuevo->rutaResultado.cadena = string_duplicate(argv[5]);
	nuevo->rutaResultado.longitud = string_length(nuevo->rutaResultado.cadena);

	return nuevo;
}

/*int dameUnID(){
	return ultimoIdMaster++;//FIXME
}*/

void enviarArchivo(int socketPrograma, char* pathArchivo){
	struct stat fileStat;
	if(stat(pathArchivo,&fileStat) < 0)
		exit(1);
	int fd = open(pathArchivo,O_RDWR);
	string* paquete = malloc(sizeof(string));
	paquete->cadena = mmap(0,fileStat.st_size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	paquete->longitud = fileStat.st_size;
	empaquetar(socketPrograma, mensajeArchivo, fileStat.st_size, paquete);

	if (munmap(paquete->cadena,paquete->longitud) == 0)
				printf("%s\n", "todo joya");
			else
				printf("%s\n", "todo no joya");
	close(fd);
	free(paquete);
}
