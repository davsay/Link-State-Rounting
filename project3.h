//  header file for Programming Assignment 3
//  Joshua Zarin
//  CS457
//  December 6, 2016
#ifndef PA3_HEAD
#define PA3_HEAD

#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <sys/types.h>  //system call data types
#include <netinet/in.h> //structs for domain addresses
#include <arpa/inet.h>  //inet_addr
#include <netdb.h>      //hostent
#include <unistd.h>     //close sockets
#include <cstring>      //std::strlen
#include <stdio.h>      //std::(I/O)
#include <fstream>      //file io
#include <iostream>     //std:://cout
#include <string>       //std::string#include <strstream>
#include <sstream>      //std::stringstream
#include <ctype.h>      //isdigit
#include <vector>       //vector
#include <math.h>       //sqrt
#include <cstdlib>      //
#include <streambuf>    //stream buffer
#include <functional>   //bit_xor
#include <ctime>
#include <stdlib.h>
#include <signal.h>
#include <iomanip>
#include <pthread.h>
#include <cstdint>
#include <thread> 
#include <time.h>

using namespace std;//relieves std:: usage
#define DefaultPort 3450
//----------------------------------------------^IMPORTS^------------------------------------------
    struct routingTable{
        //short isPacket = 0;
        unsigned short size;
        short weights[20];
        short routerID; //the routing table for routerID.
        short senderID; //the router that is sending the message.
    };
    struct linkReq{
        bool ACK;//if ACK = false, then this is a request
        unsigned short sender;
        unsigned short receiver;
    };

    struct packet{
        short isPacket = 1;
        short src;
        short dest;
        short sentFrom;
        short receivedBy;
    };

void toCharStar(char * val, routingTable r){
    memcpy(val, &r, sizeof(r));
}

    //----------------------------------------------vGET_IPv-------------------------------------------
char* getIp(char* buffer, size_t buflength){
    char* f = (char*) "fail";
    if(buflength < 16){
        return f;
    }
    int descriptor = socket(AF_INET, SOCK_DGRAM, 0);    //open socket
    if(descriptor<0){
        return f;
    }
    const char* googleDNSIP = "8.8.8.8";
    uint16_t googlePort = 53;
    struct sockaddr_in server;
    int ssize = sizeof(server);
    memset(&server, 0, ssize);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(googleDNSIP);
    server.sin_port = htons(googlePort);
    int con = connect(descriptor, (const sockaddr*) &server, ssize);    //connect to google DNS
    if(con<0){
        return f;
    }
    sockaddr_in n;
    socklen_t nlen = sizeof(n);
    con = getsockname(descriptor, (sockaddr*) &n, &nlen);               //retrieving ip info 
    if(con<0){
        return f;
    }
    const char* p = inet_ntop(AF_INET, &n.sin_addr, buffer, buflength); //getting actual IP address
    close(descriptor);
    return (char*)p;            //return IP address
}

#endif  // PA3_HEAD