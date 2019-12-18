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
#define FORMAT "-> "
#define FIRSTSEQUENCE 0
#define ICMPHEADERTYPE 8
#define ICMPHEADERCODE 0
#define ICMPHEADERINITCHECKSUM 0
#define PAYLOADSIZE 64
#define RESPONSESIZE 92

//Cabeceras de funciones
void error(char message[]);
void help();
void checkArguments(int argc, char *argv[]);
void closeSocket(int result);
ECHORequest generateICMPRequest(int sequenceNumber);
unsigned short int calcChecksum(ECHORequest request);
void checkResponseType(ECHOResponse response);
void printResponseType(char message[], unsigned char type, unsigned char code);

//Variables globales
struct in_addr serverIPAddress;
int verboseMode = 0;
int socketResult;

int main(int argc, char *argv[])
{
    //Comprobamos los argumentos de entrada del programa
    checkArguments(argc, argv);

    //Si todo va correcto, continuamos con la ejecucion normal del programa
    //Establecemos la conexion con el socket

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

    if (sendResult < 0)
    {
        error("Error al enviar datos al servidor.\n");
        close(socketResult);
    }
    printf("Paquete ICMP enviado a %s\n.", inet_ntoa(serverIPAddress));

    //Recibimos la respuesta del servidor
    ECHOResponse response;
    socklen_t addressLength = sizeof(serverAddr);
    int rcvResult;
   
    rcvResult = recvfrom(socketResult, &response, RESPONSESIZE, 0, (struct sockaddr *)&serverAddr, &addressLength);
    if (rcvResult < 0)
    {
        error("Error al recibir datos del servidor.\n");
        closeSocket(socketResult);
    }
    //Mostramos la IP de origen de la respuesta
    printf("Respuesta recibida desde %s\n.", inet_ntoa(response.ipHeader.iaSrc));

    if (verboseMode)
    {
        printf(FORMAT "Tamaño de la respuesta: %d\n.", rcvResult);
        printf(FORMAT "Cadena recibida: %s\n.", response.payload);
        printf(FORMAT "Identifier (pid): %d\n.", response.ID);
        printf(FORMAT "TTL: %d\n.", response.ipHeader.TTL);
    }

    checkResponseType(response);
    closeSocket(socketResult);
    exit(EXIT_SUCCESS);
}

/*
Metodo para mostrar mensaje de error y parar el programa.
*/
void error(char message[])
{
    perror(message);
    exit(EXIT_FAILURE);
}

/*
Metodo para mostrar ayuda de como usar el programa.
*/
void help()
{
    printf("El uso del programa es el siguiente:\n");
    printf("Llamada: ./nombrePrograma IP [-v] [-h]\n");
    printf("- IP: Direccion IP a la que hacer ping.\n");
    printf("- [-v]: Parametro opcional que indica el modo verbose, para mostrar traza del comando.\n");
    printf("- [-h]: Parametro opcional que muestra la ayuda de como usar el comando.\n");
}

/*
Metodo para comprobar los argumentos de entrada
*/
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

/*
Metodo para comprobar el cierre del socket y terminar el programa.
*/
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

/*
Metodo para generar la peticion ICMP que se mandara a la direccion IP solicitada.
*/
ECHORequest generateICMPRequest(int sequenceNumber)
{
    ECHORequest result;

    result.icmpHeader.Type = ICMPHEADERTYPE;
    result.icmpHeader.Code = ICMPHEADERCODE;
    result.icmpHeader.Checksum = ICMPHEADERINITCHECKSUM;
    result.ID = getpid();
    result.SeqNumber = sequenceNumber;

    if (verboseMode)
    {
        printf(FORMAT "Generando cabecera ICPM.\n");
        printf(FORMAT "Type: %d\n", result.icmpHeader.Type);
        printf(FORMAT "Code: %d\n", result.icmpHeader.Code);
        printf(FORMAT "Identifier (pid): %d\n", result.ID);
        printf(FORMAT "Seq. number: %d\n", result.SeqNumber);
    }

    //Como se comento en uno de los laboratorios, vamos a generar una cadena aleatoria, que no da informacion, mas que cargar el payload de la request.
    int printResult;
    printResult = snprintf(result.payload, PAYLOADSIZE, "%s", "gpBPox8Rn3Ib0ukTlFYKnEVMKdFjogzAcWIfgWwPX5SpmyVT6QG9UMnr6bMxNbY");

    if (printResult < 0)
    {
        error("Error al formatear el payload.\n");
    }

    result.icmpHeader.Checksum = calcChecksum(result);
    if (verboseMode)
    {
        printf(FORMAT "Cadena a enviar: %s\n", result.payload);
        printf(FORMAT "Checksum: 0x%x\n", result.icmpHeader.Type);
        printf(FORMAT "Tamaño total del paquete: %d\n", 9 + printResult);
    }

    return result;
}

/*
Metodo para calcular el checksum del paquete. Se usara para comprobar que el envio ha sido correcto
*/
unsigned short int calcChecksum(ECHORequest request)
{
    int halfSizeofRequest;
    unsigned short int *vector;
    int i;
    unsigned int result = 0;

    halfSizeofRequest = sizeof(request) / 2;
    vector = (unsigned short int *)&request;
    for (i = 0; i < halfSizeofRequest; i++)
    {
        result = result + (unsigned int)vector[i];
    }
    result = (result >> 16) + (result & 0x0000ffff);
    result = (result >> 16) + (result & 0x0000ffff);
    return ~result;
}

/*
Metodo para comprobar el tipo de la respuesta proporcionada por el servidor
para poder identificar los posibles errores que surgen en esta.
*/
void checkResponseType(ECHOResponse response)
{
    unsigned char type = response.icmpHeader.Type;
    unsigned char code = response.icmpHeader.Code;
    switch (type)
    {
    //Echo reply
    case 0:
        printResponseType("Descripción de la respuesta: respuesta correcta (type %d, code %d).\n", type, code);
        break;
    //Destination Unreachable
    case 3:
        switch (code)
        {
        //Net Unreachable
        case 0:
            printResponseType("Descripción de la respuesta: Destination Network Unreachable (type %d, code %d)\n", type, code);
            break;
            //Host Unreachable
        case 1:
            printResponseType("Descripción de la respuesta: Destination Host Unreachable (type %d, code %d)\n", type, code);
            break;
        //Destination Network Unknown
        case 6:
            printResponseType("Descripción de la respuesta: Destination Network Unknown (type %d, code %d)\n", type, code);
            break;
        //Destination Host Unknown
        case 7:
            printResponseType("Descripción de la respuesta: Destination Host Unknown (type %d, code %d)\n", type, code);
            break;
        //Caso general
        default:
            printResponseType("Descripción de la respuesta: Destination unreachable (type %d, code %d)\n", type, code);
        }
        closeSocket(socketResult);
        exit(EXIT_FAILURE);
        break;
    case 11:
        printResponseType("Descripción de la respuesta: Time exceed (type %d, code %d)\n", type, code);
        closeSocket(socketResult);
        exit(EXIT_FAILURE);
        break;
    case 12:
        printResponseType("Descripción de la respuesta: Parameter problem (type %d, code %d)\n", type, code);
        closeSocket(socketResult);
        exit(EXIT_FAILURE);
        break;
    }
}

/*
Metodo para imprimir el mensaje de respuesta, junto con su tipo y su codigo. 
*/
void printResponseType(char message[], unsigned char type, unsigned char code)
{
    printf(message, type, code);
}