/*
 * FuncionesHilosFs.c
 *
 *  Created on: 11/10/2017
 *      Author: utnso
 */

#include "FuncionesHilosFs.h"

int clienteYama;
int servidorFS;
struct sockaddr_in direccionCliente;
extern sem_t pedidoLecturaFS[];

void levantarServidorFS(){
	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion = sizeof(direccionCliente);
	respuesta paqueteInfoNodo,respuestaId;
	informacionNodo info;
	int cantidadNodos;
	bool noSeConecteYama = true;

	while (noSeConecteYama) {
		int nuevoCliente;
		if ((nuevoCliente = accept(servidorFS,(struct sockaddr *) &direccionCliente, &tamanioDireccion))!= -1) {

			pthread_t nuevoHilo;
			pthread_attr_t attr;

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			respuestaId = desempaquetar(nuevoCliente);
			int id = *(int*)respuestaId.envio;

			if(id == idDataNodes){
				paqueteInfoNodo = desempaquetar(nuevoCliente);
				info = *(informacionNodo*)paqueteInfoNodo.envio;
				if (nodoRepetido(info) == 0){
					pthread_mutex_lock(loggerFS);
					log_trace(loggerFS, "Conexion de DataNode %d\n", info.numeroNodo);
					pthread_mutex_unlock(loggerFS);

					info.bloquesOcupados = info.sizeNodo - levantarBitmapNodo(info.numeroNodo, info.sizeNodo);
					info.socket = nuevoCliente;
					memcpy(paqueteInfoNodo.envio, &info, sizeof(informacionNodo));
					list_add(nodosConectados,paqueteInfoNodo.envio);
					cantidadNodos = list_size(nodosConectados);
					actualizarArchivoNodos();
					sem_init(&pedidoLecturaFS[list_size(nodosConectados)-1],0,1);

					if (pthread_create(&nuevoHilo, &attr, &manejarConexionDataNode,NULL) == -1) {
						log_error(loggerFS, "could not create thread");
						perror("could not create thread");
						log_destroy(loggerFS);
						exit(1);
					}
				}
				else{
					pthread_mutex_lock(loggerFS);
					log_trace(loggerFS, "DataNode repetido\n");
					pthread_mutex_unlock(loggerFS);
				}
			}
			else if (id == idYAMA){
				pthread_mutex_lock(loggerFS);
				log_trace(loggerFS, "Conexion de YAMA");
				pthread_mutex_unlock(loggerFS);
				empaquetar(nuevoCliente,mensajeOk,0,0);
				clienteYama = nuevoCliente;
				noSeConecteYama= false;

				if (pthread_create(&nuevoHilo, &attr, &manejarConexionYama,NULL) == -1) {
					log_error(loggerFS, "could not create thread");
					perror("could not create thread");
					log_destroy(loggerFS);
					exit(1);
				}

			}
			else{
				pthread_mutex_lock(loggerFS);
				log_trace(loggerFS, "Conexion invalida\n");
				pthread_mutex_unlock(loggerFS);
			}
		}
	}
}

void* consolaFS(){

	int sizeComando = 256;

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando + 1);
		memset(comando, 0,sizeComando + 1);
		comando = readline(">");
		if (comando)
			add_history(comando);

		log_trace(loggerFS, "El usuario ingreso: %s", comando);

		if (string_starts_with(comando, "format")) {
			log_trace(loggerFS, "File system formateado");
			leerArchivo("hola/chau/hola3.txt");
		}
		else if (string_starts_with(comando, "rm -d")) {
			if (eliminarDirectorio(comando) == 0)
				log_trace(loggerFS, "Directorio eliminado");
			else
				log_error(loggerFS, "No se pudo eliminar el directorio");
		}
		else if (string_starts_with(comando, "rm -b")) {
			log_trace(loggerFS, "Bloque eliminado");
		}
		else if (string_starts_with(comando, "rm")) {
			if (eliminarArchivo(comando) == 0)
				log_trace(loggerFS, "archivo eliminado");
			else
				log_error(loggerFS, "No se pudo eliminar el archivo");
		}
		else if (string_starts_with(comando, "rename")) {
			if (cambiarNombre(comando) == 0)
				log_trace(loggerFS, "Renombrado");
			else
				log_error(loggerFS, "No se pudo renombrar");

		}
		else if (string_starts_with(comando, "mv")) {
			if (mover(comando) == 0)
				log_trace(loggerFS, "Archivo movido");
			else
				log_error(loggerFS, "No se pudo mover el archivo");
		}
		else if (string_starts_with(comando, "cat")) {
			if (mostrarArchivo(comando) == 0){
			log_trace(loggerFS, "Archivo mostrado");
			}else{
				log_error(loggerFS, "No se pudo mostrar el archivo");
			}
		}
		else if (string_starts_with(comando, "mkdir")) {
			if (crearDirectorio(comando) == 0){

			log_trace(loggerFS, "Directorio creado");// avisar si ya existe
			}else{
				if (crearDirectorio(comando) == 1){
					log_error(loggerFS, "El directorio ya existe");
				}else{
					log_error(loggerFS, "No se pudo crear el directorio");
				}
			}
		}
		else if (string_starts_with(comando, "cpfrom")) {
			if (copiarArchivo(comando) == 1)
				log_trace(loggerFS, "Archivo copiado a yamafs");
			else
				log_error(loggerFS, "No se pudo copiar el archivo");
		}
		else if (string_starts_with(comando, "cpto")) {
			log_trace(loggerFS, "Archivo copiado desde yamafs");
		}
		else if (string_starts_with(comando, "cpblock")) {
			log_trace(loggerFS, "Bloque copiado en el nodo");
		}
		else if (string_starts_with(comando, "md5")) {
			if (generarArchivoMD5(comando) == 0)
				log_trace(loggerFS, "MD5 del archivo");
			else
				log_error(loggerFS, "No se pudo obtener el MD5 del archivo");

		}
		else if (string_starts_with(comando, "ls")) {
			if (listarArchivos(comando) == 0)
				log_trace(loggerFS, "Archivos listados");
			else
				log_error(loggerFS, "El directorio no existe");

		}
		else if (string_starts_with(comando, "info")) {
			if (informacion(comando) == 0)
				log_trace(loggerFS, "Mostrando informacion del archivo");
			else
				log_error(loggerFS, "No se pudo mostrar informacion del archivo");
		}
		else {
			printf("Comando invalido\n");
			log_error(loggerFS, "Comando invalido");
		}
		free(comando);
	}
	return 0;
}

void* manejarConexionYama(){
	respuesta respuestaYama;
	solicitudInfoNodos* solicitud;

	while(1){
		respuestaYama = desempaquetar(clienteYama);

		switch(respuestaYama.idMensaje){

		case mensajeSolicitudInfoNodos:
			solicitud = (solicitudInfoNodos*)respuestaYama.envio;
			informacionArchivoFsYama infoArchivo = obtenerInfoArchivo(solicitud->rutaDatos);
			empaquetar(clienteYama,mensajeRespuestaInfoNodos,0,&infoArchivo);
			desempaquetar(clienteYama);
			break;
		}
	}
}

void* manejarConexionDataNode(){
	while(1){

	}
}
