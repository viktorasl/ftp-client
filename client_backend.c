#include <stdio.h>

#include "client_backend.h"

SOCKET establishClient (char* ServerHostName, int port){
    SOCKET clientSocket;                     // Kliento soketo deskriptorius.
	
    //char ServerHostName [256] = ip;           // Hosto, kuriame paleistas serveris, vardas.
    unsigned short int ServerPortNumber = port;       // Porto numeris, kuriuo serveris laukia klientu.
	
    struct sockaddr_in ServerAddress;          // Serverio adreso struktura.
    struct hostent *ptrServerHostEntry = NULL; // Serverio hosto informacine struktura.
	
	// Windows sockets parameter should be chat
    const char yes = '1';

    // Pagal turima hosto varda susizinome hosto informacija.
    if ( NULL == (ptrServerHostEntry = gethostbyname (ServerHostName)) )
        return INVALID_SOCKET;
		
    // Inicializuojame soketo adreso struktura.
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons ( ServerPortNumber );
    ServerAddress.sin_addr = *(struct in_addr *)ptrServerHostEntry->h_addr;
	
    memset (&(ServerAddress.sin_zero), 0, 8);

    // Sukuriame pati soketo deskriptoriu.
    if ( INVALID_SOCKET == (clientSocket = socket (AF_INET, SOCK_STREAM, 0)) )
        return INVALID_SOCKET;

    // Uzdedame opcija soketo adreso ir porto pakartotinam panaudojimui.
    if ( SOCKET_ERROR == setsockopt (clientSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (yes)) )
        return INVALID_SOCKET;

    // Bandome prisijungti prie serverio.
    if ( SOCKET_ERROR == connect (clientSocket, (struct sockaddr *)&ServerAddress, sizeof (struct sockaddr)) )
    {
        closesocket (clientSocket);
        return INVALID_SOCKET;
    }

    // Isvedame informacija apie sekminga prisijungima.
	#ifdef DEBUG
		printf (
			"Connected successfully to host \'%s\' - (%s).\n",
			ServerHostName,
			inet_ntoa (*(struct in_addr *)ptrServerHostEntry->h_addr)
		);
	#endif
	
    // Graziname sukurto soketo deskriptoriu.
    return clientSocket;
}
