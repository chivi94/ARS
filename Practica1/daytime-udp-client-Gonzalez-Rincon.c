//Practica tema 5, Gonzalez Rincon Ivan
//Cliente
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

	exit(EXIT_SUCCESS);
	
	
}
