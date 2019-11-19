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
void error(char message[]);
void closeSocket(int result);
void help();

//Variables globales necesarias para el programa
int socketResult;
FILE *fich;
char bufferOut[SIZEBUFF];

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
    int i;
    // Iteramos sobre los argumentos para ver cuales han entrado.
    for (i = 1; i < argc; i++)
    {
        //Modo lectura
        if (strcmp("-r", argv[i]) == 0)
        {
        }
        //Modo escritura
        if (strcmp("-w", argv[i]) == 0)
        {
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

    exit(EXIT_SUCCESS);
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

void help(){
    printf("El uso del programa es el siguiente:\n");
    printf("Formato:nombreFicheroCompilado IP {-r|-w} archivo [-v][-h].\n");
    printf("\t- IP: Direccion IP del servidor.\n");
    printf("\t- {-r|-w}}: Modo de consulta hacia el servidor. Lectura o escritura, respectivamente\n");
    printf("\t- archivo: Nombre del archivo con el que trabajaremos.\n");
    printf("\t- [-v]: Modo verbose en el que se muestra una traza del cliente.\n");
    printf("\t- [-h]]: Muestra de ayuda para ver como usar el programa.\n");
}