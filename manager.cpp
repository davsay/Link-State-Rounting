//  main file manager.cpp for Programming Project 3
//  Joshua Zarin, Xi Wei, David Sahud
//  CS457
//  December 6, 2016
#include "project3.h"

int server_sockfd;
int router_sockfd;
struct hostent *he;
char hostname[128];
struct in_addr **addr_list;
int yes=1;
struct sockaddr_in man_addr;
socklen_t man_len;



int main(int argc, char* argv[]){
	time_t timer;
	time(&timer);
	if(argc != 2){
		cerr << "Usage error: program requires two args. Correct usage: 'manager <input_file_name>'" << endl;
		return -1;
	}
	string filename = argv[1];
	ifstream inFile;
	inFile.open(filename);
	if(!inFile.is_open()){
		cerr << "Error: topology file could not be opened." << endl;
		return -1;
	}
	ofstream outFile;
	outFile.open("manager.out");
	if(!outFile.is_open()){
		cerr << "Error: manager output file could not be opened." << endl;
		return -1;
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Input file, " << filename << ", and output file, manager.out, were both opened successfully." << endl;
	//----------------------------------------------^IO_ERROR_CHECKS^----------------------------------
	//----------------------------------------------vREAD/STORE_CONNECTION/WEIGHTv---------------------
	string fline;
	getline(inFile, fline);
	int numCons = stoi(fline);
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Total number of routers: " << numCons <<  "\n\n" <<endl;
	// build socket for connection
	int port = DefaultPort-1;
	gethostname(hostname, sizeof hostname);
	server_sockfd=socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
	if(server_sockfd<0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Error creating manager's socket" << endl;
		cerr << "Error creating manager's socket" << endl;
		return -1;
	}
	if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Manager Setsockopt error" << endl;
	   	cerr << "Manager Setsockopt error" << endl;
		return -1;
	}
	//----------------------------------------------vBUILDING_MANAGER_SOCKETv--------------------------
	man_addr.sin_family=AF_INET;
	man_addr.sin_port=htons(port);
	char buffer[140];
	string ip=getIp(buffer, 140);
	man_addr.sin_addr.s_addr = htons(INADDR_ANY);
	man_addr.sin_port = htons(port);
	man_len= sizeof(man_addr);
	if(bind(server_sockfd, (struct sockaddr *) &man_addr, sizeof(man_addr)) < 0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Error binding manager socket." << endl;
	 	cerr << "Error binding manager socket." << endl;
	 	return -1;
	}
	listen(server_sockfd,5);
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Creating a connection on " << ip << " port:" << port  << "\n\n" << endl;
	string s;
	char const* portbuffer;
	int i = 0;
	pid_t pid=0 ;
	//----------------------------------------------vBUILDING_ROUTING_TABLESv--------------------------
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Creating routing tables..." << endl;
	//cout << "Creating routing tables..." << endl;
	struct routingTable tables[numCons];
	int connections[numCons];
	for(i = 0; i < numCons; i++){//establish connection with router. Send routing Table.
		tables[i].size = numCons;
		for(int b = 0; b < tables[i].size; b++){
			tables[i].weights[b] = -1;
		}
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "\tsrc\t|\tdest|  weight" << endl;
	outFile << "------------------------" << endl;
	while(inFile.eof() == false){
		int src;
		int dest;
		int weight;
		inFile >> src;
		if (src == -1){ //end of network connection
			break;
		}
		inFile >> dest;
		if (dest == -1){ //end of network connection
			break;
		}

		inFile >> weight;
		if (weight == -1){ //end of network connection
			break;
		}
		// outFile << "src: " << src << ", dest: " << dest << ", weight: " << weight << endl;
		outFile << "\t" << src << "\t|\t" << dest << "\t|\t" << weight  << endl;
		tables[src].weights[dest] = weight;
		tables[dest].weights[src] = weight;
	}
	bool linksup[numCons];
	for(int j=0;j<numCons;j++){
		linksup[j]=false;
	}
	//--------------------------vFORKING_ROUTER_PROCESSES/CONNECTING_ROUTERSv--------------------------
	outFile << "\n\n" << endl;
	for(i = 0; i < numCons; i++){
		pid = fork();
		if(pid == 0){	//child process
			s = to_string(i);
			portbuffer = s.c_str();
			time(&timer);
			outFile << endl;
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Creating router " << i << endl;
			execl("./router", portbuffer, NULL);	//executing gambler
		}else if(pid > 0){	//parent process
				//creating a TCP connection to the new router------------------
	 		int connectCheck = accept(server_sockfd, (struct sockaddr *)&man_addr, &man_len);	
			connections[i]=connectCheck;
			if(connectCheck < 0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error accepting connection from router." << endl;
				cerr << "Error accepting connection from router." << endl;
				return -1;
			}else{
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Connected to router " << i << "!" << endl;
			}
			char msg[140];
			if(recv(connectCheck,msg,sizeof(msg),0)<0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error receiving information." << endl;
				cerr << "Error receiving information." << endl;
				return -1;
			}
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Received message: " << msg << endl;

			char bitpack [sizeof(tables[i])];
			bzero(bitpack, sizeof(bitpack));
			toCharStar(bitpack,tables[i]);
			cout<<"going to send routing table" <<endl;
			if(send(connectCheck, bitpack, sizeof(bitpack),0) < 0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			 	outFile << "Error sending informafion." << endl;
			 	cerr << "Error sending informafion." << endl;
			 	return -1;
			}
			cout<<"sent routing table" <<endl;
			


		}else{
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Error: process fork failed" << endl;
			cerr << "process fork failed" << endl;
			return -1;
		}
	}

	outFile << "\n" << endl;
	for (int i = 0; i < numCons; i++){
			int ackpid=0;
			if(recv(connections[i],&ackpid,sizeof(ackpid),0)<0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error recving pid." << endl;
			 	cerr << "Error recving pid" << endl;
			 	return -1;
			}
			linksup[ackpid]=true;
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile<<"Receiving ACK:" <<ackpid<<endl;
	}

	outFile << "\n" << endl;
	for (int i = 0; i < numCons; i++){
		if(linksup[i]==false){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Not all routers are connected" << endl;
			cerr << "Not all routers are connected" << endl;
			int startLSP = 0; //Send failure to routers, so that they exit correctly.
			for(int k = 0; k < numCons; k++){
				if(send(connections[k], &startLSP, sizeof(startLSP),0) < 0){
					time(&timer);
					outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			 		outFile << "Error sending LSP exchange start." << endl;
			 		return -1;
				}
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Send not all routers connect to router " << k << endl;
			}
			return -1;
		}
	}

	outFile << "\n" << endl;

	//When All ACK received, send ok to all router to start LSP exchange.
	for (int i = 0; i < numCons; i++){
		int startLSP = 1;
		if(send(connections[i], &startLSP, sizeof(startLSP),0) < 0){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 	outFile << "Error sending LSP exchange start." << endl;
		 	return -1;
		}
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Sending ok for router " << i << " to start LSP packet exchange." << endl;
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Waiting for all routers to finish dijkstras algorithm..." << endl;
	for (int i = 0; i < numCons; i++){
			int ackpid=0;
			if(recv(connections[i],&ackpid,sizeof(ackpid),0)<0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error recving pid." << endl;
			 	cerr << "Error recving pid" << endl;
			 	return -1;
			}

			if(ackpid == 0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error with router " << i << endl;
				outFile << "Exiting..." << endl;
				return -1;
			}
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Router: " << i << " is ready" <<endl;
	}

	outFile << "\n" << endl;

	while(inFile.eof() == false){
		//Sending Packets
		int src;
		int dest;
		inFile >> src;
		if (src == -1){ //end of file
			break;
		}
		inFile >> dest;
		if (dest == -1){ //end of file
			break;
		}
		struct packet pack;
		pack.src = src;
		pack.dest = dest;
		pack.sentFrom = -1;
        pack.receivedBy = -1;
        time(&timer);
        outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Sending packet to router " << pack.src << " with a final destination of router " << pack.dest << endl;

		for (int i = 0; i < numCons; i++){//temp
			int startPacket = src;//if src matches routerID, recv another packet from manager. Otherwise listen for udp.
			if(send(connections[i], &startPacket, sizeof(startPacket),0) < 0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			 	outFile << "Error sending startPacket exchange start." << endl;
			 	return -1;
			}
		}

		char bitpack [sizeof(pack)];
		bzero(bitpack, sizeof(bitpack));
		memcpy(bitpack, &pack, sizeof(pack));
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile<<"Sending packet to router " << pack.src << endl;
		if(send(connections[pack.src], bitpack, sizeof(bitpack),0) < 0){
		 	outFile << "Error sending packet." << endl;
		 	return -1;
		}
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile <<"Waiting for packet from router "  << pack.dest << endl;
		int fwdComplete = -1;
		if(recv(connections[pack.dest],&fwdComplete,sizeof(fwdComplete),0)<0){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Error recving pid." << endl;
		 	return -1;
		}
		if(fwdComplete == pack.dest){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Packet has arrived at its destination router " << pack.dest << ". Forwarding is complete." << endl;
		}else{
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Error unexpected dest - exiting" << endl;
			return -1;
		}

		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Waiting for all routers to complete their packet transmissions." << '\n' << '\n' << endl;
		for (int i = 0; i < numCons; i++){
			int ackpid=0;
			if(recv(connections[i],&ackpid,sizeof(ackpid),0)<0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error recving pid." << endl;
			 	cerr << "Error recving pid" << endl;
			 	return -1;
			}

			if(ackpid == 0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Error with router " << i << endl;
				outFile << "Exiting..." << endl;
				return -1;
			}
		}
	}

	//Nothing left for routers. Tell routers to complete
	for (int i = 0; i < numCons; i++){
		int startPacket = -1;
		if(send(connections[i], &startPacket, sizeof(startPacket),0) < 0){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 	outFile << "Error sending startPacket exchange start." << endl;
		 	return -1;
		}
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Manager process is finished, now closing i/o files." << endl;
	cout << "Manager process is finished, now closing i/o files." << endl;
	inFile.close();			//closing input and output files
	outFile.close();

	return 0;				//return 0 for success
}