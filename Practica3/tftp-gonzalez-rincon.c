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
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZEBUFF 512
#define HOSTNAME 512

//Cabeceras de funciones
void error(char message[]);
void closeSocket(int result);

//Variables globales necesarias para el programa
int socketResult;
FILE *fich;
char bufferOut[SIZEBUFF];

int main(int argc, char *argv[])
{
    if (argc > 3)
    {
        printf("Demasiados argumentos.\n");
        exit(EXIT_FAILURE);
    }
    /*Despues de comprobar la cantidad de argumentos, se comprueba
	si se ha introducido un puerto del servidor*/
    int puerto = 0;

    if (argc >= 2)
    {
        if (strcmp("-p", argv[1]) == 0)
        {
            //Cogemos el numero de puerto del usuario
            sscanf(argv[2], "%d", &puerto);
            //htons(puerto);
        }
    }
    else
    {
        //PUERTO: Estructura para almacenar el numero del puerto.
        struct servent *defaultPort;
        defaultPort = getservbyname("daytime", "tcp");
        //Comprobamos que el puerto elegido no sea NULL
        if (!defaultPort)
        {
            error("Puerto no valido.\n");
        }
        puerto = defaultPort->s_port;
    }
}