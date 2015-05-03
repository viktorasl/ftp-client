#define WIN32OS          // 1. Apibreziame varda 'WIN32OS'.
#include <windows.h>     // 2. Itraukiame pagrindini winapi '.h' faila.
#include <winsock.h>     // 3. Itraukiame darbui su soketu api skirta '.h' faila.
#include <time.h>        // 4. Itraukiame darn\bui su laiku skirta '.h' faila.
#include <conio.h> // 1. Itraukiame darba su konsole palengvinanti '.h'.

SOCKET establishClient (char* ip, int port);