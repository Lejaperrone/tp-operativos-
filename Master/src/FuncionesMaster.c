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

void crearHilosConexionTransformacion(respuestaSolicitudTransformacion* rtaYama) {
	int i;

	for(i=0 ; i<list_size(rtaYama->workers);i++){
		workerDesdeYama* worker = list_get(rtaYama->workers, i);
		crearHilosPorBloqueTransformacion(worker);
	}
}

void crearHilosPorBloqueTransformacion(workerDesdeYama* worker){
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

		if (pthread_create(&hiloConexion, NULL, (void *) conectarseConWorkersTransformacion, parametrosConexion) != 0) {
			log_error(loggerMaster, "No se pudo crear el thread de conexion");
			exit(-1);
		}
		pthread_join(hiloConexion, NULL);
	}
}

void* conectarseConWorkersTransformacion(void* params) {
	parametrosTransformacion* infoTransformacion= (parametrosTransformacion*)params;
	respuesta confirmacionWorker;
	bloqueYNodo* bloqueOK;

	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(infoTransformacion->ip.cadena,infoTransformacion->puerto);
	if(!conectarCon(direccion, socketWorker, 2)){//2 id master
		mandarAReplanificar(infoTransformacion);
		return 0;

	}

	log_trace(loggerMaster, "Conexion con Worker %d para bloque %d", infoTransformacion->numero, infoTransformacion->bloquesConSusArchivos.numBloque);

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
			bloqueOK = malloc(sizeof(bloqueYNodo));
			bloqueOK->workerId = infoTransformacion->numero;
			bloqueOK->bloque = infoTransformacion->bloquesConSusArchivos.numBloque;
			empaquetar(socketYama, mensajeTransformacionCompleta, 0 , bloqueOK);
			free(bloqueOK);
			break;

		case mensajeDesconexion:
			mandarAReplanificar(infoTransformacion);
			break;


	}
	return 0;
}
void crearHilosConexionRedLocal(respuestaReduccionLocal* rtaYama){
	int i;

		for(i=0 ; i<list_size(rtaYama->workers);i++){
			workerDesdeYama* worker = list_get(rtaYama->workers, i);
			crearHilosPorTmpRedLocal(worker);
		}

}

void crearHilosPorTmpRedLocal(workerDesdeYama* worker){
	parametrosReduccionLocal* parametrosConexion = malloc(sizeof(parametrosReduccionLocal));
	int j;

	parametrosConexion->ip.cadena = worker->ip.cadena;
	parametrosConexion->ip.longitud = worker->ip.longitud;
	parametrosConexion->numero = worker->numeroWorker;
	parametrosConexion->puerto = worker->puerto;
	parametrosConexion->archivosTemporales = list_create();

	pthread_t hiloConexion;
	for(j=0 ; j<list_size(worker->bloquesConSusArchivos);j++){
		bloquesConSusArchivosRedLocal* bloque = list_get(worker->bloquesConSusArchivos, j);
		parametrosConexion->rutaDestino = bloque->archivoReduccion;

		string* ruta = malloc(sizeof(string));
		ruta->cadena = strdup(bloque->archivoTransformacion.cadena);
		ruta->longitud = bloque->archivoTransformacion.longitud;

		list_add(parametrosConexion->archivosTemporales,ruta);
	}
	if (pthread_create(&hiloConexion, NULL, (void*)conectarseConWorkersRedLocal, parametrosConexion) != 0) {
		log_error(loggerMaster, "No se pudo crear el thread de conexion");
		exit(-1);
	}
	pthread_join(hiloConexion, NULL);

}

void* conectarseConWorkersRedLocal(void* params){
	parametrosReduccionLocal* infoRedLocal= (parametrosReduccionLocal*)params;
	respuesta confirmacionWorker;
	int numeroBloque;

	int socketWorker = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(infoRedLocal->ip.cadena,infoRedLocal->puerto);
	if(!conectarCon(direccion, socketWorker, 2)){//2 id master
		mandarFalloEnReduccion();
		return 0;

	}

	log_trace(loggerMaster, "Conexion con Worker %d para estos tmp %d", infoRedLocal->numero, list_size(infoRedLocal->archivosTemporales));

	struct stat fileStat;
	if(stat(miJob->rutaReductor.cadena,&fileStat) < 0){
		printf("No se pudo abrir el archivo\n");
		return 0;
	}

	int fd = open(miJob->rutaReductor.cadena,O_RDWR);
	int size = fileStat.st_size;

	infoRedLocal->contenidoScript.cadena = mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);
	infoRedLocal->contenidoScript.longitud = size;

	empaquetar(socketWorker, mensajeProcesarRedLocal, 0, infoRedLocal);

	confirmacionWorker = desempaquetar(socketWorker);

	if (munmap(infoRedLocal->contenidoScript.cadena, infoRedLocal->contenidoScript.longitud) == -1){
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);

	switch(confirmacionWorker.idMensaje){

	case mensajeRedLocalCompleta:
		empaquetar(socketYama, mensajeRedLocalCompleta, 0 , &numeroBloque);
		finalizarTiempo(estadisticas->tiempoFinRedLocal,numeroBloque);
		break;

	case mensajeDesconexion:
		mandarFalloEnReduccion();
		break;


	}
	return 0;

}

void mandarFalloEnReduccion(){
	empaquetar(socketYama,mensajeFalloReduccion,0,0);
}

void mandarAReplanificar(parametrosTransformacion* infoTransformacion){
	bloqueYNodo* bloqueReplanificar=malloc(sizeof(bloqueYNodo));
	bloqueReplanificar->workerId = infoTransformacion->numero;
	bloqueReplanificar->bloque = infoTransformacion->bloquesConSusArchivos.numBloque;
	empaquetar(socketYama, mensajeFalloTransformacion, 0 , bloqueReplanificar);
	estadisticas->cantFallos++;
	estadisticas->cantTareas[0]--;
	free(bloqueReplanificar);
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
	respuestaReduccionLocal* infoRedLocal = malloc(sizeof(respuestaReduccionLocal));
	workerDesdeYama* worker;

	while (1) {
		instruccionesYama = desempaquetar(socketYama);

		switch (instruccionesYama.idMensaje) {

			case mensajeRespuestaTransformacion:
				infoTransformacion = (respuestaSolicitudTransformacion*)instruccionesYama.envio;
				inicializarTiemposTransformacion(infoTransformacion);
				crearHilosConexionTransformacion(infoTransformacion);
				break;
			case mensajeRespuestaRedLocal:
				infoRedLocal = (respuestaReduccionLocal*)instruccionesYama.envio;
				crearHilosConexionRedLocal(infoRedLocal);
				break;
			case mensajeDesconexion:
				log_error(loggerMaster, "Error inesperado al recibir instrucciones de YAMA.");
				exit(1);
				break;
			case mensajeReplanificacion:
				worker= instruccionesYama.envio;
				crearHilosPorBloqueTransformacion(worker);
				break;
			case mensajeFinJob:
				finalizarJob();
				break;
		}
	}
}

void inicializarTiemposTransformacion(respuestaSolicitudTransformacion* infoTransformacion){
	int i,j;
	for(i=0 ; i<list_size(infoTransformacion->workers);i++){
		workerDesdeYama* worker = list_get(infoTransformacion->workers, i);
		for(j=0 ; j<list_size(worker->bloquesConSusArchivos);j++){
			bloquesConSusArchivosTransformacion* bloque = list_get(worker->bloquesConSusArchivos, j);
			setearTiempo(0,bloque->numBloque);
			estadisticas->cantTareas[0]++;
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
	estadisticas->tiempoInicio=time(NULL);
	estadisticas->tiempoFinTrans= list_create();
	estadisticas->tiempoFinRedLocal=list_create();
	estadisticas->tiempoFinRedGlobal= list_create();
	estadisticas->tiempoInicioTrans= list_create();
	estadisticas->tiempoInicioRedGlobal= list_create();
	estadisticas->tiempoInicioRedLocal= list_create();
	estadisticas->cantFallos=0;
	estadisticas->cantTareas[0]=0;
	estadisticas->cantTareas[1]=0;
	estadisticas->cantTareas[2]=0;
	return estadisticas;
}

void setearTiempo(int etapa,int numero){
	t_list* tiemposInicio,*tiemposFin;
	switch(etapa){
		case 0:
			tiemposInicio= estadisticas->tiempoInicioTrans;
			tiemposFin=estadisticas->tiempoFinTrans;
			break;

		case 1:
			tiemposInicio= estadisticas->tiempoInicioRedLocal;
			tiemposFin=estadisticas->tiempoFinRedLocal;
			break;

		case 2:
			tiemposInicio= estadisticas->tiempoInicioRedGlobal;
			tiemposFin=estadisticas->tiempoFinRedGlobal;
			break;
	}


	tiempoPorBloque* tiempoInicio = malloc(sizeof(tiempoPorBloque));
	tiempoInicio->finalizo= true;
	tiempoInicio->numero=numero;
	tiempoInicio->tiempo=time(NULL);
	list_add(tiemposInicio,tiempoInicio);

	tiempoPorBloque* tiempoFin= malloc(sizeof(tiempoPorBloque));
	tiempoFin->finalizo= false;
	tiempoFin->numero=numero;
	tiempoFin->tiempo=time(NULL);
	list_add(tiemposFin,tiempoFin);
}

void finalizarTiempo(t_list* tiempos,int numero){
	bool encontrarEnTablaEstados(tiempoPorBloque* time ) {
		return time->numero == numero;
	}

	tiempoPorBloque* tiempo = list_get(tiempos,numero);
	tiempo->finalizo=true;
	tiempo->tiempo=time(NULL);
}

void finalizarJob(){
	time_t fin= time(NULL);
	double duracion = difftime(fin,estadisticas->tiempoInicio);
	double promTransformacion = calcularDuracionPromedio(estadisticas->tiempoInicioTrans,estadisticas->tiempoFinTrans);
	double promLocal = calcularDuracionPromedio(estadisticas->tiempoInicioRedLocal,estadisticas->tiempoFinRedLocal);
	double promGlobal= calcularDuracionPromedio(estadisticas->tiempoInicioRedGlobal,estadisticas->tiempoFinRedGlobal);

	printf("Duracion total: %f\n",duracion);
	printf("Cantidad tareas transformacion %d\n",estadisticas->cantTareas[0]);
	printf("Duracion promedio transformacion: %f\n",promTransformacion);
	printf("Cantidad tareas reduccion local: %d\n",estadisticas->cantTareas[1]);
	printf("Duracion promedio reduccion local: %f\n",promLocal);
	printf("Cantidad tareas reduccion global: %d\n",estadisticas->cantTareas[2]);
	printf("Duracion promedio reduccion global: %f\n",promGlobal);
	printf("Cantidad de fallos obtenidos: %d\n",estadisticas->cantFallos);

	exit(0);
}

double calcularDuracionPromedio(t_list* tiemposInicio,t_list* tiemposFin){
	int i;
	double cont=0;

	if(list_is_empty(tiemposFin)){
		return 0;
	}

	for(i=0;i<list_size(tiemposInicio);i++){
		tiempoPorBloque* inicio = list_get(tiemposInicio,i);
		tiempoPorBloque* fin = list_get(tiemposFin,i);
		if(fin->finalizo){
			cont = cont + difftime(fin->tiempo,inicio->tiempo);
		}

	}

	return cont / list_size(tiemposInicio);

}
