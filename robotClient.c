#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"




// This is the main function that simulates the "life" of the robot
// The code will exit whenever the robot fails to communicate with the server
int main() {
  
 	// Set up the random seed
  	srand(time(NULL));
  
  	// Register with the server
  	int clientSocket, addrSize, bytesReceived;
 	struct sockaddr_in serverAddr;
 	char inStr[80]; // stores user input from keyboard
	unsigned char buffer[80]; // stores sent and received data
	int id = 0;
	float x = 0, y = 0;
	int direction = 0;
	
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
 	
  	// Send register command to server.  Get back response data
  	// and store it.   If denied registration, then quit.
  	
  	buffer[0] = REGISTER;
  	addrSize = sizeof(serverAddr);
  	sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
  	
  	
  	bytesReceived = recvfrom(clientSocket, buffer, 80, 0, (struct sockaddr *) &serverAddr, &addrSize);
  	
  	if (bytesReceived > 0) {
 
  		buffer[bytesReceived] = 0;
  	
  	}
  	
 	if (buffer[0] == OK) {
 	
 		//Receive additional bytes containing the robot id, randomly chosen position and direction
  		
  		//Reconstructs bytes and stores the data if robot was registered
  		
  		id = buffer[1];
  		x = (buffer[2] << 8) | (buffer[3]);
  		y = (buffer[4] << 8) | (buffer[5]);
  		direction = ((buffer[7] << 8) | buffer[8]);
  		
  		if (buffer[6] == 0) {
  			direction *= -1;
  		}	
  			
  	
  	} else {
  		printf("MAX_ROBOTS reached, unable to register. \n"); 
  	}
  	
  	// Go into an infinite loop exhibiting the robot behavior
  	while (1) {
  		
    		// Check if can move forward
    		buffer[0] = CHECK_COLLISION;
    		buffer[1] = id;
    		
    		unsigned char xHighByte = (int) x  >> 8;
		buffer[2] = xHighByte;
		
		unsigned char xLowByte = (int) x ^ (xHighByte << 8);
		buffer[3] = xLowByte;		
		 
		unsigned char yHighByte = (int) y >> 8;
		buffer[4] = yHighByte; 			
		
		unsigned char yLowByte = (int) y ^ (yHighByte << 8);
		buffer[5] = yLowByte;
		 			
		char directionSign = 1;
		  
		if (direction < 0) { 	
			directionSign = 0;
		} 
		buffer[6] = directionSign;
		 			
		unsigned char directionHighByte = direction >> 8;
		buffer[7] = directionHighByte;
		
		unsigned char directionLowByte = direction ^ (directionHighByte << 8);
    		buffer[8] = directionLowByte;
    		
    		sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
    		
    		// Get response from server.
		bytesReceived = recvfrom(clientSocket, buffer, 80, 0, (struct sockaddr *) &serverAddr, &addrSize);
		
    		// If ok, move forward
    		
    		if (buffer[0] == OK) {
    			
    			float newXPos = x + ROBOT_SPEED * cos((direction*PI)/180);
	 		float newYPos = y + ROBOT_SPEED * sin((direction*PI)/180);
    			x = newXPos;
    			y = newYPos; 	
    			
    			
    			// Send update to server
	    		buffer[0] = STATUS_UPDATE;
	    		buffer[1] = id;
	    		
	    		xHighByte = (int) x  >> 8;
			buffer[2] = xHighByte;
			
			xLowByte = (int) x ^ (xHighByte << 8);
			buffer[3] = xLowByte;		
			 
			yHighByte = (int) y >> 8;
			buffer[4] = yHighByte; 			
			
			yLowByte = (int) y ^ (yHighByte << 8);
			buffer[5] = yLowByte;
			 			
			directionSign = 1;
			  
			if (direction < 0) { 	
				directionSign = 0;
			} 
			buffer[6] = directionSign;
			 			
			directionHighByte = direction >> 8;
			buffer[7] = directionHighByte;
			
			directionLowByte = direction ^ (directionHighByte << 8);
	    		buffer[8] = directionLowByte;
    			sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
    			
    		
    		}
    		// Otherwise, we could not move forward, so make a turn.
    		// If we were turning from the last time we collided, keep
    		// turning in the same direction, otherwise choose a random 
    		// direction to start turning.
    		
    		else if (buffer[0] == NOT_OK_BOUNDARY || buffer[0] == NOT_OK_COLLIDE) {
    			
    			int newDirection = rand() % 2;
		 				
			//Rotate Left (Counter-clockwise)
			if (newDirection == 0) {
			 					
			 	if (direction - ROBOT_TURN_ANGLE <= -180) {
			 						
			 		direction *= -1;
			 					
			 	}
			 					
			 	direction -= ROBOT_TURN_ANGLE;
			 				
			//Rotate Right (Clockwise)
			} else {
			 				
				if (direction + ROBOT_TURN_ANGLE >= 180) {
			 					
 					direction *= -1;
		
				}
			 					
			 	direction += ROBOT_TURN_ANGLE;
			 					
			}
			
    		
    		}
    		
      		else if (buffer[0] == LOST_CONTACT) {
      			//If LOST_CONTACT is received, send a 0 byte to notify server robot client has shutdown and end loop
     			
      			buffer[0] = 0;
      			sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, addrSize);
      			
      			break;
      		
      		
      		} 
    
    		// Uncomment line below to slow things down a bit 
    		usleep(1000);
  	} 
  	
  	close(clientSocket); 
  	return 0;

}

