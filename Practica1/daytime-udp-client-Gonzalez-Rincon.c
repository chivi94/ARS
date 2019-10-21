//Practica tema 5, Gonzalez Rincon Ivan
//Cliente
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

void error(char * message){
	perror(message);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
	printf("Numero de argumentos: %i\n",argc);
	printf("Direccion:%s\n",argv[1]);
	if(argc<2){
		printf("Falta la direccion IP del servidor.\n");
		exit(EXIT_FAILURE);
	}else if(argc>4){
		printf("Demasiados argumentos.\n");
		exit(EXIT_FAILURE);
	
	}
	/*Despues de comprobar la cantidad de argumentos, se comprueba
	si se ha introducido un puerto del servidor*/
	int puerto;
	if(argc >=3){
		if(strcmp("-p", argv[2])==0){
			//Cogemos el numero de puerto del usuario
			sscanf(argv[3],"%d",&puerto);
			//htons(puerto);		

		}
	}
	//Comprobadas las opciones, pasamos a convertir la IP
	int value;
	struct in_addr address;
	value = inet_aton(argv[1],&address);
	if(value<=0){
		perror("Conversion IP:");
		exit(EXIT_FAILURE);
	}
	//Aqui cogemos los argumentos introducidos por el usuario
	printf("Direccion IP:%x\n",address.s_addr);

	//Declaramos el socket
	int socketResult;

	//Si la funcion de creacionn del Socket devuelve algo menor a 0, quiere decir que ha fallado.
	if((socketResult = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		error("Error al crear el socket\n");
	}
	// En caso de crear correctamente el socket, se informa.
	printf("El socket se ha creado correctamente\n");

	//Ahora vamos a enlazar el socket
	int bindingResult, closeResult;
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = 0;
	myaddr.sin_addr.s_addr = INADDR_ANY;

	if((bindingResult = bind(socketResult, (struct sockaddr *) &myaddr, sizeof(myaddr))) < 0){
		error("Error al enlazar el socket\n");
		closeResult = close(socketResult);
		if(closeResult <0){
			error("Error al cerrar el socket\n");
		}
		exit(EXIT_FAILURE);
	}
	printf("El socket se ha enlazado correctamente\n");
	
	//if(connect())


	exit(EXIT_SUCCESS);
	
	
}
