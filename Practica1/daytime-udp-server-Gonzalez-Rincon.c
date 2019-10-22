//Practica tema 5, Gonzalez Rincon Ivan
//Servidor
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

#define SIZEBUFF 30

int main(int argc, char *argv[]){
	if(argc>3){
		printf("Demasiados argumentos.\n");
		exit(EXIT_FAILURE);
	}
	/*Despues de comprobar la cantidad de argumentos, se comprueba
	si se ha introducido un puerto del servidor*/
	int puerto = 0;
	
	if(argc >=2){
		if(strcmp("-p", argv[1])==0){
			//Cogemos el numero de puerto del usuario
			sscanf(argv[2],"%d",&puerto);
			//htons(puerto);		

		}
	}else{
		struct servent *defaultPort;
		defaultPort = getservbyname("daytime", "udp");
		puerto = defaultPort->s_port;
	}

    //Declaramos el socket
	int socketResult;

	//Si la funcion de creacionn del Socket devuelve algo menor a 0, quiere decir que ha fallado.
	if((socketResult = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		error("Error al crear el socket\n");
	}
	// En caso de crear correctamente el socket, se informa.
	printf("El socket se ha creado correctamente\n");

    //Ahora vamos a enlazar el socket
	int bindingResult;
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = puerto;
	myaddr.sin_addr.s_addr = INADDR_ANY;

	//Comprobamos si surge algun error al enlazar el socket a la direccion local
	if((bindingResult = bind(socketResult, (struct sockaddr *) &myaddr, sizeof(myaddr))) < 0){
		//De haberlo, se notifica, e intentamos cerrar el Socket
		error("Error al enlazar el socket\n");
		//Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
		closeSocket(socketResult);
	}
	printf("El socket se ha enlazado correctamente\n");

    exit(EXIT_SUCCESS);
}