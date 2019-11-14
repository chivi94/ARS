  
//Practica tema 6, Gonzalez Rincon Ivan
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
void stopHandler(int signal);
void error(char message[]);
void closeSocket(int result);
void childFunction(struct sockaddr_in client_addr, int socket);


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

    /*
    1. Creamos un socket
    */

    //Si la funcion de creacionn del Socket devuelve algo menor a 0, quiere decir que ha fallado.
    if ((socketResult = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("Error al crear el socket\n");
    }
    // En caso de crear correctamente el socket, se informa.
    printf("El socket se ha creado correctamente\n");

    //Comprobamos el registro de senial Ctrl + C
    if (signal(SIGINT, &stopHandler) == SIG_ERR)
    {
        perror("Error al parar el servidor.\n");
    }
    /*
    2. Enlazamos a una direccion local bien conocida del servicio que ofrece el servidor.
   */
    int bindingResult;
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = puerto;
    myaddr.sin_addr.s_addr = INADDR_ANY;

    //Comprobamos si surge algun error al enlazar el socket a la direccion local
    if ((bindingResult = bind(socketResult, (struct sockaddr *)&myaddr, sizeof(myaddr))) < 0)
    {
        //De haberlo, se notifica, e intentamos cerrar el Socket
        error("Error al enlazar el socket\n");
        //Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
        closeSocket(socketResult);
    }
    printf("El socket se ha enlazado correctamente\n");

    /*
    3. Establecer el maximo de la cola de solicitudes pendientes
    */
    //Pasamos a la funcion el socket creado y le establecemos el limite de peticiones concurrentes a 5.
    listen(socketResult, 5);

    /*
   4. Llamar a la funcion que se queda esperando por un inicio de conexion. Esta funcion
   */
    struct sockaddr_in client_addr;
    int entrySocket;
    int childId;
    socklen_t clientLength = sizeof(client_addr);
    while (1)
    {
        if ((entrySocket = accept(socketResult, (struct sockaddr *)&client_addr, &clientLength)) < 0)
        {
            error("Socket no aceptado. \n");
            closeSocket(socketResult);
        }

        /*
  5. Crear una copia del proceso(hijo) para que atienda al cliente.
  */
        //Comprobamos si el fork se hace correctamente
        //Si falla, informamos
        childId = fork();
        if (childId < 0)
        {
            error("Error al hacer fork.\n");
            closeSocket(socketResult);
        }
        //Si el resultado de la funcion es 0, todo ha salido correctamente.
        if (childId == 0)
        {
            socketResult = entrySocket;
            childFunction(client_addr, socketResult);
            exit(EXIT_SUCCESS);
        }
    }
    exit(EXIT_SUCCESS);
}

/**
 * Función para manejar ctrl+C
 */
void stopHandler(int signalType)
{
    if (signalType == SIGINT)
    {
        if (close(socketResult) < 0)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
	exit(0);
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

void childFunction(struct sockaddr_in client_addr, int socketResult)
{
    
    char hostName[HOSTNAME];
    //Mandamos los datos al cliente
    system("date > /tmp/tt.txt");
    //Obtenemos el nombre del servidor
    gethostname(hostName, sizeof(hostName));
    strcat(hostName, ": ");
    fich = fopen("/tmp/tt.txt", "r");
    //Reservamos memoria para poder concatenar la informacion del nombre del servidor y la fecha
    char *result = malloc(HOSTNAME + SIZEBUFF + 1);
    strcpy(result, hostName);
    if (fgets(bufferOut, sizeof(bufferOut), fich) == NULL)
    {
        error("Error al abrir el fichero\n");
        closeSocket(socketResult);
    }
    //Concatenamos toda la informacion
    strcat(result, bufferOut);
    printf("La fecha y las hora son: %s\n", result);
    //Mandamos la informacion al cliente

    if (send(socketResult, result, 1024, 0) < 0)
    {
        error("Error al mandar datos al cliente\n");
        closeSocket(socketResult);
    }
    closeSocket(socketResult);
}