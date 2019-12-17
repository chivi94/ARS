// Practica tema 8, Renero Balgañón Pablo
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

#define PROGRAM_NAME "miping-Renero-Balganon"
#define FLECHA_VERDE "\033[1;32m->\033[0m "

void output(int const pos, char const *argv[], const int total);
void paramError();
void noParamError();
void ayuda();
void ipError(const char *);
ECHORequest icmpRequest(int);
unsigned short int checksum(ECHORequest);
void fin(int);

// Variables Globales
struct in_addr server_ip;
int verbose = 0;

int main(int argc, char const *argv[])
{
	// Bloque datos de entrada
	// Compruebo que hay parametros de entrada, si no los hay se imprime un mensaje avisando
	if (argc <= 1)
	{
		noParamError();
	}

	// Compruebo que si existen más de un parámetro de entrada
	if ((argc - 1) > 2)
	{
		paramError();
	}

	output(1, argv, argc - 1);

	if (argc == 3)
	{
		output(2, argv, argc - 1);
	}
	//Creo el paquete que se va a enviar
	ECHORequest echo_request = icmpRequest(0);
	// Fin bloque datos de entrada

	//Bloque del socket
	int id_sock;
	//Creamos el socket y comprobamos los posibles errores
	id_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (id_sock == -1)
	{
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	int error;
	// Fin bloque del socket
	struct sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = 0;
	remote_addr.sin_addr = server_ip;
	// Llamos a la directiva sendto para enviar los datos, en el caso de que falle cerramos el socket e imprimimos el error
	error = sendto(id_sock, &echo_request, sizeof(echo_request), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
	printf("Paquete ICMP enviado a %s\n", inet_ntoa(server_ip));
	if (error < 0)
	{
		perror("sendto()");
		fin(id_sock);
		exit(EXIT_FAILURE);
	}
	ECHOResponse echo_response;
	socklen_t len = sizeof(remote_addr);
	// Recibo los datos solicitados al servidor comprobando posibles errores
	error = recvfrom(id_sock, &echo_response, 92, 0, (struct sockaddr *)&remote_addr, &len);
	if (error < 0)
	{
		perror("recvfrom()");
		fin(id_sock);
		exit(EXIT_FAILURE);
	}
	printf("Respuesta recibida desde %s\n", inet_ntoa(echo_response.ipHeader.iaSrc));
	if (verbose)
	{
		printf(FLECHA_VERDE "Tamaño de la respuesta: %d\n", error);
		printf(FLECHA_VERDE "Cadena recibida: %s\n", echo_response.payload);
		printf(FLECHA_VERDE "Identifier (pid): %d\n", echo_response.ID);
		printf(FLECHA_VERDE "TTL: %u\n", echo_response.ipHeader.TTL);
	}

	// Parsea algunas combinaciones de tipo y codigo que he considerado relevantes y el resto se tratan como un error desconocido.
	switch (echo_response.icmpHeader.Type)
	{
	case 0:
		printf("Descripción de la respuesta: respuesta correcta (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		break;
	case 3:
		if (echo_response.icmpHeader.Code == 0)
		{
			printf("Descripción de la respuesta: Destination network unreachable (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		}
		else if (echo_response.icmpHeader.Code == 1)
		{
			printf("Descripción de la respuesta: Destination host unreachable (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		}
		else if (echo_response.icmpHeader.Code == 6)
		{
			printf("Descripción de la respuesta: Destination network unknown (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		}
		else if (echo_response.icmpHeader.Code == 7)
		{
			printf("Descripción de la respuesta: Destination host unknown (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		}
		else
		{
			printf("Descripción de la respuesta: Destination unreachable (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		}
		fin(id_sock);
		exit(EXIT_FAILURE);
	case 11:
		printf("Descripción de la respuesta: Time Exceeded (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		fin(id_sock);
		exit(EXIT_FAILURE);
	case 12:
		printf("Descripción de la respuesta: Parameter Problem: Bad IP header (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		fin(id_sock);
		exit(EXIT_FAILURE);
	default:
		printf("Descripción de la respuesta: unknown error (type %d, code %d)\n", echo_response.icmpHeader.Type, echo_response.icmpHeader.Code);
		fin(id_sock);
		exit(EXIT_FAILURE);
	}

	//Ceramos el socket
	fin(id_sock);
	return 0;
}

/**
 * Función que parsea los argumentos de entrada
 * pos posición a parsear
 * argv argumentos de entrada
 * total total de argumentos
 */
void output(int const pos, char const *argv[], const int total)
{
	//opción para mostrar la ayuda
	if (strcmp(argv[pos], "-h") == 0)
	{
		//En el caso de que la opción -h no esté en la pos 1 o tenga más amentos detras se lanza un error de entrada
		if (pos != 1 || total != 1)
		{
			paramError();
		}
		ayuda();
	}
	else if (strcmp(argv[pos], "-v") == 0)
	{
		if (pos != 2)
			paramError();

		verbose = 1;
	}
	else
	{
		//En el caso de que llegue aquí con una posición que no sea 1 se lanza un mensaje de error.
		if (pos != 1)
		{
			paramError();
		}
		//Traduce la ip y comprueba que es válida.
		if (inet_aton(argv[pos], &server_ip) == 0)
		{
			ipError(argv[pos]);
		}
	}
}

/**
 * Función que imprime un mensaje de error en los parámetros
 */
void paramError()
{
	printf("\nNumero de parametros erroneo\nPruebe './%s -h' para mas informacion\n\n", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}
/**
 * Función que imprime un mensaje de error cuando no hay parámetros de entrada
 */
void noParamError()
{
	printf("Cliente: debe indicar almenos una direción IP\nPruebe './%s -h' para más información.\n", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}

/**
 * Función que imprime la ayuda desplegada con la opción -h
 */
void ayuda()
{
	printf("\n\033[1;33mUso:\033[0m\n\t./%s direccion.IP.servidor [-v] \n\n", PROGRAM_NAME);
	printf("\033[1;33m-v\033[0m\n\t   Parametro opcional que hara que el cliente informe de todos los pasos necesarios para enviar o recibir un fichero.\n\n");
	exit(0);
}

/**
 * Función que imprime un mensaje de error si la ip no es válida
 */
void ipError(const char *in)
{
	printf("\nLa ip: '%s' no es válidad\n\n", in);
	exit(EXIT_FAILURE);
}

/**
 * Función que crea los paquetes de ICMP Request. 
 * 
 * param seq valor de tipo entero, contiene el numero de la secuencia.
 * return ECHORequest estructura con los datos a mandar
 */
ECHORequest icmpRequest(int seq)
{
	ECHORequest request;
	if (verbose)
	{
		printf(FLECHA_VERDE "Generando cabecera ICMP\n");
	}

	request.icmpHeader.Type = 8;
	if (verbose)
	{
		printf(FLECHA_VERDE "Type: %d\n", request.icmpHeader.Type);
	}

	request.icmpHeader.Code = 0;
	if (verbose)
	{
		printf(FLECHA_VERDE "Code: %d\n", request.icmpHeader.Code);
	}

	request.icmpHeader.Checksum = 0;

	request.ID = getpid();
	if (verbose)
	{
		printf(FLECHA_VERDE "Identifier (pid): %d\n", request.ID);
	}
	request.SeqNumber = seq;
	if (verbose)
	{
		printf(FLECHA_VERDE "Seq. number: %d\n", request.SeqNumber);
	}
	// La cadena ha sido autogenerada, no representa nada
	int error;
	error = snprintf(request.payload, 64, "%s", "czhQcNOEMqbWMMrCadhKsUavCWatwGehkdYFllUYSXwBDYcKkXQKcLKJCNqZczh");
	if (error < 0)
	{
		perror("sprintf()");
		exit(EXIT_FAILURE);
	}
	if (verbose)
	{
		printf(FLECHA_VERDE "Cadena a enviar: %s\n", request.payload);
	}

	request.icmpHeader.Checksum = checksum(request);
	if (verbose)
	{
		printf(FLECHA_VERDE "Checksum: 0x%x\n", request.icmpHeader.Checksum);
		printf(FLECHA_VERDE "Tamaño total del paquete ICMP: %d\n", 8 + error + 1);
	}

	return request;
}

/**
 * Función para calcular el checksum del paquete generado.
 * 
 * param request variable de tipo ECHORequest que contienen los datos sobre los que se va a calcular el checksum
 * return retorna el resultado de hacer el checksum
 */
unsigned short int checksum(ECHORequest request)
{
	int numShorts = sizeof(request) / 2;
	unsigned short int *puntero = (unsigned short int *)&request;
	int contador;
	unsigned int acumulador = 0;
	for (contador = 0; contador < numShorts; contador++)
	{
		acumulador = acumulador + (unsigned int)puntero[contador];
	}
	acumulador = (acumulador >> 16) + (acumulador & 0x0000ffff);
	acumulador = (acumulador >> 16) + (acumulador & 0x0000ffff);
	return ~acumulador;
}

/**
 * Funcion para cerrar el socket
 * 
 * param id_sock id del socket.
 */
void fin(int id_sock)
{
	int error;
	//Cerramos el soscket
	error = close(id_sock);
	if (error < 0)
	{
		perror("close()");
		exit(EXIT_FAILURE);
	}
}