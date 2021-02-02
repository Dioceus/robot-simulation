#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "simulator.h"


Environment environment;  // The environment that contains all the robots


/*

Helper function to determine if robot can move forwards to newX newY position

*/
int checkCollision(void *e, char robotId, float newX, float newY) {
	
	//Boundary Check
	if (newX <= ROBOT_RADIUS || newX >= ENV_SIZE - ROBOT_RADIUS|| newY <= ROBOT_RADIUS  || newY >= ENV_SIZE - ROBOT_RADIUS) {
		
		return NOT_OK_BOUNDARY;
	
	}
	
	Environment *env = (Environment *) e; 
	
	//Check if robot collides with other robots in the environment
	for (int i = 0; i < env->numRobots; i++) {
		
		if (i != robotId) {
			
			if (sqrt(pow((newX - env->robots[i].x),2) + pow((newY - env->robots[i].y),2)) <= 2 * ROBOT_RADIUS) {
			
				return NOT_OK_COLLIDE;
			
			}
		}
	
	}
	
	//If neither of the above happens, the robot is good to move forwards
	return OK;	
	
}

// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should repeatedly grab an incoming messages and process them. 
// The requests that may be handled are STOP, REGISTER, CHECK_COLLISION and STATUS_UPDATE.   
// Upon receiving a STOP request, the server should get ready to shut down but must
// first wait until all robot clients have been informed of the shutdown.   Then it 
// should exit gracefully.  
void *handleIncomingRequests(void *e) {
	char   online = 1;
	
  	 int serverSocket;
	 struct sockaddr_in serverAddr, clientAddr;
	 int status, addrSize, bytesReceived;
	 fd_set readfds, writefds;
	 unsigned char buffer[30];
	 unsigned char response[30];
	 
	 // Create the server socket
	 serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  	
  	if (serverSocket < 0) {
  		
  		printf("Server Error. Could Not Open Socket \n");
  		exit(-1);
  	
	}
  	
  	//Create the server socket
  	memset(&serverAddr, 0, sizeof(serverAddr)); // zeros the struct
 	serverAddr.sin_family = AF_INET;
 	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
 	serverAddr.sin_port = htons((unsigned short) SERVER_PORT);

	//Bind the server socket
	status = bind(serverSocket,(struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (status < 0) {
 		printf(" SERVER ERROR: Could not bind socket.\n");
 		exit(-1);
 	}


  	// Wait for clients now
	while (online) {
		
		FD_ZERO(&readfds);
	 	FD_SET(serverSocket, &readfds);
	 	FD_ZERO(&writefds);
	 	FD_SET(serverSocket, &writefds);
	 	status = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
	 	
	 	if (status == 0) { // Timeout occurred, no client ready
	 	}
	 	else if (status < 0) {
	 		printf("*** SERVER ERROR: Could not select socket.\n");
	 		exit(-1);
	 	}
	 	else {
	 		addrSize = sizeof(clientAddr);
	 		bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientAddr, &addrSize);
	 	
	 		if (bytesReceived > 0) {
	 			buffer[bytesReceived] = 0;
	 			
	 		}
	 		
	 		
	 		Environment *env = (Environment *) e;
	 		
	 		//If STOP request was received, the server should shutdown by updating the flag
	 		
	 		if(buffer[0] == STOP) {
	 			
	 			env->shutDown = 1;
	 			
	 			
	 			
	 		}
	 		
	 		else if (buffer[0]  == REGISTER) {
	 			
	 			int okResponse = 0;
	 			
	 			//Updates response depending on if it's possible to register another robot client
	 			if (env->numRobots == MAX_ROBOTS) {
	 				okResponse = NOT_OK;
	 				
	 			} else {
	 				okResponse = OK;
	 			}
	 			
	 			response[0] = okResponse; 
	 			
	 			if (okResponse == OK) {
	 				
	 				char foundPosition = 0;
	 				
		 			float randX =  0;
		 			float randY =  0;
		 			
		 			//Ensures initial start position for each robot is unique
		 			while (foundPosition == 0) {
		 				
		 				randX =  rand() % ((ENV_SIZE - ROBOT_RADIUS) + 1 - ROBOT_RADIUS) + ROBOT_RADIUS;
		 				randY =  rand() % ((ENV_SIZE - ROBOT_RADIUS) + 1 - ROBOT_RADIUS) + ROBOT_RADIUS;
		 				
		 				if (env->numRobots == 0) {
		 					
		 					foundPosition = 1;
		 					break;	
		 					
		 				}
		 				
		 				for (int i = 0; i < env->numRobots; i++) {
		 					
		 					if (sqrt(pow((randX - env->robots[i].x),2) + pow((randY - env->robots[i].y),2)) >= 2 * ROBOT_RADIUS) {
		 						
		 						foundPosition = 1;
		 						break;
		 					}
		 				
		 				}
		 			
		 			}
		 			
		 			//Generates random direction
		 			int randDirection = (int)(rand()/(double)RAND_MAX*((180) - (-180) +1 ) - 180);
		 			
		 			env->robots[env->numRobots].x = randX;
		 			env->robots[env->numRobots].y = randY;
		 			env->robots[env->numRobots].direction = randDirection; 
		 			
		 			//Divides values into high low bytes
		 			
		 			unsigned char randXHighByte = (int) randX  >> 8;
		 			
		 			unsigned char randXLowByte = (int) randX ^ (randXHighByte << 8);
		 			
		 			unsigned char randYHighByte = (int) randY >> 8;
		 			
		 			unsigned char randYLowByte = (int) randY ^ (randYHighByte << 8);
		 			
		 			//Creates additional byte to store the sign of the direction
		 			char randDirectionSign = 1;
		 			  
		 			if (randDirection < 0) {
		 				randDirectionSign = 0;
		 			} 
		 			
		 			unsigned char randDirectionHighByte = randDirection >> 8;
		 			unsigned char randDirectionLowByte = randDirection ^ (randDirectionHighByte << 8);
		 			
		 			response[1] = env->numRobots;
		 			response[2] = randXHighByte;
		 			response[3] = randXLowByte;
		 			response[4] = randYHighByte;
		 			response[5] = randYLowByte;
		 			response[6] = randDirectionSign;
		 			response[7] = randDirectionHighByte;
		 			response[8] = randDirectionLowByte;
		 			
		 			env->numRobots++;
		 			
	 			} else {
	 				//Response is NOT_OK - MAX_ROBOTS Reached
	 				env->shutDown = 1;
	 			
	 			}
	 			
	 			
	 		
	 		} else if (buffer[0]  == CHECK_COLLISION) {
	 			
	 			//Sends LOST_CONTACT if shutDown flag is set to 1
	 			if (env->shutDown == 1) {
	 				
	 				response[0] = LOST_CONTACT;
	 				
	 			
	 			} else {
	 			
		 			//Reconstruct location and direction from high low bytes
		 			
		 			float xVal = (buffer[2] << 8) | buffer[3];
		 			float yVal = (buffer[4] << 8) | buffer[5];
		 			int robotDirection = ((buffer[7] << 8) | buffer[8]);
		 			
		 			if (buffer[6] == 0) {
		 				robotDirection *= -1;
		 			}
		 			
		 			char robotId = buffer[1];
		 			
		 			//Determines if new location if a valid location
		 			float newXPos = xVal + ROBOT_SPEED * cos((robotDirection * PI)/180);
		 			float newYPos = yVal + ROBOT_SPEED * sin((robotDirection * PI)/180);
		 			
		 			int collisionResult = checkCollision(env, robotId, newXPos, newYPos);
	 				
	 				response[0] = collisionResult;
	 			
	 			}
	 			
	 		
	 		} else if (buffer[0] == STATUS_UPDATE) {
	 			
	 			float xVal = (buffer[2] << 8) | buffer[3];
	 			float yVal = (buffer[4] << 8) | buffer[5];
	 			int robotDirection = ((buffer[7] << 8) | buffer[8]);
	 			
	 			if (buffer[6] == 0) {
	 				robotDirection *= -1;
	 			}
	 			
	 			char robotId = buffer[1];
	 			
	 			//Updates location of robot to server
	 			
	 			env->robots[robotId].x = xVal;
	 			env->robots[robotId].y = yVal;
	 			env->robots[robotId].direction = robotDirection;
	 			
	 		
	 		} else if (buffer[0] == 0) {
	 			//If robot client shutdown - decrease number of robots
	 			
	 			env->numRobots--; 
	 		
	 		}
	 		
	 		//Exit Loop if shutdown flag has been activated and all robots are terminated
	 		if (env->shutDown == 1 && env->numRobots == 0) {
	 			
	 			online = 0;
	 		
	 		}
	 		
	 		sendto(serverSocket, response, sizeof(response), 0,(struct sockaddr *) &clientAddr, addrSize);
	 	
		}

  	}
  	
  	exit(0);
}

int main() {
	// So far, the environment is NOT shut down
	environment.shutDown = 0;
  
	// Set up the random seed
	srand(time(NULL));
	
	//Spawn threads
	pthread_t clientThread, redrawThread;
	
	pthread_create(&clientThread, NULL, handleIncomingRequests, &environment);
	pthread_create(&redrawThread, NULL, redraw, &environment);
	
	// Spawn an infinite loop to handle incoming requests and update the display
	while(1) {
		//If shutdown flag == 1 - end threads
		if (environment.shutDown == 1) {
			pthread_exit(NULL);
			break;
		
		}	
	
	}
	
	
	
}
