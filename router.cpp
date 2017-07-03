//  main file router.cpp for Programming Project 3
//  Joshua Zarin, Xi Wei, David Sahud
//  CS457
//  December 6, 2016
#include "project3.h"
int pid;
int port;
int server_sockfd;
int router_sockfd;
int MYUDPSocket;
struct hostent *he;
char hostname[128];
struct in_addr **addr_list;
int yes=1;
struct sockaddr_in router_addr;
struct sockaddr_in MYUDProuter_addr, DestUDPSocket_addr;
socklen_t router_len;
socklen_t destaddr_len;
int main(int argc, char* argv[]){
	time_t timer;
	time(&timer);
	cout << "Hello from router.cpp, argc: " << argc << ", argv[0]: " << argv[0] << endl;
	if(argc == 1){
		pid= atoi(argv[0]);
		port=pid+DefaultPort;
		cout << port << endl;
	}else{
		cerr << "Error: Can not create the router process (invalid input arguments)." << endl;
		return -1;
	}
 	//creating output file-
 	string fNam = "router";
 	fNam += to_string(pid);
 	fNam += ".out";
 	ofstream outFile;
	outFile.open(fNam);
	if(!outFile.is_open()){
		cerr << "Error: router " << pid << "'s output file could not be opened." << endl;
		return -1;
	}
	gethostname(hostname, sizeof hostname);
  	server_sockfd=socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
  	if(server_sockfd<0){
  		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
   		outFile << "Can not create socket" << endl;
   		cerr << "Can not create socket" << endl;
   		return -1;
 	}
 	if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
 		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
   		outFile << "Error: Setsockopt from router " << pid << endl;
   		cerr << "Error: Setsockopt from router " << pid << endl;
   		return -1;
 	}
 	time(&timer);
 	outFile << "----TimeStamp----: " << asctime(localtime(&timer));
  	outFile << "Router " << pid << " successfully created!"<<endl;
 	router_addr.sin_family=AF_INET;
 	char buffer[140];
 	string ip=getIp(buffer, 140);
 	router_addr.sin_addr.s_addr = htons(INADDR_ANY);
 	router_addr.sin_port = htons(DefaultPort-1);
 	router_sockfd =connect(server_sockfd,(struct sockaddr *)&router_addr, sizeof(router_addr));
 	if(router_sockfd < 0){
 		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
 	 	outFile << "Error connecting router " << pid << " to manager socket." << endl;
 	 	cerr << "Error connecting router " << pid << " to manager socket." << endl;
 	 	return -1;
 	}
 	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Requesting routing table " << pid << endl;

	char msg[140]="Requesting routing table  .";
	msg[25] = 48 + pid;
	if(send(server_sockfd,msg,sizeof(msg),0)<0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Error sending from router " << pid << endl;
		cerr << "Error sending from router " << pid << endl;
	 	return -1;
	}

	routingTable myRT;
	char recBitPack[sizeof(myRT)];
	int value = recv(server_sockfd,recBitPack,sizeof(recBitPack),0);
	if (value < 0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	 	outFile << "Error receiving routing table from manager in router " << pid << endl;
	 	cerr << "Error receiving routing table from manager in router " << pid << endl;
	 	return -1;
	}
	memcpy(&myRT,recBitPack,sizeof(recBitPack));
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Received routing table from manager." << endl;

	MYUDPSocket=socket(AF_INET, SOCK_DGRAM, 0);
	MYUDProuter_addr.sin_family=AF_INET;
	MYUDProuter_addr.sin_addr.s_addr = htons(INADDR_ANY);
	MYUDProuter_addr.sin_port = htons(DefaultPort+pid);

	//Set a timeout so prevent router from getting stuck waiting for message. Important for when doing the packet sending portion. 
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 100000;
	if (setsockopt(MYUDPSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    perror("Error");
	}
	if(bind(MYUDPSocket, (struct sockaddr *) &MYUDProuter_addr, sizeof(MYUDProuter_addr)) < 0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Error binding UDP socket in router " << pid << endl;
		cerr << "Error binding UDP socket in router " << pid << endl;
		return -1;
	}
	sleep(1); 
	int * neighborACK;
	neighborACK = (int*) malloc(myRT.size*sizeof(int*));
	int numNeighbors = 0;		//finding the amout of neighbors this router has
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Routing table:" << endl;
	for(int i=0;i<myRT.size;i++){
		if(myRT.weights[i] != -1){
			numNeighbors++;
			neighborACK[i] = 0; //neighbor, ack is espected
		}else{
			neighborACK[i] = -1;//it's not a neighbor

		}
		outFile << "Weight[" << i << "] = " <<myRT.weights[i] << endl;
	}
	
	DestUDPSocket_addr.sin_family=AF_INET;
	DestUDPSocket_addr.sin_addr.s_addr = htons(INADDR_ANY);
	time(&timer);
	outFile << "\n\n----TimeStamp----: " << asctime(localtime(&timer));	 
	outFile << "Starting the UDP loop from router " << pid << endl;
	for(int i=0;i<myRT.size;i++){ //send Requests
		DestUDPSocket_addr.sin_port = htons(DefaultPort+i);
		time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Connecting from router " << pid << " to router " << i << " with weight [" << myRT.weights[i] << "]" << endl;
		if( myRT.weights[i] != -1){
		 	struct linkReq request;
		 	request.ACK = false;
		 	request.sender = pid;
	       	request.receiver = i;
		 	char bitpack [sizeof(request)];
			bzero(bitpack, sizeof(request));
			memcpy(bitpack, &request, sizeof(request));
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 	outFile << "1(" << pid << ") " << "request: ACK: " << request.ACK << " sender: " << request.sender << " receiver: " << request.receiver << endl;

		 	if (sendto(MYUDPSocket, bitpack, sizeof(bitpack), 0, (struct sockaddr *)&DestUDPSocket_addr, sizeof(DestUDPSocket_addr)) < 0) { //link request
		 		time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		outFile << "Error: UDP send failure" << endl;
		 		cerr << "Error: UDP send failure from router " << pid << endl;
		 		return -1;
		 	}

			 	//int outcount = 0;
		 	struct linkReq receive;
		 	char recBitPack[sizeof(receive)];
		 	bzero(recBitPack, sizeof(recBitPack));
		 	destaddr_len=sizeof(DestUDPSocket_addr);		 	
		 	if (recvfrom(MYUDPSocket, recBitPack, sizeof(recBitPack), 0, (struct sockaddr *)&DestUDPSocket_addr, &destaddr_len) < 0){
		 		outFile << "Error receiving" << endl;
		 		return -1;
		 	}		 	//cout << "recvlen: " << recvlen << endl;
		 	memcpy(&receive, recBitPack,sizeof(bitpack)); 
		 	time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));	
		 	outFile << "1(" << pid << ") " << "received: ACK: " << receive.ACK << " sender: " << receive.sender << " receiver: " << receive.receiver << endl;
		 	if(receive.ACK==true){
		 		neighborACK[receive.sender]=1; 
		 		time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		outFile << "update ACK[" << receive.sender << "] = " << neighborACK[receive.sender]<< endl; 

		 		outFile<< "1(" << pid << ") " << "ACK received Loop: " << i << " Sender: "<< receive.sender << endl; 
		 	}else{//This is a request. We must send an ACK back
		 		receive.ACK = true;
		 		receive.receiver = receive.sender;
		 		receive.sender = pid;
		 		bzero(recBitPack, sizeof(recBitPack));
		 		memcpy(recBitPack, &receive, sizeof(receive));
		 		if (sendto(MYUDPSocket, recBitPack, sizeof(recBitPack), 0, (struct sockaddr *)&DestUDPSocket_addr, sizeof(DestUDPSocket_addr)) < 0) { //link request
			 		time(&timer);
					outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			 		outFile << "1(" << pid << ") " << "Send failure" << endl;
			 		return -1;
		 		}
		 		time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		outFile << "Sent ACK to " << i << endl; 
		 	}
			 	
		 	bzero(bitpack, sizeof(bitpack));	 	
		 	if(recvfrom(MYUDPSocket, bitpack, sizeof(bitpack), 0, (struct sockaddr *)&DestUDPSocket_addr, &destaddr_len)< 0){
				outFile << "Error receiving" << endl;
		 		return -1;
		 	}
			//cout << "recvlen: " << recvlen << endl;
			struct linkReq receive2;
			memcpy(&receive2, bitpack,sizeof(bitpack)); 
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "1(" << pid << ") " << "received: ACK: " << receive2.ACK << " sender: " << receive2.sender << " receiver: " << receive2.receiver << endl;
	
		 	if(receive2.ACK==true){
		 		neighborACK[receive2.sender]=1; 
		 		time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		outFile << "update ACK[" << receive2.sender << "] = " << neighborACK[receive2.sender]<< endl; 

		 		outFile<< "2(" << pid << ") " << "ACK received Loop: " << i << " Sender: "<< receive2.sender << endl; 
		 	}else{//This is a request. We must send an ACK back
		 		//cout<< "2(" << pid << ") " << "Request from " << i << endl; 
		 		receive2.ACK = true;
		 		receive2.receiver = receive.sender;
		 		receive2.sender = pid;
		 		bzero(bitpack, sizeof(bitpack));
		 		memcpy(bitpack, &receive2, sizeof(receive2));
	
		 		if (sendto(MYUDPSocket, bitpack, sizeof(bitpack), 0, (struct sockaddr *)&DestUDPSocket_addr, sizeof(DestUDPSocket_addr)) < 0) { //link request
			 		time(&timer);
					outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			 		outFile << "2(" << pid << ") " << "Send failure" << endl;
			 		return -1;
		 		}
		 		time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		outFile << "Sent ACK to " << i << endl; 
		 	}			

			 	
		 	//neighborInfo[i] = DestUDPSocket_addr;
		 	
		}
		//neighborInfo[i]=NULL;
	}
	//cout<<"herer"<<endl;
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Finished UDP loop in router " << pid  << ", printing neighbors: " << endl;
	int fail = 0;
	for (int i = 0; i < myRT.size; i++){
		if(neighborACK[i] == 1){
		outFile << "NeighborACK [" << i << "] = " << neighborACK[i] << " (Neighboring router)" << endl;
		} else if(neighborACK[i] == -1){
		outFile << "NeighborACK [" << i << "] = " << neighborACK[i] << " (Non-neighboring router)" << endl;
		}else{
			outFile<< "(" << pid << ") " <<"Error receiving all ACKs: " << i << endl;
			cerr << "(" << pid << ") " <<"Error receiving all ACKs: " << i << endl;
			fail = 1;			
		}		
	}//if exits loop, then all ACKs are received.

	if (fail == 1){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Atleast 1 ACK fail" << endl;
		return -1;
	}
	
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	cout<< "(" << pid << ") " <<"Link establishment complete, sending message to manager" <<endl;
 	if(send(server_sockfd,&pid,sizeof(pid),0)<0){
 		 cerr<< "Error sending "<< endl;
 		 outFile<< "Error sending "<< endl;
	}

	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));	
	outFile << "Waiting for all routers to establish links." << endl;
	int startLSP=0;
	if(recv(server_sockfd,&startLSP,sizeof(startLSP),0)<0){
		outFile << "Error recving.1" << endl;
	 	return -1;
	}

	if(startLSP == 0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Not All Routers established links correctly. Terminating" << endl;
		return -1;
	}

	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "All Routers established links correctly. Begining LSP exchange.\n" << endl;

	//--------------------------***************************************************
	
	struct routingTable table[myRT.size];
	table[pid] = myRT;
	bool haveLSP[myRT.size];
	bool haveAllLSP = false;

	for(int i = 0; i < myRT.size; i++){
		haveLSP[i] = false;
		if(i == pid){
			haveLSP[i] = true;
		}
	}
	myRT.routerID = pid;
		//send own lsp to all neightbors.
	char LSPbuffer[sizeof(myRT)];
	for(int i = 0; i < myRT.size; i++){
		myRT.senderID = pid;
		bzero(LSPbuffer, sizeof(LSPbuffer));
		memcpy(LSPbuffer, &myRT, sizeof(myRT));
		DestUDPSocket_addr.sin_port = htons(DefaultPort+i);
		if(neighborACK[i] == 1){//This is a neighbor, send your lsp
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Sending myRT to neighbor routerID: " << i << endl;
			if (sendto(MYUDPSocket, LSPbuffer, sizeof(LSPbuffer), 0, (struct sockaddr *)&DestUDPSocket_addr, sizeof(DestUDPSocket_addr)) < 0) { //
					time(&timer);
					outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			 		outFile << "LSP - Error: UDP send failure to "<< i << endl;
			 		return -1;
		 	}
		}
	}
	//revieve from neightbors untill haveLSP for all routers is true.

	while(haveAllLSP == false){
		for(int i = 0; i < myRT.size; i++){
			if(haveAllLSP == false){
				struct routingTable recTable;
				bzero(LSPbuffer, sizeof(myRT));
				DestUDPSocket_addr.sin_port = htons(DefaultPort+i);
				if (recvfrom(MYUDPSocket, LSPbuffer, sizeof(LSPbuffer), 0, (struct sockaddr *)&DestUDPSocket_addr, &destaddr_len) < 0){
					time(&timer);
					outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
					outFile << "LSP - Error: UDP receive failure" << endl;
		 			return -1;
				}
				memcpy(&recTable, LSPbuffer, sizeof(recTable));
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Received Table - senderID: " << recTable.senderID << " routerID: " << recTable.routerID << endl;
				if(haveLSP[recTable.routerID] == false){
					//New LSP table
					table[recTable.routerID] = recTable;
					haveLSP[recTable.routerID] = true;
					bool LSPfail = true;
					for(int k = 0; k < myRT.size; k++){

						if(haveLSP[k] == false){
							LSPfail = false;
						}
					}
					if(LSPfail == true){
						haveAllLSP = true;
					}

					recTable.senderID = pid;
					bzero(LSPbuffer, sizeof(myRT));
					memcpy(LSPbuffer, &recTable, sizeof(recTable));

					for(int j = 0; j < myRT.size; j++){//Send received packet to all neighbors, excluding neighbor sender
						if(neighborACK[j] == 1 && j != recTable.senderID){
							time(&timer);
							outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
							outFile << "Sending Table - routerID: " << recTable.routerID << " to router " << j << endl;
							DestUDPSocket_addr.sin_port = htons(DefaultPort+j);
							if (sendto(MYUDPSocket, LSPbuffer, sizeof(LSPbuffer), 0, (struct sockaddr *)&DestUDPSocket_addr, sizeof(DestUDPSocket_addr)) < 0) { //
						 		outFile << "LSP - Error: UDP send failure" << endl;
						 		return -1;
						 	}
						}
					}
				}//end if
			}else{
				break;
			}
		}
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "All tables recevied" << '\n' << "Printing all routing tables:" << '\n' << '\n' << endl;

	//--------------------------***************************************************
	myRT.weights[pid] = 0;
	//Checking all if all rt are present.
	for (int i = 0; i < myRT.size; i++){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Routing Table for router " << i << endl;
		for (int node = 0; node < myRT.size; node++){
				outFile << "Weight[" << node << "] = " << table[i].weights[node] << endl;
		}
		outFile << '\n' << endl;
	}
	//table
	bool traversed[myRT.size];
	int path[myRT.size];
	for (int node = 0; node < myRT.size; node++){
		path[node] = pid;
		if(node != pid){
			traversed[node] = false;
		}else{
			traversed[node] = true;
		}
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << '\n' << "Starting dijkstras algorithm on router: " << pid << endl;

	for(int i = 0; i < myRT.size - 1; i++){
		//Select Min value.
		int index = -1;
		int minVal = -1;
		for (int node = 0; node < myRT.size; node++){
			if(traversed[node] == false && myRT.weights[node] != -1){
				if(minVal == -1){
					minVal = myRT.weights[node];
					index = node;
				}else{
					if(myRT.weights[node] < minVal){
						minVal = myRT.weights[node];
						index = node;
					}
				}
			}
		}
		if(minVal == -1){
			break;
		}
		traversed[index] = true;

		for (int node = 0; node < myRT.size; node++){
			if( ((table[index].weights[node] + myRT.weights[index] < myRT.weights[node]) || (myRT.weights[node] == -1 && table[index].weights[node]>-1) )&& table[index].weights[node] != -1 && traversed[node] == false){
				myRT.weights[node] = table[index].weights[node] + myRT.weights[index];
				path[node] = index;
				//outFile << "Updated" << endl;
			}
		}

	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Dijkstras algorithm complete." << endl;
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Shortest Path Table." << endl;
	outFile << "Router ID\t|\tWeight" << endl;
	outFile << "----------------------" << endl;
	for (int node = 0; node < myRT.size; node++){
		if(path[node] == pid){
			path[node] = node;
		}
		outFile << "\t" << node << "\t\t|\t" << myRT.weights[node]  << endl;
	}
	time(&timer);
	outFile << "\n\n\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Routing Table:" << endl;
	outFile << "Router ID\t|\tNext Hop" << endl;
	outFile << "----------------------" << endl;

	for (int node = 0; node < myRT.size; node++){
		if(path[node] == pid){
			path[node] = node;
		}
		outFile << "\t" << node << "\t\t|\t" << path[node]<< endl;
	}
	outFile << "----------------------" << endl;

	sleep(1);
	//This is to flush out current buffers. 
	struct routingTable extrart;
	char extrabuffer[sizeof(extrart)];
	for(int i = 0; i < myRT.size; i++){
		DestUDPSocket_addr.sin_port = htons(DefaultPort+i);

		if (recvfrom(MYUDPSocket, extrabuffer, sizeof(extrabuffer), 0, (struct sockaddr *)&DestUDPSocket_addr, &destaddr_len) < 0){
			//outFile << "No Extra data received - router: " << i << endl;
		}else{
			//outFile << "Extra data received - cleaning out buffer - router: " << i << endl;
			int extra = 1;
			while(extra == 1){
				if (recvfrom(MYUDPSocket, extrabuffer, sizeof(extrabuffer), 0, (struct sockaddr *)&DestUDPSocket_addr, &destaddr_len) < 0){
					//outFile << "No Extra data received - router: " << i << endl;
					extra = 0;
				}else{
					//outFile << "Extra data received - cleaning out buffer - router: " << i << endl;
					extra = 1;
				}
			}
		}
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Sending message to Manager" << endl;
	int correct = 1;
	if(send(server_sockfd,&correct,sizeof(correct),0)<0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
 		 cerr<< "Error sending "<< endl;
 		 outFile<< "Error sending "<< endl;
	}
	time(&timer);
	outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Waiting for incoming packets." << endl;
	startLSP=-1;
	while(true){//Keep listening for packets
		if(recv(server_sockfd,&startLSP,sizeof(startLSP),0)<0){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Error recving.2" << endl;
		 	return -1;
		}
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile<<"Received broadcast src address from manager: "<< startLSP<<endl;
		if(startLSP == -1){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Manager has no packets to send." << endl;
			break;
		}
		struct packet pack;
		char bitpack[sizeof(pack)];
		bzero(bitpack, sizeof(bitpack));
		if(startLSP == pid){
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Ok from manager, You are the src Address. Expecting packet from manager" << '\n' << '\n' << endl;
			if(recv(server_sockfd,&bitpack,sizeof(bitpack),0)<0){
				outFile << "Error" << endl;
			 	return -1;
			}
			sleep(1);
		}else{
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Ok from manager, ready to start receiving packets from routers" << endl;
			DestUDPSocket_addr.sin_port = htons(DefaultPort+path[startLSP]);
			if (recvfrom(MYUDPSocket, bitpack, sizeof(bitpack), 0, (struct sockaddr *)&DestUDPSocket_addr, &destaddr_len) < 0){
				time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
				outFile << "Timed Out" << endl;
				outFile<< "Ready for next iteration"<< '\n' << '\n' << endl;
				if(send(server_sockfd,&correct,sizeof(correct),0)<0){
			 		 outFile<< "Error timeout error "<< endl;
				}
				continue;
			}
		}
		memcpy(&pack, bitpack, sizeof(pack));
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		outFile << "Revieved Packet - src: " << pack.src << " dest: " << pack.dest << " sentFrom: " << pack.sentFrom << " receivedBy: " << pack.receivedBy << endl; 
		//forward packet if pid not dest

		if(pack.dest == pid){//done sending, tell manager you are complete.
			if(send(server_sockfd,&pid,sizeof(pid),0)<0){
				time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		 outFile<< "Error sending "<< endl;
			}
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Destination reached, messaging mangager" << endl;

		}else{//forward to next router.
			pack.sentFrom = pid;
			pack.receivedBy = path[pack.dest];
			char fwdbitPack[sizeof(pack)];
			bzero(fwdbitPack, sizeof(fwdbitPack));
			memcpy(fwdbitPack, &pack, sizeof(pack));
			time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
			outFile << "Forwarding packet to router " << path[pack.dest] << endl;
			outFile << "Forwarding Packet - src: " << pack.src << " dest: " << pack.dest << " sentFrom: " << pack.sentFrom << " receivedBy: " << pack.receivedBy << endl; 

			DestUDPSocket_addr.sin_port = htons(DefaultPort+path[pack.dest]);
			if (sendto(MYUDPSocket, fwdbitPack, sizeof(fwdbitPack), 0, (struct sockaddr *)&DestUDPSocket_addr, sizeof(DestUDPSocket_addr)) < 0) { //
		 		time(&timer);
				outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 		outFile << "Packet fwd - Error: UDP send failure" << endl;
		 		return -1;
		 	}
		 	time(&timer);
			outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 	outFile << "Packet sent to router " << pack.receivedBy << endl;

		}
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
		 outFile<< "Ready for next iteration"<< '\n' << '\n' << endl;

		if(send(server_sockfd,&correct,sizeof(correct),0)<0){
		time(&timer);
		outFile << "\n----TimeStamp----: " << asctime(localtime(&timer));
 		outFile<< "Error sending complete "<< endl;
		}
		startLSP = -1;
	}



 	cout << "Goodbye from router " << pid <<endl;

	free(neighborACK);
	time(&timer);
	outFile << "\n\n\n\n----TimeStamp----: " << asctime(localtime(&timer));
	outFile << "Router " << pid << " is now finished. Closing output file." << endl;
 	outFile.close();
	return 0;
}