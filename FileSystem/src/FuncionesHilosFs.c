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

void* levantarServidorFS(){

	int maxDatanodes;
	int nuevoDataNode;
	int cantidadNodos;
	informacionNodo info;

	int i = 0;
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
								log_trace(loggerFS, "Conexion de DataNode %d\n", info.numeroNodo);
								info.bloquesOcupados = info.sizeNodo - levantarBitmapNodo(info.numeroNodo, info.sizeNodo);
								info.socket = nuevoDataNode;
								memcpy(paqueteInfoNodo.envio, &info, sizeof(informacionNodo));
								list_add(nodosConectados,paqueteInfoNodo.envio);
								cantidadNodos = list_size(nodosConectados);
								actualizarArchivoNodos();
							}
							else{
								log_trace(loggerFS, "DataNode repetido\n");
							}
						}

					}
				}
				else {
					conexionNueva = desempaquetar(nuevoDataNode);
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
		char* comando = malloc(sizeof(char) * sizeComando);
		bzero(comando, sizeComando);
		comando = readline(">");
		if (comando)
			add_history(comando);

		log_trace(loggerFS, "El usuario ingreso: %s", comando);

		if (string_starts_with(comando, "format")) {
			log_trace(loggerFS, "File system formateado");
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
			printf("me llega la solicitud\n");
			solicitud = (solicitudInfoNodos*)respuestaYama.envio;
			printf("la ruta es %s\n",solicitud->rutaDatos.cadena);
			informacionArchivoFsYama infoArchivo = obtenerInfoArchivo(solicitud->rutaDatos);

			break;
		}
	}
}

