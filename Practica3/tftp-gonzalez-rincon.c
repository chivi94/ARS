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
#define PACKAGEPART 512
#define PACKAGETORCV 516
#define BYTESTOADD 2
#define MAXFILESIZE 100
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
#define MAXARGS 5

/**/
#define TRANSMISSIONMODE "octet"

//Cabeceras de funciones
unsigned char *allocateMemory(int sizeToAllocate);
void checkArguments(int argc, char *argv[]);
void error(char message[]);
void closeSocket(int result);
void help();
void verboseText(int programPart);
void checkResult(int result, char message[]);
unsigned char *initPackage();
unsigned char *packageType(int blockNumber, int type);
unsigned char *checkPckg(int pckgSize, unsigned char *package, int blockNumber);
void readMode(int socketResult);
void writeMode(int socketResult);

//Variables globales necesarias para el programa
int socketResult;
int communicationMode;
int pckgSize = 0;
char *nameOfFile;
char bufferOut[SIZEBUFF];
uint16_t serverPort;
struct in_addr serverIPAddress;
unsigned int auxPackage;
FILE *fichOut;
FILE *fichIn;
/*
Esta variable se usara para activar el modo paso a paso(verbose), 
solicitado por el enunciado de la practica. Puesta la variable
a valor 1(true), indica que este modo se encuentra activado*/
int verboseMode = 0;

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
    //Primero, vamos a comprobar los argumentos de nuestro programa.
    //Caso 1: 1 argumento. Nombre del programa. Mostramos la ayuda de como usar el comando y terminamos.
    if (argc <= MINARGS)
    {
        perror("Faltan argumentos.\n");
        help();
        exit(EXIT_FAILURE);
        //Caso 2, resto de argumentos. Puede ir desde 2 argumentos hasta el maximo, que son 6.
    }
    else if (argc > MAXARGS)
    {
        error("Demasiados argumentos.\n");
    }
    else
    {
        checkArguments(argc, argv);
        //Como ya tenemos los argumentos, procedemos a obtener numero de puerto
        struct servent *defaultPort;
        defaultPort = getservbyname("tftp", "udp");

        if (!defaultPort)
        {
            error("Error al bindear el puerto.\n");
        }

        if (verboseMode)
        {
            verboseText(6);
        }
        serverPort = defaultPort->s_port;

        //Una vez obtenido el numero de puerto, procedemos a establecer la conexion con el socket
        int socketResult;
        if ((socketResult = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            error("Error al crear el socket.\n");
        }

        if (verboseMode)
        {
            verboseText(7);
        }
        //Una vez creado el socket, procedemos a bindearlo.
        int bindingResult;
        struct sockaddr_in clientAddress;
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_port = 0;
        clientAddress.sin_addr.s_addr = INADDR_ANY;

        //Comprobamos si surge algun error al enlazar el socket a la direccion local
        if ((bindingResult = bind(socketResult, (struct sockaddr *)&clientAddress, sizeof(clientAddress))) < 0)
        {
            //De haberlo, se notifica, e intentamos cerrar el Socket
            error("Error al enlazar el socket\n");
            //Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
            closeSocket(socketResult);
        }

        if (verboseMode)
        {
            verboseText(8);
        }

        //Una vez enlazado todo, comenzamos la comunicacion
        switch (communicationMode)
        {
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
    }
    exit(EXIT_SUCCESS);
}

//Metodo para evitar repetir la llamada repetitiva de calloc
unsigned char *allocateMemory(int sizeToAllocate)
{
    return (unsigned char *)calloc(sizeToAllocate, sizeof(unsigned char));
}

//Metodo para comprobar los parametros de entrada
void checkArguments(int argc, char *argv[])
{
    int i = 0;
    for (i = 1; i < argc; i++)
    {
        printf("Estoy comprobando argumentos(%d).\n", i);
        if (i == 1)
        {
            if ((inet_aton(argv[i], &serverIPAddress)) <= 0)
            {
                error("Conversion IP.\n");
            }
        }
        //Modo lectura
        if (strcmp("-r", argv[i]) == 0)
        {
            communicationMode = 1;
        }
        //Modo escritura
        else if (strcmp("-w", argv[i]) == 0)
        {
            communicationMode = 2;
        }
        if (i == 3)
        {
            if ((nameOfFile = (char *)calloc(MAXFILESIZE, sizeof(char))) == 0)
            {
                error("Fallo al asignar la memoria necesaria para el nombre del fichero.\n");
            }
            strncpy(nameOfFile, argv[3], MAXFILESIZE);
        }

        //Verbose
        if (strcmp("-v", argv[i]) == 0)
        {
            verboseMode = 1;
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

//Metodo para mostrar ayuda al usuario sobre el uso del comando
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

/*Este metodo se encargara de imprimir el texto del modo verbose
dependiendo de las distintas partes del programa donde se llame.
*/
void verboseText(int programPart)
{
    switch (programPart)
    {
    case 1:
        printf("Enviada solicitud de lectura de %s a servidor tftp en %s.\n", nameOfFile, inet_ntoa(serverIPAddress));
        break;
    case 2:
        printf("Recibido bloque del servidor tftp.\n");
        printf("El bloque correspondiente a %d.\n", auxPackage);
        break;
    case 3:
        printf("Enviamos el paquete ACK correspondiente al bloque %d.\n", auxPackage);
        break;
    case 4:
        printf("Recibido el ACK proviniente del servidor.\n");
        printf("El bloque corresponde al codigo %d.\n", auxPackage);
        break;
    case 5:
        printf("Ultimo paquete mandado. Cerramos el fichero.\n");
        break;
    case 6:
        printf("Se ha enlazado el puerto del servidor.\n");
        break;
    case 7:
        printf("Se ha enlazado el Socket.\n");
        break;
    case 8:
        printf("Se ha bindeado la direccion local con el servidor.\n");
        break;
    default:
        //Aqui no deberia entrar nunca
        printf("-------");
        break;
    }
}

void checkResult(int result, char message[])
{
    if (result < 0)
    {
        fclose(fichIn);
        error(message);
        closeSocket(socketResult);
    }
}

/*
Este metodo se encargara de la tarea de inicializar los paquetes
de la aplicacion y formatearlos correctamente para que los datos
se almacenen como corresponde.
*/
unsigned char *initPackage()
{

    pckgSize = 0;
    unsigned char *resultPackage;
    /*En este caso usamos calloc en lugar de malloc
    porque nos interesa que se reserven espacios contiguos de memoria
    correspondientes al tamanio maximo de cada subconjunto del paquete total
    que se va a solicitar al servidor, y que todos esos espacios sean del mismo tipo.
    */
    resultPackage = allocateMemory(PACKAGEPART);
    if (resultPackage == 0)
    {
        error("Fallo al reservar memoria.\n");
    }

    /*
    A continuacion se crea el formato del paquete, 
    siguiendo el indicado en el enunciado de la practica.
    */
    resultPackage[1] = communicationMode;
    pckgSize = 2;
    int addSize = sprintf((char *)(resultPackage + BYTESTOADD), "%s", nameOfFile);
    if (addSize < 0)
    {
        error("Ha ocurrido un error al formatear el nombre del fichero.\n");
    }
    /*Las sumas que se realizan al tamanio del paquete se corresponden
    a desplazamientos en los bytes necesarios para respetar
    el formato de los paquetes de datos.
    */
    pckgSize = (pckgSize + addSize);
    pckgSize++;

    addSize = sprintf((char *)resultPackage + pckgSize, "%s", TRANSMISSIONMODE);
    if (addSize < 0)
    {
        error("Fallo en el formato del paquete.\n");
    }
    pckgSize += addSize;
    resultPackage[pckgSize] = 0;
    pckgSize++;
    return resultPackage;
}

/*
Este metodo se encargara de la creacion de los paquetes de datos.
*/
unsigned char *packageType(int blockNumber, int type)
{
    pckgSize = 0;
    unsigned char *newPackage;

    newPackage = allocateMemory(4);
    if (newPackage == 0)
    {
        error("Fallo al crear el paquete de datos.\n");
    }

    switch (type)
    {
    //Codigo 3 para paquete de datos
    case 0:
        newPackage[1] = 3;
        break;
    //Codigo 4 para ACK
    case 1:
        newPackage[1] = 4;
        break;
    }

    pckgSize = 2;

    //Desplazamos las posiciones de los paquetes, siguiendo las indicaciones dadas en clase.
    newPackage[2] = blockNumber / 256;
    newPackage[3] = blockNumber % 256;

    pckgSize += 2;

    return newPackage;
}

/*Metodo que se encargara de comprobar los tipos de paquete que
se envian y/o reciben entre el cliente y el servidor, para poder
llevar un control de los datos enviados.
*/
unsigned char *checkPckg(int pckgSize, unsigned char *package, int blockNumber)
{

    auxPackage = 0;
    int content;

    /*Tenemos que comprobar el contenido del paquete, para saber que tipo de paquete esta tratando el programa.
    Los casos son los codigos de operacion planteados por el enunciado:
    1 - RRQ.
    2 - WRQ.
    3 - Datos.
    4 - ACK.
    5 - Error.
    Para este bloque, manejaremos los 3 ultimos.
    */
    switch (package[1])
    {
    //Bloque de datos
    case 3:
        auxPackage = package[2] * 256 + package[3];
        if (verboseMode)
        {
            verboseText(2);
        }

        //Para comprobar el orden de los paquetes
        if (blockNumber >= auxPackage)
        {
            error("Error en el orden de envio de los paquetes.\n");
        }

        if (fichOut == NULL)
        {
            fichOut = fopen(nameOfFile, "wb");
        }
        fwrite(package + 4, 1, pckgSize - 4, fichOut);

        if (verboseMode)
        {
            verboseText(3);
        }
        return packageType(auxPackage, 1);
        break;

    //ACK
    case 4:
        auxPackage = package[2] + 256 + package[3];
        if (verboseMode)
        {
            verboseText(4);
        }

        if (blockNumber != auxPackage)
        {
            error("Error en el orden de envio de los paquetes.\n");
        }

        if (fichIn == NULL)
        {
            fichIn = fopen(nameOfFile, "rb");
        }
        unsigned char *ackResult = packageType(auxPackage + 1, 0);
        content = fread(ackResult + 4, 1, PACKAGEPART, fichIn);
        pckgSize += content;
        if (verboseMode)
        {
            auxPackage += 1;
            verboseText(3);
        }
        return ackResult;
        break;
    case 5:
        error("Ha surgido un error.\n");
        break;
    }

    return NULL;
}

//Metodo para activar el modo lectura del cliente
void readMode(int socketResult)
{
    //Vamos a mandar datos al servidor
    int sendResult, recvResult;
    unsigned char *out;
    unsigned char *in;

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = serverPort;
    serverAddr.sin_addr = serverIPAddress;

    out = initPackage();

    //Si el modo verbose esta activado -> informacion.
    if (verboseMode)
    {
        verboseText(1);
    }

    //Enviamos los datos y comprobamos si da fallo. En caso afirmativo, el programa termina.
    sendResult = sendto(socketResult, out, pckgSize, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (sendResult < 0)
    {
        error("Error al enviar datos al servidor\n");
        closeSocket(socketResult);
    }

    printf("Alojamos memoria para el paquete.\n");
    in = allocateMemory(PACKAGETORCV);
    if (in == 0)
    {
        error("Reserva en la memoria fallida para los datos provinientes del servidor.\n");
    }

    printf("Memoria alojada.\n");
    socklen_t addressLength = sizeof(serverAddr);
    int blockNumber = 0;
    //Comprobamos que se enlaza correctamente, cerrando la conexion en caso contrario.
    do
    {
        printf("Comenzamos el bucle do-while.\n");
        recvResult = recvfrom(socketResult, &in, PACKAGETORCV, 0, (struct sockaddr *)&serverAddr, &addressLength);
        if (recvResult < 0)
        {
            fclose(fichOut);
            error("Error al recibir datos del servidor.\n");
            closeSocket(socketResult);
        }
        printf("Recibimos datos.\n");
        //checkResult(recvResult, "Error al recibir datos del servidor\n");
        if (out != 0)
        {
            free(out);
        }
        printf("Pasamos el null del paquete de salida.\n");
        out = checkPckg(recvResult, in, blockNumber);
        printf("Hemos comprobado el paquete.\n");
        sendResult = sendto(socketResult, out, pckgSize, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        printf("Comprobamos resultado.\n");
        checkResult(sendResult, "Error al enviar datos al servidor.\n");
        blockNumber++;
    } while (recvResult - 4 == PACKAGEPART);
    if (verboseMode)
    {
        verboseText(5);
    }
    fclose(fichOut);
}

//Metodo para activar el modo escritura del cliente
void writeMode(int socketResult)
{
}