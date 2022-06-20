#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

// global variables
using namespace std;
const int backLog = 3;
const int maxDataSize = 1460;

// define and return a TCP socket after doing error checking and setting relevant attributes
int client_setup(string clientIpAddr, int serverPort, string type_of_connection)
{
   int clientSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(!clientSocketFd)
   {
      cout<<"Error creating socket"<<endl;
      exit(1);
   }

return clientSocketFd; 

}

// given the client fd and server port and IP addr, connect the client to the server
void connect_to_server(int serverPort, string serverIpAddr, int clientSocketFd)
{
   struct sockaddr_in serverSockAddressInfo;
   serverSockAddressInfo.sin_family = AF_INET;
   serverSockAddressInfo.sin_port = htons(serverPort);
   //serverSockAddressInfo.sin_addr.s_addr = INADDR_ANY;
   inet_pton(AF_INET, serverIpAddr.c_str(), &(serverSockAddressInfo.sin_addr));

   memset(&(serverSockAddressInfo.sin_zero), '\0', 8);

   socklen_t sinSize = sizeof(struct sockaddr_in);
   int flags = 0;
   int dataRecvd = 0, dataSent = 0;
   struct sockaddr_in clientAddressInfo;
   char rcvDataBuf[maxDataSize], sendDataBuf[maxDataSize];
   string sendDataStr, rcvDataStr;

   int ret = connect(clientSocketFd, (struct sockaddr *)&serverSockAddressInfo, sizeof(struct sockaddr));
   if (ret<0)
   {
      cout<<"Error with server connection "<<endl;
      close(clientSocketFd);
      exit(1);
   }
   cin.ignore();   
   cout<<"Connected to server!"<<endl;

   return;
}

// this function received data from the server and stores it in rcvDataBuf array
void recv_data(int clientSocketFd, int flags)
{

   int dataRecvd;
   char rcvDataBuf[maxDataSize];

   while(1)
   {
      memset(&rcvDataBuf, 0, maxDataSize);
      dataRecvd = recv(clientSocketFd, rcvDataBuf, maxDataSize, flags);
      // if chatroom is full then exit
      if(!strcmp(rcvDataBuf,"#FULL"))
      {
         cout<<"Chatroom is full. Exiting"<<endl;
         close(clientSocketFd);
         exit(1);
      }
      if(dataRecvd>0) cout<<rcvDataBuf<<endl;
   }
}

// this function sends data taken from the terminal to the server
void send_data(int clientSocketFd, int flags)
{
   char sendDataBuf[maxDataSize];
   int dataSent;

   while(1)
   {
      memset(&sendDataBuf, 0, maxDataSize);
      cin.clear();
      cin.getline(sendDataBuf,maxDataSize);
      dataSent = send(clientSocketFd, sendDataBuf, strlen(sendDataBuf), flags);
      if(!strcmp(sendDataBuf, "bye"))
      {
         close(clientSocketFd);
         exit(1);
      }
   }
}


int main()
{

   // declaring and defining the server port and IP address
   uint16_t serverPort=3002;
   string serverIpAddr = "127.0.0.1", clientIpAddr = "127.0.0.1", type_of_connection="TCP";
   // declaring buffers for sending and receiving data
   char rcvDataBuf[maxDataSize], sendDataBuf[maxDataSize]; 
   int flags = 0;
   cout<<"------------------ Client ----------------------"<<endl;
   cout<<"IP address of the Server you want to connect with: ";
   cin>>serverIpAddr;
   cout<<"Port number of the Server you want to connect with: ";
   cin>>serverPort;

   // setup the client and connect to the server
   int clientSocketFd = client_setup(serverIpAddr, serverPort, type_of_connection);
   connect_to_server(serverPort, serverIpAddr, clientSocketFd);

   // beginning threads to asynchronously send and receive data from the server
   thread worker_r(recv_data, clientSocketFd, flags); // this thread is responsible to constantly checking if server 
                                    // has sent some data or not and then print data in the console.

   thread worker_s(send_data, clientSocketFd, flags); // this thread is responsible for taking input from the terminal
                                    // at any point of time and sending it to the server

   // stop the threads
   worker_r.join();
   worker_s.join();

   return 0;

}   
