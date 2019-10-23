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


#define SIZEBUFF 128
#define HOSTNAME 256

void error(char message[]){
	perror(message);
	exit(EXIT_FAILURE);
}

void closeSocket(int result){
//Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
	int closeResult = close(result);
	if(closeResult <0){
		error("Error al cerrar el socket\n");
	}
	exit(EXIT_FAILURE);
}

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

	//Despues de enlazar el socket, comenzamos el bucle a la espera de conexiones.
	char bufferOut[SIZEBUFF];
	FILE *fich;
	struct sockaddr_in address;
	socklen_t addressLength = sizeof(address);
	int rcvResult;
	int sndResult;
	while(1){
		char bufferIn[SIZEBUFF];
		char hostName[HOSTNAME];
		rcvResult = recvfrom(socketResult, &bufferIn, 1024, 0, (struct sockaddr *)&address, &addressLength);
		if(rcvResult < 0){
			error("Error al recibir datos del cliente\n");
			closeSocket(socketResult);
		}
		//Mostramos los datos recibidos por parte del cliente
		printf("El cliente dice:%s\n",bufferIn);
		//Mandamos los datos al cliente
		system("date > /tmp/tt.txt");
		gethostname(hostName, sizeof(hostName));
		strcat(hostName,": ");
		fich = fopen("/tmp/tt.txt","r");
		char *result = malloc(HOSTNAME + strlen(bufferIn) + 1);
		strcpy(result,hostName);
		if(fgets(bufferOut, sizeof(bufferOut), fich)== NULL){
			error("Error al abrir el fichero\n");
			closeSocket(socketResult);
		}
		strcat(result,bufferOut);
		printf("La fecha y las hora son: %s\n",result);
		sndResult = sendto(socketResult, result, 1024, 0, (struct sockaddr *)&address, sizeof(address));
		if(sndResult < 0){
			error("Error al mandar datos al cliente\n");
			closeSocket(socketResult);
		}
	}
	closeSocket(socketResult);

    exit(EXIT_SUCCESS);
}