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
extern sem_t pedidoFS;

void enviarBloqueAFS(int numeroBloque) {

}//cpfrom /home/utnso/hola2.txt hola/chau

int setBloque(int numeroBloque, char* datos) {
	int fd = open(config.RUTA_DATABIN, O_RDWR);
	char* mapaDataBin = mmap(0, strlen(datos), PROT_READ | PROT_WRITE, MAP_SHARED, fd, mb*numeroBloque);
	memcpy(mapaDataBin, datos, strlen(datos));
	if (msync(mapaDataBin, strlen(datos), MS_SYNC) == -1)
	{
		perror("Could not sync the file to disk");
	}
	if (munmap(mapaDataBin, strlen(datos)) == -1)
	{
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);
	return 1;
}

char* getBloque(int numeroBloque) {
	int fd = open(config.RUTA_DATABIN, O_RDWR);
	char* mapaDataBin = mmap(0, mb, PROT_READ, MAP_SHARED, fd, mb*numeroBloque);
	char* datos = malloc(mb+1);
	memset(datos,0,mb+1);
	memcpy(datos, mapaDataBin, mb);
	if (munmap(mapaDataBin,mb) == -1)
	{
		close(fd);
		perror("Error un-mmapping the file");
		exit(EXIT_FAILURE);
	}
	close(fd);
	return datos;
}

void inicializarDataBin(){
	if (!validarArchivo(config.RUTA_DATABIN)){
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
	printf("soy el nodo %d\n", info.numeroNodo);
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

	switch (numeroBloque.idMensaje) {
	case mensajeNumeroCopiaBloqueANodo:
		bloqueArchivo = desempaquetar(socketFs);
		memcpy(&bloqueId, numeroBloque.envio, sizeof(int));
		data = malloc(bloqueArchivo.size + 1);
		memset(data, 0, bloqueArchivo.size + 1);
		memcpy(data, bloqueArchivo.envio, bloqueArchivo.size);
		setBloque(bloqueId, data);
		free(data);
		free(bloqueArchivo.envio);
		free(numeroBloque.envio);
		break;

	case mensajeNumeroLecturaBloqueANodo:
		memcpy(&bloqueId, numeroBloque.envio, sizeof(int));
		data = getBloque(bloqueId);
		printf("envioooo %d\n", strlen(data));
		empaquetar(socketFs, mensajeRespuestaGetBloque, strlen(data),data);
		free(numeroBloque.envio);
		break;

	default:
		//printf("mal %d\n",numeroBloque.idMensaje);
		//cpfrom /home/utnso/hola3.txt hola/chau
		break;
	}
}

void escucharAlFS(int socketFs) {
	int success = 1;
	while (1) {
		//pedido = desempaquetar(socketFs);
		//memcpy(&bloqueMock, pedido.envio, sizeof(int));
		recibirMensajesFileSystem(socketFs);
		//free(envio);
		//success = setBloque(bloqueMock, envio);
		//empaquetar(socketFs, mensajeRespuestaEnvioBloqueANodo, sizeof(int),&success);
	}
}


