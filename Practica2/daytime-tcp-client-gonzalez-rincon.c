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
    const char *LOCALHOST = "localhost";
    const char *LOCALHOSTIP = "0.0.0.0";

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
    int portValue;
    struct in_addr address;
    //Comprobamos si el usuario al invocado al cliente con localhost
    if (argv[1] == LOCALHOST)
    {
        //En ese caso, le asignamos la IP correspondiente a localhost.
        portValue = inet_aton(LOCALHOSTIP, &address);
    }
    portValue = inet_aton(argv[1], &address);
    if (portValue <= 0)
    {
        error("Conversion IP mal realizada.\n");
    }
    //Aqui cogemos los argumentos introducidos por el usuario
    printf("Direccion IP:%x\n", address.s_addr);

    //Declaramos el socket
    int socketResult;

    //Si la funcion de creacionn del Socket devuelve algo menor a 0, quiere decir que ha fallado.
    if ((socketResult = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("Error al crear el socket\n");
    }
    // En caso de crear correctamente el socket, se informa.
    printf("El socket se ha creado correctamente\n");

    //Ahora vamos a conectar el socket
    int connectionResult;
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = portValue;
    myaddr.sin_addr = address;

    //Comprobamos si surge algun error al enlazar el socket a la direccion local
    if ((connectionResult = connect(socketResult, (struct sockaddr *)&myaddr, sizeof(myaddr))) < 0)
    {
        //De haberlo, se notifica, e intentamos cerrar el Socket
        error("Error al enlazar el socket\n");
        //Comprobamos si al cerrar el socket algo va mal, notificando al usuario de ser asi.
        closeSocket(socketResult);
    }
    printf("El socket se ha enlazado correctamente\n");

    //Recibimos datos del servidor
    char datosRecibidos[1024] = "";
    //socklen_t longitudDireccion = sizeof(serverAddr);
    int recvResult;
    //Comprobamos que se enlaza correctamente, cerrando la conexion en caso contrario.
    recvResult = recv(socketResult, &datosRecibidos, 1024, 0);
    if (recvResult < 0)
    {
        error("Error al recibir datos del servidor\n");
        closeSocket(socketResult);
    }
    //Imprimimos los datos recibidos por el servidor
    printf("Datos recibidos:%s\n", datosRecibidos);

    //Cerramos la conexion
    int closeError = close(socketResult);
    if (closeError < 0)
    {
        error("Error al cerrar el socket\n");
    }

    // Cerramos la conexión establecida con el servidor
    int shutdownError = shutdown(socketResult, SHUT_RDWR);
    if (shutdownError < 0)
    {
        error("Fallo al cerrar la conexion.\n");
    }

    //Realizamos otro recv para comprobar que el cliente tiene todos los datos enviados por el servidor.
    recvResult = recv(socketResult, &datosRecibidos, 0, 0);
    if (recvResult < 0)
    {
        error("Error al recibir datos.\n");
        closeSocket(socketResult);
    }

    //Cerramos el socket.
    closeSocket(socketResult);
    exit(EXIT_SUCCESS);
}