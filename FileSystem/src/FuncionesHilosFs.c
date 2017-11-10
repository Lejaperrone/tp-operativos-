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
extern int bloquesLibresTotales;

void* levantarServidorFS(){
	solicitudInfoNodos* solicitud;
	bool noSeConectoYama = true;
	int maxDatanodes;
	int nuevoCliente;
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
	FD_SET(servidorFS, &datanodes);
	// seguir la pista del descriptor de fichero mayor
	maxDatanodes = servidorFS; // por ahora es éste
	// bucle principal

	while(1){
		read_fds_datanodes = datanodes; // cópialo

		if (select(maxDatanodes+1, &read_fds_datanodes, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maxDatanodes; i++) {
			if (FD_ISSET(i, &read_fds_datanodes)) { // ¡¡tenemos datos!!
				if (i == servidorFS && noSeConectoYama) {
					// gestionar nuevas conexiones
					addrlen = sizeof(direccionCliente);
					if ((nuevoCliente = accept(servidorFS, (struct sockaddr *)&direccionCliente,
							&addrlen)) == -1) {
						perror("accept");
					} else {
						FD_SET(nuevoCliente, &datanodes); // añadir al conjunto maestro
						if (nuevoCliente > maxDatanodes) {    // actualizar el máximo
							maxDatanodes = nuevoCliente;
						}

						conexionNueva = desempaquetar(nuevoCliente);

						if (*(int*)conexionNueva.envio == idDataNodes){
							paqueteInfoNodo = desempaquetar(nuevoCliente);
							info = *(informacionNodo*)paqueteInfoNodo.envio;
							if (nodoRepetido(info) == 0){
								pthread_mutex_lock(&logger_mutex);
								log_trace(loggerFS, "Conexion de DataNode %d\n", info.numeroNodo);
								pthread_mutex_unlock(&logger_mutex);
								info.bloquesOcupados = levantarBitmapNodo(info.numeroNodo, info.sizeNodo);
								bloquesLibresTotales += info.sizeNodo - info.bloquesOcupados;
								info.socket = nuevoCliente;
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
						else if(*(int*)conexionNueva.envio == 1){//yama
							clienteYama = nuevoCliente;
							log_trace(loggerFS, "Nueva Conexion de Yama");
							empaquetar(nuevoCliente,mensajeOk,0,0);
							noSeConectoYama=false;
						}

					}
				}
				else{
					if (i == clienteYama){
						conexionNueva= desempaquetar(clienteYama);
						switch(conexionNueva.idMensaje){

						case mensajeSolicitudInfoNodos:
							solicitud = (solicitudInfoNodos*)conexionNueva.envio;
							informacionArchivoFsYama infoArchivo = obtenerInfoArchivo(solicitud->rutaDatos);
							empaquetar(clienteYama,mensajeRespuestaInfoNodos,0,&infoArchivo);
							break;
						}
					}


				}

			}
		}
	}
	return 0;

}

void* consolaFS(){

	int sizeComando = 256;
	int resultado = 0;

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando + 1);
		memset(comando, 0,sizeComando + 1);
		comando = readline(">");

		char** arguments = string_split(comando, " ");

		if(validarParametros(arguments, 1)){
			continue;
		}

		//printf("----------------------%s\n",)

		if (comando)
			add_history(comando);

		pthread_mutex_lock(&logger_mutex);
		log_trace(loggerFS, "El usuario ingreso: %s", comando);
		pthread_mutex_unlock(&logger_mutex);

		if (strcmp(arguments[0], "format") == 0) {
			if (formatearFS() == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "File system formateado");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo formatear el File system");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (strcmp(arguments[0],"rm") == 0) {

			if(validarParametros(arguments, 2)){
				continue;
			}

			if (strcmp(arguments[1],"-d") == 0){
				if(validarParametros(arguments, 3)){
					continue;
				}
				resultado = eliminarDirectorio(comando);
				if (resultado == 0){
					pthread_mutex_lock(&logger_mutex);
					log_trace(loggerFS, "Directorio eliminado");
					pthread_mutex_unlock(&logger_mutex);
				}
				else if (resultado == 2){
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

			else if (strcmp(arguments[1], "-b") == 0) {
				if(validarParametros(arguments, 3)){
					continue;
				}
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Bloque eliminado");
				pthread_mutex_unlock(&logger_mutex);
			}

			else if (arguments[1] != NULL) {
				resultado = eliminarArchivo(comando);
				if (resultado == 0){
					pthread_mutex_lock(&logger_mutex);
					log_trace(loggerFS, "archivo eliminado");
					pthread_mutex_unlock(&logger_mutex);
				}
				else if (resultado == 2){
					pthread_mutex_lock(&logger_mutex);
					log_trace(loggerFS, "la ruta ingresada no pertence a yamafs");
					pthread_mutex_unlock(&logger_mutex);
				}
				else if (resultado == 3){
					pthread_mutex_lock(&logger_mutex);
					log_trace(loggerFS, "la ruta ingresada no pertenece a un archivo");
					pthread_mutex_unlock(&logger_mutex);
				}
				else{
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "No se pudo eliminar el archivo");
					pthread_mutex_unlock(&logger_mutex);
				}

			}
		}
		else if (strcmp(arguments[0], "rename") == 0) {
			if(validarParametros(arguments, 3)){
				continue;
			}
			resultado = cambiarNombre(comando);
			if (resultado == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Renombrado");
				pthread_mutex_unlock(&logger_mutex);
			}
			else if (resultado == 2){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "la ruta ingresada no pertence a yamafs");
				pthread_mutex_unlock(&logger_mutex);
			}
			else if (resultado == 3){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "la ruta ingresada no pertenece a un archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo renombrar");
				pthread_mutex_unlock(&logger_mutex);
			}

		}
		else if (strcmp(arguments[0], "mv") == 0) {
			if(validarParametros(arguments, 3)){
				continue;
			}
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
		else if (strcmp(arguments[0], "cat") == 0) {
			if(validarParametros(arguments, 2)){
				continue;
			}
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
		else if (strcmp(arguments[0], "mkdir") == 0) {
			if(validarParametros(arguments,2)){
				continue;
			}
			resultado = crearDirectorio(comando);
			if (resultado == 0){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Directorio creado");// avisar si ya existe
				pthread_mutex_unlock(&logger_mutex);
			}else{
				if (resultado == 1){
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
		else if (strcmp(arguments[0], "cpfrom") == 0) {
			if(validarParametros(arguments, 3)){
				continue;
			}
			int resultado = copiarArchivo(comando);
			if (resultado == 1){
				pthread_mutex_lock(&logger_mutex);
				log_trace(loggerFS, "Archivo copiado a yamafs");
				pthread_mutex_unlock(&logger_mutex);
			}
			else if(resultado == 0){
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el archivo");
				pthread_mutex_unlock(&logger_mutex);
			}
			else{
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el archivo, espacio insuficiente");
				pthread_mutex_unlock(&logger_mutex);
			}
		}
		else if (strcmp(arguments[0], "cpto") == 0) {
			if(validarParametros(arguments, 3)){
				continue;
			}
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
		else if (strcmp(arguments[0], "cpblock") == 0) {
			if(validarParametros(arguments, 4)){
				continue;
			}
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
		else if (strcmp(arguments[0], "md5") == 0) {
			if(validarParametros(arguments, 2)){
				continue;
			}
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
		else if (strcmp(arguments[0], "ls") == 0) {
			if(validarParametros(arguments, 2)){
				continue;
			}
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
		else if (strcmp(arguments[0], "info") == 0) {
			if(validarParametros(arguments, 2)){
				continue;
			}
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
		free(arguments);
		free(comando);
	}
	return 0;
}

int validarParametros(char** arguments, int cantidadParametros){
	int i = 0;
	for (i = 0; i < cantidadParametros; ++i){
		if(arguments[i] == NULL){
			log_error(loggerFS, "comando invalido, faltan parametros");
			return 1;
		}
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
			break;
		}
	}
}

