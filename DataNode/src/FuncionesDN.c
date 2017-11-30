#include <stdio.h>
#include <stdlib.h>
#include "FuncionesDN.h"
#include "Globales.h"
#include "Serializacion.h"
#include "Sockets.h"
#include "Configuracion.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <commons/string.h>
#include "Serial.h"
#include <sys/types.h>

#define mb 1048576

int cantBloques = 50;
extern struct configuracionNodo config;

void enviarBloqueAFS(int numeroBloque) {

}//cpfrom /home/utnso/hola2.txt hola/chau

int setBloque(int numeroBloque, char* datos) {
	int fd = open(config.RUTA_DATABIN, O_RDWR);
	//printf("-----------%d %d\n", strlen(datos), numeroBloque);
	char* mapaDataBin = mmap(0, mb, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mb*numeroBloque);
	memset(mapaDataBin,0, mb);
	memcpy(mapaDataBin, datos, strlen(datos));
	int success = 1;
	if (msync(mapaDataBin, strlen(datos), MS_SYNC) == -1)
	{
		perror("Could not sync the file to disk");
		success = 0;
	}
	if (munmap(mapaDataBin, mb) == -1)
	{
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);
	return success;
}

char* getBloque(int numeroBloque) {
	int fd = open(config.RUTA_DATABIN, O_RDWR);
	char* mapaDataBin = mmap(0, mb, PROT_READ, MAP_SHARED, fd, mb*numeroBloque);
	//printf("%d %d\n", numeroBloque, sizeBloque);
	char* datos = malloc(mb+1);
	memset(datos,0,mb+1);
	memcpy(datos, mapaDataBin, strlen(mapaDataBin));
	if (munmap(mapaDataBin,mb) == -1)
	{
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);
	return datos;
}

int borrarDataBin(){

	FILE* databin = fopen(config.RUTA_DATABIN,"w+");

	truncate(config.RUTA_DATABIN, config.SIZE_NODO*mb);

	fclose(databin);

	return 1;
}

void inicializarDataBin(){
	if (!validarArchivo(config.RUTA_DATABIN)){
		char* directorioDataBin = rutaSinArchivo(config.RUTA_DATABIN);
		if (!validarDirectorio(directorioDataBin)){
			char* mkdirDirectorio = string_from_format("mkdir %s", directorioDataBin);
			system(mkdirDirectorio);
			free(mkdirDirectorio);
		}

		printf("Se crea el archivo data.bin\n");
		FILE* databin = fopen(config.RUTA_DATABIN,"w+");
		truncate(config.RUTA_DATABIN, config.SIZE_NODO*mb);
		//fwrite("0",1,config.SIZE_NODO*mb, databin);
		fclose(databin);
	}
}

void conectarseConFs() {
	int socketFs = crearSocket();
	struct sockaddr_in direccion = cargarDireccion(config.IP_FILESYSTEM, config.PUERTO_FILESYSTEM);
	conectarCon(direccion, socketFs, 3);
	informacionNodo info;
	info.ip.cadena = config.IP_NODO;
	info.ip.longitud = strlen(info.ip.cadena);
	info.puerto = config.PUERTO_WORKER;
	info.sizeNodo = config.SIZE_NODO;
	info.bloquesOcupados = -1;
	info.numeroNodo = atoi(string_substring_from(config.NOMBRE_NODO, 4));
	log_trace(logger, "Conexion con File System establecida, soy el Nodo: %d", info.numeroNodo);
	info.socket = -1;
	empaquetar(socketFs, mensajeInformacionNodo, sizeof(informacionNodo), &info);
	escucharAlFS(socketFs);
}

void recibirMensajesFileSystem(int socketFs) {
	respuesta numeroBloque = desempaquetar(socketFs);
	respuesta bloqueArchivo;
	//char* buffer = malloc(mb + 4);
	int bloqueId;
	char* data;
	int success;

	switch (numeroBloque.idMensaje) {
	case mensajeNumeroCopiaBloqueANodo:
		bloqueArchivo = desempaquetar(socketFs);
		memcpy(&bloqueId, numeroBloque.envio, sizeof(int));
		data = malloc(bloqueArchivo.size + 1);
		memset(data, 0, bloqueArchivo.size + 1);
		memcpy(data, bloqueArchivo.envio, bloqueArchivo.size);
		log_trace(logger, "Solicitud de guardar bloque %d", bloqueId);
		success = setBloque(bloqueId, data);
		if(success == 0)
		log_trace(logger, "Success: %d", success);
		empaquetar(socketFs, mensajeRespuestaEnvioBloqueANodo, sizeof(int),&success);
		free(data);
		free(bloqueArchivo.envio);
		free(numeroBloque.envio);
		break;

	case mensajeNumeroLecturaBloqueANodo:
		memcpy(&bloqueId, numeroBloque.envio, sizeof(int));
		log_trace(logger, "Solicitud lectura de bloque %d", bloqueId);
		data = getBloque(bloqueId);
		empaquetar(socketFs, mensajeRespuestaGetBloque, strlen(data),data);
		free(data);
		free(numeroBloque.envio);
		break;

	case mensajeBorraDataBin:
		log_trace(logger, "Solicitud de borrado de databin");
		success = borrarDataBin();
		if (success == 1){
			log_trace(logger, "Databin eliminado correctamente");
			empaquetar(socketFs, mensajeRespuestaBorraDataBin, sizeof(int), &success);
			break;
		}
		log_error(logger, "Error al eliminar databin");
		success = 0;
		empaquetar(socketFs, mensajeRespuestaBorraDataBin, sizeof(int), &success);
		break;

	case mensajeConectado:
		empaquetar(socketFs, mensajeOk, sizeof(char), "a");
		break;


	default:
		//printf("mal %d\n",numeroBloque.idMensaje);
		//cpfrom /home/utnso/hola3.txt hola/chau
		exit(1);
		break;
	}
}

void escucharAlFS(int socketFs) {
	while (1) {
		//pedido = desempaquetar(socketFs);
		//memcpy(&bloqueMock, pedido.envio, sizeof(int));
		recibirMensajesFileSystem(socketFs);
		//free(envio);
		//success = setBloque(bloqueMock, envio);
		//empaquetar(socketFs, mensajeRespuestaEnvioBloqueANodo, sizeof(int),&success);
	}
}


