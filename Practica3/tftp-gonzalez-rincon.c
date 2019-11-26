//Practica tema 7, Gonzalez Rincon Ivan
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

#define SIZEBUFF 512
#define HOSTNAME 512
#define MINARGS 2
#define MAXARGS 6

//Cabeceras de funciones
void checkArguments(int argc, char *argv[]);
void error(char message[]);
void closeSocket(int result);
void help();
void readMode(int socketResult);
void writeMode(int socketResult);

//Variables globales necesarias para el programa
int socketResult;
int communicationMode;
FILE *fich;
char bufferOut[SIZEBUFF];
uint16_t server;

/*
Formato de invocacion: client IP {-r|-w} archivo [-v][-h].
- client: Nombre del fichero compilado.
- IP: Direccion ip del servidor.
- {-r|-w}: Modo de consulta del cliente(lectura o escritura, respectivamente).
- archivo: Nombre del archivo en el que escribiremos o del que leeremos.
- [-v]: Modo verbose. Muestra una traza de las acciones del cliente.
- [-h] : Muestra una ayuda al cliente, para que vea como se usa el programa.
*/
int main(int argc, char *argv[])
{
    // Si los argumentos n o llegan al minimo, se acaba el programa.
    if (argc < MINARGS)
    {
        printf("Faltan argumentos.\n");
        exit(EXIT_FAILURE);
    }

    // Si los argumentos superan el maximo, termina el programa.
    if (argc > 6)
    {
        printf("Demasiados argumentos.\n");
        exit(EXIT_FAILURE);
    }

    //Comprobamos los argumentos que ha pasado el usuario como parametros del programa
    checkArguments(argc, &argv);

    //Como ya tenemos los argumentos, procedemos a obtener numero de puerto
    struct servent *defaultPort;
    defaultPort = getservbyname("tftp", "udp");

    if (!defaultPort)
    {
        error("Error al bindear el puerto.\n");
    }

    server = defaultPort->s_port;

    //Una vez obtenido el numero de puerto, procedemos a establecer la conexion con el socket
    int socketResult;
    if ((socketResult = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        error("Error al crear el socket.\n");
    }
    
    //Una vez creado el socket, procedemos a bindearlo.
	int bindingResult;
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = 0;
	myaddr.sin_addr.s_addr = INADDR_ANY;

	//Comprobamos si surge algun error al enlazar el socket a la direccion local
	if((bindingResult = bind(socketResult, (struct sockaddr *) &myaddr, sizeof(myaddr))) < 0){
		//De haberlo, se notifica, e intentamos cerrar el Socket
		error("Error al enlazar el socket\n");
		//Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
		closeSocket(socketResult);
	}

    //Una vez enlazado todo, comenzamos la comunicacion
    switch(communicationMode){
        case 01:
            readMode(socketResult);
            break;
        case 02:
            writeMode(socketResult);
            break;
        default:
            error("Fallo en la ejecucion del programa.\n");
            break;
    }
    closeSocket(socketResult);
    exit(EXIT_SUCCESS);
}

//Metodo para comprobar los parametros de entrada
void checkArguments(int argc, char *argv[]){
    int i;
    // Iteramos sobre los argumentos para ver cuales han entrado.
    for (i = 1; i < argc; i++)
    {
        //Modo lectura
        if (strcmp("-r", argv[i]) == 0)
        {
            communicationMode = 1;
        }
        //Modo escritura
        if (strcmp("-w", argv[i]) == 0)
        {
            communicationMode = 2;
        }
        //Verbose
        if (strcmp("-v", argv[i]) == 0)
        {
        }
        //Ayuda
        if (strcmp("-h", argv[i]) == 0)
        {
            help();
            exit(EXIT_SUCCESS);
        }
    }
}

//Metodo para imprimir un mensaje de error y terminar el programa.
void error(char message[])
{
    perror(message);
    exit(EXIT_FAILURE);
}

//Metodo para comprobar el cierre del socket y terminar el programa.
void closeSocket(int result)
{
    //Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
    int closeResult = close(result);
    if (closeResult < 0)
    {
        error("Error al cerrar el socket\n");
    }
    exit(EXIT_FAILURE);
}

void help()
{
    printf("El uso del programa es el siguiente:\n");
    printf("Formato:nombreFicheroCompilado IP {-r|-w} archivo [-v][-h].\n");
    printf("\t- IP: Direccion IP del servidor.\n");
    printf("\t- {-r|-w}}: Modo de consulta hacia el servidor. Lectura o escritura, respectivamente\n");
    printf("\t- archivo: Nombre del archivo con el que trabajaremos.\n");
    printf("\t- [-v]: Modo verbose en el que se muestra una traza del cliente.\n");
    printf("\t- [-h]]: Muestra de ayuda para ver como usar el programa.\n");
}