/*
 * FuncionesHilosFs.c
 *
 *  Created on: 11/10/2017
 *      Author: utnso
 */

#include "FuncionesHilosFs.h"

int clienteYama = -1;
int servidorFS;
struct sockaddr_in direccionCliente;
extern t_list* pedidosFS;
extern t_log* loggerFS;
extern int bloquesLibresTotales;
extern bool recuperarEstado;
extern int EstadoFS;
bool noSeConectoYama = true;

void* levantarServidorFS(){
	solicitudInfoNodos* solicitud;
	int maxDatanodes;
	int nuevoCliente;
	int cantidadNodos;
	informacionNodo info;
	respuesta respuestaWorker;
	almacenamientoFinal* almacenar;

	int i = 0, l = 0, m = 0;
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


	t_list* listAux;
	int j;
	char* buff = malloc(sizeof(int)*3);
	int desconexion = -1;

	// bucle principal

	signal(SIGPIPE, SIG_IGN);

	while(1){
		read_fds_datanodes = datanodes; // cópialo
		if (select(maxDatanodes+1, &read_fds_datanodes, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= maxDatanodes; i++) {
			if (FD_ISSET(i, &read_fds_datanodes)) { // ¡¡tenemos datos!!
				if (i == servidorFS) {

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
							//revisarNodos();
							if (nodoRepetido(info) == 0 && nodoDeEstadoAnterior(info)){
								pthread_mutex_lock(&logger_mutex);
								log_trace(loggerFS, "Conexion de DataNode %d\n", info.numeroNodo);
								pthread_mutex_unlock(&logger_mutex);
								printf("Conexión de DataNode %d.\n", info.numeroNodo);
								info.bloquesOcupados = levantarBitmapNodo(info.numeroNodo, info.sizeNodo);
								bloquesLibresTotales += info.sizeNodo - info.bloquesOcupados;
								info.socket = nuevoCliente;
								memcpy(paqueteInfoNodo.envio, &info, sizeof(informacionNodo));
								list_add(nodosConectados,paqueteInfoNodo.envio);
								cantidadNodos = list_size(nodosConectados);
								if (EstadoFS == 1)
									actualizarArchivoNodos();
								sem_t semaforo;
								sem_init(&semaforo,1,1);
								list_add(pedidosFS, &semaforo);
								EstadoFS = verificarEstado();
								/*if (!recuperarEstado){
									informacionNodo* nodo = (informacionNodo*) paqueteInfoNodo.envio;
							 		empaquetar(nodo->socket, mensajeBorraDataBin, 0, 0);
							 		respuesta res = desempaquetar(nodo->socket);
							 		int resultado = *(int*) res.envio;
							 		if (resultado == 1)
							 			printf("Nodo%d formateado\n", nodo->numeroNodo);
								}*/
							}
							else{
								pthread_mutex_lock(&logger_mutex);
								log_trace(loggerFS, "DataNode invalido\n");
								pthread_mutex_unlock(&logger_mutex);
								if (fsFormateado)
									printf("No se puede conectar el DataNode, fileSystem formateado.\n");
								if (nodoRepetido(info))
									printf("El nodo ya esta conectado.\n");
								if (!nodoDeEstadoAnterior(info))
									printf("El nodo no pertenece al estado anterior.\n");
								close(nuevoCliente);
								FD_CLR(nuevoCliente, &datanodes);
							}
						}
						else if(*(int*)conexionNueva.envio == 1){//yama
							if(!EstadoFS){
								empaquetar(nuevoCliente,mensajeNoEstable,0,0);
								close(nuevoCliente);
								FD_CLR(nuevoCliente, &datanodes);
								continue;
							}
							clienteYama = nuevoCliente;
							log_trace(loggerFS, "Nueva Conexion de Yama");
							printf("Nueva Conexion de Yama\n");
							empaquetar(nuevoCliente,mensajeOk,0,0);
							noSeConectoYama=false;
						}
						else if (*(int*)conexionNueva.envio == idWorker){
							empaquetar(nuevoCliente,mensajeOk,0,0);
							log_trace(loggerFS, "Nueva Conexion de Worker");
							printf("Nueva Conexion de Worker\n");
							respuestaWorker=desempaquetar(nuevoCliente);
							almacenar= respuestaWorker.envio;

							char* directorio = rutaSinArchivo(almacenar->nombre.cadena);
							char* nombreArchivo = ultimaParteDeRuta(almacenar->nombre.cadena);

							string* mmap = malloc(sizeof(string));
							mmap->cadena = strdup(almacenar->contenido.cadena);
							mmap->longitud = almacenar->contenido.longitud;
							free(almacenar->contenido.cadena);

							int resultado = guardarEnNodos(directorio, nombreArchivo, "t", mmap);

							if (resultado == 1){
								pthread_mutex_lock(&logger_mutex);
								log_trace(loggerFS, "Archivo copiado a yamafs");
								pthread_mutex_unlock(&logger_mutex);
								empaquetar(nuevoCliente,mensajeAlmacenamientoCompleto,0,0);
							}
							else if(resultado == 0){
								pthread_mutex_lock(&logger_mutex);
								log_error(loggerFS, "No se pudo copiar el archivo");
								pthread_mutex_unlock(&logger_mutex);
								empaquetar(nuevoCliente,mensajeFalloAlmacenamiento,0,0);
							}
							else if(resultado == 2){
								pthread_mutex_lock(&logger_mutex);
								log_error(loggerFS, "No se pudo copiar el archivo, espacio insuficiente");
								pthread_mutex_unlock(&logger_mutex);
								empaquetar(nuevoCliente,mensajeFalloAlmacenamiento,0,0);
							}
							else if(resultado == 3){
								pthread_mutex_lock(&logger_mutex);
								log_error(loggerFS, "No se pudo copiar el archivo, ya existe");
								pthread_mutex_unlock(&logger_mutex);
								empaquetar(nuevoCliente,mensajeFalloAlmacenamiento,0,0);
							}
							free(mmap);
							//free(mmap->cadena);
							free(almacenar);
							close(nuevoCliente);
							FD_CLR(nuevoCliente, &datanodes);
						}

					}
				}
				else if(i == clienteYama){
					conexionNueva= desempaquetar(clienteYama);
					switch(conexionNueva.idMensaje){

					case mensajeSolicitudInfoNodos:
						solicitud = (solicitudInfoNodos*)conexionNueva.envio;
						informacionArchivoFsYama infoArchivo = obtenerInfoArchivo(solicitud);
						if(infoArchivo.tamanioTotal==0){
							empaquetar(clienteYama,mensajeRespuestaInfoFallida,0,0);
						}
						else{
							empaquetar(clienteYama,mensajeRespuestaInfoNodos,0,&infoArchivo);

							void siesta(ubicacionBloque* ubi){
								free(ubi->ip.cadena);
								free(ubi);
							}

							void limpiar(infoBloque* info){
								list_iterate(info->ubicaciones,(void*)siesta);
								list_destroy(info->ubicaciones);
								free(info);
							}

							list_iterate(infoArchivo.informacionBloques,(void*)limpiar);

						}
						break;

					case mensajeDesconexion:
						printf("Se deconecto Yama\n");
						noSeConectoYama = true;
						FD_CLR(clienteYama, &datanodes);
						close(clienteYama);
						clienteYama = -1;
						break;
					}

				}
				if (recv(i,buff,1,MSG_PEEK | MSG_DONTWAIT) == 0){

					bool sacarDesconectar(informacionNodo* info){
						return info->socket==i;
					}

					list_remove_by_condition(nodosConectados,(void*)sacarDesconectar);

					printf("Desconexion\n");
					close(i);
					FD_CLR(i, &datanodes);
					EstadoFS = verificarEstado();
				}



			}

		}
	}

	return 0;

}

void* estadoYama(){

		while (1){
			if (!noSeConectoYama){
				if (recv(clienteYama, 0, 1, MSG_PEEK) == 0){
					printf("Se desconecto Yama\n");
					noSeConectoYama = true;
				}
			}
		}

	return 0;

}


int nodoDeEstadoAnterior(informacionNodo info){

	int i = 0;
	char* pathArchivo = "../metadata/Nodos.bin";

	if(!recuperarEstado)
		return 1;

	t_config* nodos = config_create(pathArchivo);
	char** arrayNodos = config_get_array_value(nodos,"NODOS");
	while(arrayNodos[i] != NULL){
		if(atoi(string_substring_from(arrayNodos[i], 4)) == info.numeroNodo)
			return 1;
		++i;
	}

	return 0;
}

int revisarNodos(){
	int cantidadNodos = list_size(nodosConectados);
	int i, j;
	informacionNodo info;
	respuesta res;
	t_list* listAux;
	//printf("entre----");
	for (i = 0; i < cantidadNodos; ++i){
		info = *(informacionNodo*)list_get(nodosConectados,i);
		empaquetar(info.socket, mensajeConectado, sizeof(char), "a");
		res = desempaquetar(info.socket);
		if (res.idMensaje == mensajeDesconexion){
			listAux = list_create();
			for (j = 0; j < cantidadNodos; ++j){
				if (j != i){
					list_add(listAux, list_get(nodosConectados, j));
				}
			}
			list_destroy(nodosConectados);
			nodosConectados = list_create();
			for (j = 0; j < list_size(listAux); ++j){
				list_add(nodosConectados, list_get(listAux, j));
			}
			list_destroy(listAux);
			return info.socket;
		}
	}
	return -1;
}

void* consolaFS(){

	int sizeComando = 256;
	int resultado = 0;
	int i;
	informacionNodo info;

	while (1) {
		printf("Introduzca comando: ");
		char* comando = malloc(sizeof(char) * sizeComando + 1);
		memset(comando, 0,sizeComando + 1);
		comando = readline(">");

		char** arguments = string_split(comando, " ");

		//revisarNodos();

		if(validarParametros(arguments, 1)){
			continue;
		}

		//printf("----------------------%s\n",)

		if (comando)
			add_history(comando);

		pthread_mutex_lock(&logger_mutex);
		log_trace(loggerFS, "El usuario ingreso: %s", comando);
		pthread_mutex_unlock(&logger_mutex);
		
		EstadoFS = verificarEstado();

		if (strcmp(arguments[0], "format") == 0) {
			if (formatearFS(0) == 0){
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
		else if (strcmp(arguments[0],"status") == 0) {
			if (list_size(nodosConectados) == 0){
				printf("Estado del File system: Inestable\n");
				printf("No hay nodos conectados\n");
			}
			else{
				if (EstadoFS)
					printf("Estado del File system: Estable\n");
				else
					printf("Estado del File system: Inestable\n");
				for (i = 0; i < list_size(nodosConectados); ++i){
					info = *(informacionNodo*)list_get(nodosConectados, i);
					printf("Nodo%d\n", info.numeroNodo);
					printf("	Tamaño: %d\n", info.sizeNodo);
					printf("	Bloques ocupados: %d\n", info.bloquesOcupados);
					printf("	Bloques libres: %d\n", info.sizeNodo - info.bloquesOcupados);
				}
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
				else if (resultado == 3){
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "No se pudo eliminar directorio root");
					pthread_mutex_unlock(&logger_mutex);
				}
				else{
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "No se pudo eliminar el directorio, no esta vacio");
					pthread_mutex_unlock(&logger_mutex);

				}

			}

			else if (strcmp(arguments[1], "-b") == 0) {
				if(validarParametros(arguments, 5)){
					continue;
				}
				resultado = eliminarBloque(comando);
				if (resultado == 0){
					pthread_mutex_lock(&logger_mutex);
					log_trace(loggerFS, "Bloque eliminado");
					pthread_mutex_unlock(&logger_mutex);
				}
				else if (resultado == 1){
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "El bloque no existe");
					pthread_mutex_unlock(&logger_mutex);
				}
				else if (resultado == 2){
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "El bloque es la ultima copia");
					pthread_mutex_unlock(&logger_mutex);
				}
				else{
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "Error al intentar eliminar el bloque");
					pthread_mutex_unlock(&logger_mutex);
				}

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
					log_error(loggerFS, "la ruta ingresada no pertence a yamafs");
					pthread_mutex_unlock(&logger_mutex);
				}
				else if (resultado == 3){
					pthread_mutex_lock(&logger_mutex);
					log_error(loggerFS, "la ruta ingresada no pertenece a un archivo");
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
		else if (strcmp(arguments[0], "mkdir100") == 0) {
			int i;
			for (i = 0; i < 99; ++i) {
				char* command = string_from_format("mkdir yamafs:/directorios/%d", i);
				resultado = crearDirectorio(command);
				if (resultado == 0){
					pthread_mutex_lock(&logger_mutex);
					log_trace(loggerFS, "Directorio yamafs:/directorios/%d creado", i);
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
				usleep(100000);
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
			else if(resultado == 2){
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el archivo, espacio insuficiente");
				pthread_mutex_unlock(&logger_mutex);
			}
			else if(resultado == 3){
				pthread_mutex_lock(&logger_mutex);
				log_error(loggerFS, "No se pudo copiar el archivo, ya existe");
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
			if (copiarBloqueANodo(comando) <= 0){
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
			printf("comando invalido, faltan parametros\n");
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
			informacionArchivoFsYama infoArchivo = obtenerInfoArchivo(solicitud);
			empaquetar(clienteYama,mensajeRespuestaInfoNodos,0,&infoArchivo);
			break;
		}
	}
}

