//Practica tema 8, Gonzalez Rincon Ivan
//Cliente

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include "ip-icmp-ping.h"

/*
Argumentos obligatorios:
- Nombre del programa.
En este caso se muestra la ayuda por defecto.
*/
#define MINARGS 1
/*
Maximo numero de argumentos:
- Los anteriores +
    - Modo verbose.
    - Ayuda para el usuario.
*/
#define MAXARGS 3
#define FIRSTSEQUENCE 0

//Cabeceras de funciones
void error(char message[]);
void help();
void checkArguments(int argc, char *argv[]);
void closeSocket(int result);
ECHORequest generateICMPRequest(int sequenceNumber);
unsigned short int calcChecksum(ECHORequest request);
void showResponse(unsigned char icmpHeaderType);

//Variables globales
struct in_addr serverIPAddress;
int verboseMode = 0;

int main(int argc, char *argv[])
{
    //Comprobamos los argumentos de entrada del programa
    checkArguments(argc, argv);

    //Si todo va correcto, continuamos con la ejecucion normal del programa
    //Establecemos la conexion con el socket
    int socketResult;
    if ((socketResult = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        error("Error al crear el socket.\n");
    }

    //Preparamos el envio al servidor
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 0;
    serverAddr.sin_addr = serverIPAddress;

    int sendResult;
    //Creamos la peticion ICMP
    ECHORequest request = generateICMPRequest(FIRSTSEQUENCE);
    //Mandamos la peticion
    sendResult = sendto(socketResult, &request, sizeof(request), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if(sendResult < 0){
        error("Error al enviar datos al servidor.\n");
        close(socketResult);
    }

    //Recibimos la respuesta del servidor
    ECHOResponse response;
    socklen_t addressLength = sizeof(serverAddr);
    int rcvResult;
    //¿92 sera el tamanio en bytes de la respuesta?
    rcvResult = recvfrom(socketResult, &response, 92, 0, (struct sockaddr *)&serverAddr, addressLength);
    if(rcvResult < 0){
        error("Error al recibir datos del servidor.\n");
        closeSocket(socketResult);
    }

    exit(EXIT_SUCCESS);
}

void error(char message[])
{
    perror(message);
    exit(EXIT_FAILURE);
}

void help()
{
    printf("El uso del programa es el siguiente:\n");
    printf("Llamada: ./nombrePrograma IP [-v] [-h]\n");
    printf("- IP: Direccion IP a la que hacer ping.\n");
    printf("- [-v]: Parametro opcional que indica el modo verbose, para mostrar traza del comando.\n");
    printf("- [-h]: Parametro opcional que muestra la ayuda de como usar el comando.\n");
}

void checkArguments(int argc, char *argv[])
{
    if (argc <= MINARGS)
    {
        perror("Argumentos insuficientes.\n");
        help();
        exit(EXIT_FAILURE);
    }

    if (argc > MAXARGS)
    {
        error("Demasiados argumentos.\n");
    }

    if (argc > MINARGS && argc <= MAXARGS)
    {
        int i = 0;
        for (i = 1; i < argc; i++)
        {
            if (i == 1)
            {
                if ((inet_aton(argv[i], &serverIPAddress)) <= 0)
                {
                    error("Conversion IP.\n");
                }
            }

            if (strcmp("-v", argv[i]) == 0)
            {
                verboseMode = 1;
            }

            if (strcmp("-h", argv[i]) == 0)
            {
                help();
                exit(EXIT_SUCCESS);
            }
        }
    }
}

//Metodo para comprobar el cierre del socket y terminar el programa.
void closeSocket(int result)
{
    //Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
    int closeResult = close(result);
    if (closeResult < 0)
    {
        error("Error al cerrar el socket\n");
        exit(EXIT_FAILURE);
    }
}

ECHORequest generateICMPRequest(int sequenceNumber)
{
    ECHORequest result;

    return result;
}

unsigned short int calcChecksum(ECHORequest request){
    return 0;
}

void showResponse(unsigned char icmpHeaderType){

}