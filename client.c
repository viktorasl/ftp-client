#include <stdio.h>
#include <conio.h>

#include "client_backend.h"
#include "client.h"

#define SUCCESS 1
#define RESPONSE_ERROR 0
#define SOCKETS_ERROR -1

#define SERVER_IP	"79.98.28.28"
#define PORT		21
#define BUFFSIZE	1000

int getResponseCode(char* response){
	int code = 0;
	sscanf(response, "%d%*s", &code);
	return code;
}

int enterPassiveMode(const SOCKET* client, SOCKET* dataSocket){
	char temp[BUFFSIZE];
	
	char passive[5] = "PASV\n";
	int code;
	
	if ( INVALID_SOCKET == send(*client, passive, strlen(passive), 0) ){
		printf("Connection have failed\n");
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if ( INVALID_SOCKET == recv(*client, temp, BUFFSIZE, 0) ){
		printf("Connection have failed\n");
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	
	int ip[4];
	int port[2];
	
	sscanf(temp, "%d Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &code, &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
	
	char full_IP[17];
	int full_PORT = port[0] * 256 + port[1];
	
	snprintf(full_IP, 17, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	
	if ( ((code < 200) || (code >= 300)) || (INVALID_SOCKET == (*dataSocket = establishClient(full_IP, full_PORT))) ){
		return SOCKETS_ERROR;
	}
	
	return SUCCESS;
}

int reciveFile(SOCKET client) {
	
	SOCKET data;
	
	if (SUCCESS != enterPassiveMode(&client, &data)){
		return SOCKETS_ERROR;
	}
	
	char command[BUFFSIZE] = "RETR bird.avi\n";
	char temp[BUFFSIZE];
	int code;
	
	
	memset(command, 0, BUFFSIZE);
	char filename[BUFFSIZE];
	printf("File name: ");
	scanf("%s", &filename);
	snprintf(command, BUFFSIZE, "%s %s\n", "RETR", filename);

	
	if ( INVALID_SOCKET == send(client, command, strlen(command), 0) ){
		printf("Connection have failed\n");
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if ( INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0) ){
		printf("Connection have failed\n");
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	code = getResponseCode(temp);
	if ((code < 100) || (code >= 200)){
		printf("File transfer have failed\n");
		return RESPONSE_ERROR;
	}
	
	printf("Downloading...\n");
	
	FILE *f = fopen(filename, "wb");
	int got = 1;
	while (got > 0){
		char* temp = (char*)malloc(BUFFSIZE);
		if ( INVALID_SOCKET == (got = recv(data, temp, BUFFSIZE, 0)) ){
			fclose(f);
			return SOCKETS_ERROR;
		}
		fwrite(temp, 1, got, f);
		#ifdef DEBUG
			printf("%d ", got);
		#endif
		free(temp);
	}
	fclose(f);

	
	memset(temp, 0, BUFFSIZE);
	if ( INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0) ){
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	code = getResponseCode(temp);
	if ((code < 200) || (code >= 300)){
		printf("File transfer have failed\n");
		return RESPONSE_ERROR;
	}else {
		printf("Successfully downloaded.\n");
	}
	
	return SUCCESS;
}

int reciveList(SOCKET client) {
	SOCKET data;
	
	if (SUCCESS != enterPassiveMode(&client, &data)){
		return SOCKETS_ERROR;
	}
	
	char command[BUFFSIZE];
	char temp[BUFFSIZE];
	int code;
	
	memset(command, 0, BUFFSIZE);
	snprintf(command, BUFFSIZE, "%s\n", "LIST");
		
	if ( INVALID_SOCKET == send(client, command, strlen(command), 0) ){
		printf("Connection have failed\n");
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if ( INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0) ){
		printf("Connection have failed\n");
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	code = getResponseCode(temp);
	if ((code < 100) || (code >= 200)){
		printf("List transfer have failed\n");
		return RESPONSE_ERROR;
	}
	
	int got = 1;
	while (got > 0){
		char* temp = (char*)malloc(BUFFSIZE);
		if ( INVALID_SOCKET == (got = recv(data, temp, BUFFSIZE, 0)) ){
			return SOCKETS_ERROR;
		}
		fwrite(temp, 1, got, stdout);
		#ifdef DEBUG
			printf("%d ", got);
		#endif
		free(temp);
	}
	
	memset(temp, 0, BUFFSIZE);
	if ( INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0) ){
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	code = getResponseCode(temp);
	if ((code < 200) || (code >= 300)){
		printf("List transfer have failed\n");
		return RESPONSE_ERROR;
	}
	
	return SUCCESS;
}


int changeDirectory(SOCKET client){
	char temp[BUFFSIZE];
	char dirname[BUFFSIZE];
	
	printf("Go to: ");
	scanf("%s", &dirname);
	snprintf(temp, BUFFSIZE, "%s %s\n", "CWD", dirname);
		
	if (INVALID_SOCKET == send(client, temp, strlen(temp), 0)){
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if (INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0)){
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	if (getResponseCode(temp) != 250){
		printf("Error occurred accessing %s directory\n", dirname);
		return RESPONSE_ERROR;
	}else {
		printf("Current working directory is %s\n", dirname);
	}

	return SUCCESS;
}

void printMenu(){
	printf("\t1 : Download file\n\t2 : Change working directory\n\t3 : Show list of files and folders\n\t0 : Quit (Logout)\n");
}

int runSession (SOCKET client){
	
	char temp[BUFFSIZE];
	
	char username[BUFFSIZE];
	char username_cmd[BUFFSIZE];
	char password[BUFFSIZE];
	char password_cmd[BUFFSIZE];
	
	int code = 0;
	
	printf("Username: ");
	scanf("%s", username);
	snprintf(username_cmd, BUFFSIZE, "%s %s\n", "USER", username);
	
	printf("Password: ");
	//scanf("%s", password);
	
	char ch = _getch();
	int pw_ch = 0;
	while(ch != 13){
		password[pw_ch++] = ch;
		ch = _getch();
	}
	
	printf("\n");
	
	snprintf(password_cmd, BUFFSIZE, "%s %s\n", "PASS", password);
	
	if (INVALID_SOCKET == send(client, username_cmd, strlen(username_cmd), 0)){
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if (INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0)){
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	
	if (INVALID_SOCKET == send(client, password_cmd, strlen(password_cmd), 0)){
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if (INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0)){
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	
	code = getResponseCode(temp);
	if ((code < 200) || (code >= 300)){
		printf("Username and/or password is invalid\n");
		return RESPONSE_ERROR;
	}
	
	if (INVALID_SOCKET == send(client, "TYPE I\n", strlen("TYPE I\n"), 0)){
		return SOCKETS_ERROR;
	}
	memset(temp, 0, BUFFSIZE);
	if (INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0)){
		return SOCKETS_ERROR;
	}
	#ifdef DEBUG
		printf("%s", temp);
	#endif
	
	while (1){
		printMenu();
	
		scanf("%s", temp);
		int action = atoi(temp);
		
		switch (action){
			case 1: {
				if (SOCKETS_ERROR == reciveFile(client)){
					return SOCKETS_ERROR;
				}
			}
			break;
			case 2: {
				if (SOCKETS_ERROR == changeDirectory(client)){
					return SOCKETS_ERROR;
				}
			}
			break;
			case 3: {
				if (SOCKETS_ERROR == reciveList(client)){
					return SOCKETS_ERROR;
				}
			}
			break;
			case 0: {
				if (INVALID_SOCKET == send(client, "QUIT\n", strlen("QUIT\n"), 0)){
					return SOCKETS_ERROR;
				}
				memset(temp, 0, BUFFSIZE);
				if (INVALID_SOCKET == recv(client, temp, BUFFSIZE, 0)){
					return SOCKETS_ERROR;
				}
				printf("%s\n", temp);
				scanf("%s", &temp);

				return SUCCESS;
			}
			break;
		}
	}
	
	return SUCCESS;
}

int main (void){

	SOCKET client; // Client socket descriptor
	
	printf("Welcome to FTP client.\n");
	
	// If Windows - initialize Win Sockets
	#ifdef	WIN32OS
	WSADATA wsaData;
	if ( -1 == WSAStartup (MAKEWORD (1, 1), &wsaData) ){
		printf ("Client socket could not have been created.");
		exit( EXIT_FAILURE );
	}
	#endif
	
	// Initializing connection to server via server name and port
	char IP_address[17];
	printf("Server IP address: ");
	scanf("%s", &IP_address);
	printf("Connecting to server...\n");
	
	if ( INVALID_SOCKET == (client = establishClient(IP_address, PORT)) ){
		printf ("Client initialization have failed.\n");
		goto EXIT;
	}
	
	// Accepting greeting
	char greeting[BUFFSIZE];
	int greetingLength = 0;
	if ( SOCKET_ERROR == (greetingLength = recv(client, greeting, BUFFSIZE, 0)) ){
		goto EXIT;
	}else {
		#ifdef DEBUG
			fwrite(greeting, 1, greetingLength, stdout);
		#endif
	}
	
	// Start FTP client session
	while (RESPONSE_ERROR == runSession(client)){
	
	}
	
	
	EXIT:
	// If Windows - Closing Win Sockets
	#ifdef	WIN32OS
	if ( -1 == WSACleanup () ){
		printf ("Client socket could not have been closed.");
		exit ( EXIT_FAILURE );
	}
	#endif
	
	return 1;
}