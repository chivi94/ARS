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
    int socketResult;

    //Si la funcion de creacionn del Socket devuelve algo menor a 0, quiere decir que ha fallado.
    if ((socketResult = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("Error al crear el socket\n");
    }
    // En caso de crear correctamente el socket, se informa.
    printf("El socket se ha creado correctamente\n");

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

    while(1){
        if((entrySocket = accept(socketResult, (struct sockaddr * )&client_addr, sizeof(client_addr))) < 0){
            error("Socket no aceptado. \n");
            closeSocket(socketResult);
        }
    }

    /*
  5. Crear una copia del proceso(hijo) para que atienda al cliente.
  */

    /*
 6. Proceso padre vuelve al paso 4.
 */
}