#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


int main() {
 	
 	int clientSocket, addrSize, bytesReceived;
 	struct sockaddr_in serverAddr;
 	char inStr[80]; // stores user input from keyboard
	char buffer[80]; // stores sent and received data
  	// Register with the server
  	
 	// Create socket
 	clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
 	
 	if (clientSocket < 0) {
 		printf("ERROR: Could open socket.\n");
 		exit(-1);
 	}
 	
 	// Setup address
 	memset(&serverAddr, 0, sizeof(serverAddr));
 	serverAddr.sin_family = AF_INET;
 	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
 	serverAddr.sin_port = htons((unsigned short) SERVER_PORT);
  
  	// Send command string to server
  	
  	buffer[0] = STOP;
  	
  	addrSize = sizeof(serverAddr);
  	sendto(clientSocket, buffer, strlen(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
  	
  	

}
