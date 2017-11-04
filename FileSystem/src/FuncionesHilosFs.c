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
extern t_list* pedidosFS;
extern t_log* loggerFS;

void* levantarServidorFS(){

	int maxDatanodes;
	int nuevoDataNode;
	int cantidadNodos;
	informacionNodo info;

	int i = 0, l = 0;
	int addrlen;

	fd_set datanodes;
	fd_set read_fds_datanodes;

	respuesta conexionNueva, paqueteInfoNodo;

	FD_ZERO(&datanodes);    // borra los conjuntos datanodes y temporal
	FD_ZERO(&read_fds_datanodes);
	// añadir listener al conjunto maestro
	FD_SET(0, &datanodes);
	FD_SET(servidorFS, &datanodes);
	// seguir la pista del descriptor de fichero mayor
	maxDatanodes = clienteYama; // por ahora es éste
	// bucle principal
	while(1){
		read_fds_datanodes = datanodes; // cópialo

		if (select(maxDatanodes, &read_fds_datanodes, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maxDatanodes; i++) {
			if (FD_ISSET(i, &read_fds_datanodes)) { // ¡¡tenemos datos!!
				if (i == servidorFS) {
					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((nuevoDataNode = accept(servidorFS, (struct sockaddr *)&direccionCliente,
							&addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(nuevoDataNode, &datanodes); // añadir al conjunto maestro
						if (nuevoDataNode > maxDatanodes) {    // actualizar el máximo
							maxDatanodes = nuevoDataNode;
						}

						conexionNueva = desempaquetar(nuevoDataNode);

						if (*(int*)conexionNueva.envio == idDataNodes){
							//empaquetar(nuevoDataNode,1,0,&bufferPrueba);//FIXME:SOLO A MODO DE PRUEBA
							paqueteInfoNodo = desempaquetar(nuevoDataNode);
							info = *(informacionNodo*)paqueteInfoNodo.envio;
							if (nodoRepetido(info) == 0){
								pthread_mutex_lock(&logger_mutex);
								log_trace(loggerFS, "Conexion de DataNode %d\n", info.numeroNodo);
								pthread_mutex_unlock(&logger_mutex);
								info.bloquesOcupados = levantarBitmapNodo(info.numeroNodo, info.sizeNodo);
								info.socket = nuevoDataNode;
								memcpy(paqueteInfoNodo.envio, &info, sizeof(informacionNodo));
								list_add(nodosConectados,paqueteInfoNodo.envio);
								cantidadNodos = list_size(nodosConectados);
								actualizarArchivoNodos();
								sem_t semaforo;
								sem_init(&semaforo,1,1);
								list_add(pedidosFS, &semaforo);
							}
							else{
								pthread_mutex_lock(&logger_mutex);
								log_trace(loggerFS, "DataNode repetido\n");
								pthread_mutex_unlock(&logger_mutex);
							}
						}

					}
				}
				else {
					//conexionNueva = desempaquetar(nuevoDataNode);
					switch(conexionNueva.idMensaje){

					}

				}

			}
		}
	}
	return 0;

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

		pthread_mutex_lock(&logger_mutex);
		log_trace(loggerFS, "El usuario ingreso: %s", comando);
		pthread_mutex_unlock(&logger_mutex);

		if (string_starts_with(comando, "format")) {
			int a = formatearFS(comando);
			pthread_mutex_lock(&logger_mutex);
			log_trace(loggerFS, "File system formateado");
			pthread_mutex_unlock(&logger_mutex);
		}
		else if (string_starts_with(comando, "rm -d")) {
			if (eliminarDirectorio(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Directorio eliminado");
				pthread_mutex_unlock(&logger_mutex);
			}
			if (eliminarDirectorio(comando) == 2){
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "El directorio no existe");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo eliminar el directorio, no esta vacio");
				pthread_mutex_unlock(&logger_mutex);

			}
		}
		else if (string_starts_with(comando, "rm -b")) {
			pthread_mutex_lock(&logger_mutex);
			log_trace(loggerFS, "Bloque eliminado");
			pthread_mutex_unlock(&logger_mutex);
		}
		else if (string_starts_with(comando, "rm")) {
			if (eliminarArchivo(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "archivo eliminado");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo eliminar el archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (string_starts_with(comando, "rename")) {
			if (cambiarNombre(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Renombrado");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo renombrar");
				pthread_mutex_unlock(&logger_mutex);
			}

		}
		else if (string_starts_with(comando, "mv")) {
			if (mover(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Archivo movido");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo mover el archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (string_starts_with(comando, "cat")) {
			if (mostrarArchivo(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Archivo mostrado");
				pthread_mutex_unlock(&logger_mutex);
			}else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo mostrar el archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (string_starts_with(comando, "mkdir")) {
			if (crearDirectorio(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Directorio creado");// avisar si ya existe
				pthread_mutex_unlock(&logger_mutex);
			}else{
				if (crearDirectorio(comando) == 1){
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "El directorio ya existe");
					pthread_mutex_unlock(&logger_mutex);
				}else{
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "No se pudo crear el directorio");
					pthread_mutex_unlock(&logger_mutex);
				}
			}
		}
		else if (string_starts_with(comando, "cpfrom")) {
			if (copiarArchivo(comando) == 1){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Archivo copiado a yamafs");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (string_starts_with(comando, "cpto")) {
			if (copiarArchivoAFs(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Archivo copiado desde yamafs");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el archivo desde yamafs");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (string_starts_with(comando, "cpblock")) {
			if (copiarBloqueANodo(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Bloque copiado en el nodo");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el bloque copiado en el nodo");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (string_starts_with(comando, "md5")) {
			if (generarArchivoMD5(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "MD5 del archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo obtener el MD5 del archivo");
				pthread_mutex_unlock(&logger_mutex);
			}

		}
		else if (string_starts_with(comando, "ls")) {
			if (listarArchivos(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Archivos listados");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "El directorio no existe");
				pthread_mutex_unlock(&logger_mutex);
			}

		}
		else if (string_starts_with(comando, "info")) {
			if (informacion(comando) == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Mostrando informacion del archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo mostrar informacion del archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else {
			printf("Comando invalido\n");
			pthread_mutex_lock(&logger_mutex);
			log_error(loggerFS, "Comando invalido");
			pthread_mutex_unlock(&logger_mutex);
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

