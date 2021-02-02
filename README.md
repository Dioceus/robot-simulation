# robot-simulation# robot-simulation

Multi-threaded robot collision simulation. Each robot runs a thread and communicates its location with UDP. 

Clone the repository: 
```
git clone https://github.com/Dioceus/robot-simulation.git
```
Compiling and running:

1) Run the command: 
```
make 
```
This will compile and link environmentServer.c to an executable file environmentServer, compile and link robotClient.c to robotClient, and compile and link stop.c to stop

2) Run the command: 
```
./environmentServer&
```
This will run the executable file in the background

3) To run a robot client run: 
```
./robotClient&
```
This will run the executable file in the background

4) To stop the program run: 
```
./stop
```
