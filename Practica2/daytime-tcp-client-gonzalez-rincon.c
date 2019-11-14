//Practica tema 6, Gonzalez Rincon Ivan
//Cliente
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

#define BUFFERSIZE 512

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

int main(int argc, char *argv[])
{
    //Creamos unas constantes, para comprobar si el usuario mete localhost como IP, poder interpretarla.
    const char *LOCALHOST = "localhost";
    const char *LOCALHOSTIP = "0.0.0.0";

    /*
    1. Encontrar la direccion IP y el nnumero de puerto de protocolo del servidor con el que se desea comunicar.
    */
    printf("Numero de argumentos: %i\n", argc);
    printf("Direccion:%s\n", argv[1]);
    if (argc < 2)
    {
        error("Falta la direccion IP del servidor.\n");
    }
    else if (argc > 4)
    {
        error("Demasiados argumentos.\n");
    }
    /*Despues de comprobar la cantidad de argumentos, se comprueba
	si se ha introducido un puerto del servidor*/
    //PUERTO: Variable para almacenar el puerto introducido como argumento
    int puerto = 0;

    if (argc >= 3)
    {
        if (strcmp("-p", argv[2]) == 0)
        {
            //Cogemos el numero de puerto del usuario
            sscanf(argv[3], "%d", &puerto);
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
    //Comprobadas las opciones, pasamos a convertir la IP
    //IP: Estructura para almacenar la ip.
    struct in_addr address;
    int ipError;
    //Comprobamos si el usuario al invocado al cliente con localhost
    if (strcmp(argv[1], LOCALHOST) == 0)
    {
        //En ese caso, le asignamos la IP correspondiente a localhost.
        printf("Localhost.\n");
        ipError = inet_aton(LOCALHOSTIP, &address);
    }
    //Si no, le asignamos la IP que ha introducido
    else
    {
        ipError = inet_aton(argv[1], &address);
    }

    //Comprobamos si hay error en la conversion de la IP
    if (ipError <= 0)
    {
        error("Conversion IP mal realizada.\n");
    }
    //Aqui cogemos los argumentos introducidos por el usuario
    printf("Direccion IP:%x\n", address.s_addr);
    /*
    2.Creamos el socket (punto de comunicacion entre el servidor y el cliente)
    */
    //SOCKET: Declaramos el socket. Variable donde manejaremos el mismo.
    int socketResult;

    //Si la funcion de creacion del Socket devuelve algo menor a 0, quiere decir que ha fallado.
    if ((socketResult = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("Error al crear el socket\n");
    }
    // En caso de crear correctamente el socket, se informa.
    printf("El socket se ha creado correctamente\n");

    /*
    3. Enlazamos el socket a una IP y puerto locales.
    4. Conectar el socket con el servidor (proporcionando tanto la IP como el puerto).
    */
    int connectResult;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = puerto;
    server_addr.sin_addr = address;

    //Comprobamos si surgen errores al conectar.
    printf("Comprobamos la conexion.\n");
    if ((connectResult = connect(socketResult, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        error("Error al conectar con el servidor.\n");
        closeSocket(socketResult);
    }
    printf("Se ha conectado correctamente con el servidor.\n");

    /*
    5. Recibir datos de acuerdo con el protocolo del nivel de aplicacion.
    */
    char datosRecibidos[BUFFERSIZE * 2] = "";
    int recvError = 0;

    //Comprobamos si surge algun problema al recibir datos del servidor.
    if ((recvError = recv(socketResult, &datosRecibidos, BUFFERSIZE, 0) < 0))
    {
        error("Fallo al recibir datos del servidor.\n");
        closeSocket(socketResult);
    }
    //Si todo ha ido bien, mostramos los datos que el servidor envia al cliente.
    printf("Datos recibidos:\n%s\n", datosRecibidos);

    /*
    6. Para finalizar con la comunicacion, cerramos la conexion con el servidor.
    */
   int closeError = 0;

   if((closeError = shutdown(socketResult, SHUT_RDWR)) < 0){
       error("Ha surgido un error al cerrar la conexion del cliente.\n");
       closeSocket(socketResult);
   }

   //Comprobamos que el cliente ha recibido todos los datos que ha mandado el servidor
   if ((recvError = recv(socketResult, &datosRecibidos, 0, 0) < 0))
    {
        error("Fallo al recibir datos del servidor.\n");
        closeSocket(socketResult);
    }

    //Por ultimom cerramos la conexion
    closeSocket(socketResult);

    exit(EXIT_SUCCESS);
}