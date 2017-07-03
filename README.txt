Joshua Zarin, David Sahud, Xi Wei
CS457 Project 3 README file
December 6, 2016

--------------------------------------------------------------------------------

Makefile Commands:
------------------
'Make': compiles both the manager and router cpp files into their respective executable and .o files

'Make manager': compiles the manager.cpp file into an executable manager file

'Make router': compiles the router.cpp file into an executable router file

'Make clean': deletes all executable, .o and .out (program output) files 

'Make cleanclear': deletels all executable, .o and .out (program output) files AND pipes the 'clear' command to clear the terminal view

'make target': create a FIRSTNAME_LASTNAME_P3.tar file for the program that encapsulates all .cpp files, .h files, Makefile and README.txt into

--------------------------------------------------------------------------------

Manager: 
--------
Source: manager.cpp
product: manager.o
input: ''.txt as a topology file
output: manager.out
-------------------
The manager program has only 1 argument: a .txt format topology file written into the course website program description's expected format.
The manager program will read through this file, create the correct amount of routers with the given connections in the topology file, pass
these connections to the routers it created, allow them to find their own shortest path trees after they communicate and executes Dijkstra's
algorithm, read the packet transfer section of the topology file, send the packets to their respective routers, let the routers transfer their
packets, wait for all routers to finish working, and then exit.

All steps of the above process are documented very well in the manager.out file, which the manager creates to record all of its work along with
any potential failure messages. 


--------------------------------------------------------------------------------

Router information:
-------------------
Source: router.cpp
product: router.o
input: id number (passed as input argument when spawned through a manager)
output: router.out
------------------
The router program has 1 argument, its id number, which is given to it by a manager. It is not capable of running on its own, since it is basically
an extension of the manager program because of the class website's program specifications, and it relies on establishing a connection to a manager
process and another router process in order to execute without errors. This program starts by connecting to the manager process, connecting to all 
of its neighboring routers that are given to it by the manager, communicating with all of its surrounding routers using an LSP broadcast, creating
its own shortest path routing table using dijkstra's algorithm and finally communicating with the manager process to forward any more packets.

All steps of the above process are documented very well in the routerx.out file, x being the router number, which the routers create to record all 
of their work along with any potential failure messages. 

--------------------------------------------------------------------------------

Additional notes:
-----------------
-This program takes roughly 30 seconds to properly execute, after which control will be given back to the terminal's command line. 

-After compiling the program with 'make', it can be run by typing './manager topology.txt' where 'topology.txt' is simply the name of the topology
file being used to test the program.

-The togology file being used to test the program must have the following format:

------------------

x     <----- number of routers being spawned
x y z <----- connection 1 (x = router connected from, y = router connected to, z = connection weight)
x y z <----- connection 2 (x = router connected from, y = router connected to, z = connection weight)
x y z <----- connection 3 (x = router connected from, y = router connected to, z = connection weight)
.
.     <----- any additional connections to be established
.
-1    <----- end of connection list
x y   <----- packet 1 (x = router sent from, y = router sent to)
x y   <----- packet 2 (x = router sent from, y = router sent to)
x y   <----- packet 3 (x = router sent from, y = router sent to)
x y   <----- packet 4 (x = router sent from, y = router sent to)
x y   <----- packet 5 (x = router sent from, y = router sent to)
-1    <----- end of packet list

------------------

-The program works perfectly according to the website's program specifications, ie all packets are forwarded correctly, algorithms are implemented
properly, etc, and all proper output will be written in the correct .out file.

--------------------------------------------------------------------------------