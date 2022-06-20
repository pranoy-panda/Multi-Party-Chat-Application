#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

using namespace std;

// global variables
const int backLog = 3;
const int maxDataSize = 1460;// max number of bytes that is sent
char rcvDataBuf[maxDataSize];

// converts char array to string
string convertToString(char* a, int size)
{
    string s = a;
    return s;
}

// finding index of value K in a vector v(K is always present in v)
int getIndex(vector<int> v, int K)
{
   auto it = std::find(v.begin(), v.end(), K);
   // calculating the index of K
   int index = it - v.begin();
   return index;
}

// define and return a TCP server socket after doing error checking and setting relevant attributes
// the returned socket would be listening for new connections
int server_setup(string serverIpAddr,int serverPort)
{
   int serverSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(!serverSocketFd)
   {
      cout<<"Error creating socket"<<endl;
      exit(1);
   }

   struct sockaddr_in serverSockAddressInfo;
   serverSockAddressInfo.sin_family = AF_INET;
   serverSockAddressInfo.sin_port = htons(serverPort);
   inet_pton(AF_INET, serverIpAddr.c_str(), &(serverSockAddressInfo.sin_addr));

   memset(&(serverSockAddressInfo.sin_zero), '\0', 8);
   printf("Server listening on IP %x:PORT %d \n",serverSockAddressInfo.sin_addr.s_addr, serverPort);

   // bind socket to server information
   int ret = bind(serverSocketFd, (struct sockaddr *)&serverSockAddressInfo, sizeof(struct sockaddr)); 
   if(ret<0)
   {
      cout<<"Error binding socket"<<endl;
      close(serverSocketFd);
      exit(1);
   }

   // server socket is now open for receiving new connection
   ret = listen(serverSocketFd, backLog);
   if(!serverSocketFd)
   {
      cout<<"Error listening socket"<<endl;
      close(serverSocketFd);
      exit(1);
   }      

   return serverSocketFd;

}

// this function accepts incoming connections to the socket given as input and does some error checking
// it returns the socket fd which the server will use for communicating with the client
int accept_connection(int serverSocketFd)
{
   socklen_t sinSize = sizeof(struct sockaddr_in);
   struct sockaddr_in clientAddressInfo;

   memset(&clientAddressInfo, 0, sizeof(struct sockaddr_in));
   
   int newClientFd = accept(serverSocketFd, (struct sockaddr *)&clientAddressInfo, &sinSize);
   if (!newClientFd)
   {
      cout<<"Error with new client connection "<<endl;
      close(serverSocketFd);
      exit(1);
   }

   return newClientFd;
}


// this function receives the data coming to the socket given as input
// it returns the number of bytes received
int handle_connection(int clientFd)
{
   int flags = 0;
   int dataRecvd = 0;  

   memset(&rcvDataBuf, 0, maxDataSize);
   dataRecvd = recv(clientFd, &rcvDataBuf, maxDataSize, flags);
   
   return dataRecvd;
}

// this function sends the data present in rcvDataBuf global array, to the socket given as input. 
// Some text formating is done before sending the data to visualize which client is transmitting the data.
void send_data(string parentClientName, int clientFd)
{
   int flags = 0;
   int dataSent = 0;   
   string msg = "";

   if(parentClientName!="") msg = parentClientName+" :"; // this additional text shows who sent the message
   msg.append(rcvDataBuf);
   char char_arr_msg[msg.length()+1];// 1 extra for the '/0' at the end in a char array
   strcpy(char_arr_msg, msg.c_str());// copy the contents of the string to the char arr

   dataSent = send(clientFd, char_arr_msg, strlen(char_arr_msg), flags);
}

// this function sends a message(present in rcvDataBuf) to all the connected clients(except the one which transmitted it)
void broadcast_msg_to_all_clients(string parentClientName, int serverSocketFd, vector<int> list_of_clientfd, vector<string> client_names)  
{

   for (int i = 0; i <list_of_clientfd.size(); ++i)
   {
      if(i!=serverSocketFd && (client_names[i]!=parentClientName))
      {
         send_data(parentClientName,list_of_clientfd[i]);
      }
   }
}


int main()
{

   // declaring and defining the server port
   uint16_t serverPort=3002; // any port number>1024 should work
   string serverIpAddr = "127.0.0.1", sendDataStr; // dummy server ip address(here it is the loop back address)
   int N; // stores max number of clients 
   vector<string> client_names; // stores the username of different clients
   vector<int> list_of_clientfd;
   int max_socket_seen, bytes_recv;

   // take input from the terminal to set the server port and server ip address
   cout<<"------------------ Server ----------------------"<<endl;
   cout<<"IP address of the Server: ";
   cin>>serverIpAddr;
   cout<<"Port number of the Server: ";
   cin>>serverPort;
   cout<<"Max number of clients that can group chat: ";
   cin>>N;

   // setup the server process
   int serverSocketFd = server_setup(serverIpAddr,serverPort);
   max_socket_seen = serverSocketFd;

   // declaring fd_set to keep track of open sockets and readable sockets
   fd_set current_sockets, ready_read_sockets;

   //initialize
   FD_ZERO(&current_sockets);
   FD_SET(serverSocketFd, &current_sockets); // add server socket to current socket

   // keep looping untill either server or client closes connection
   while(1)
   {

      // select is destructive so we need to use the copy of current_sockets
      ready_read_sockets = current_sockets;

      // use select() API to check which socket is currently readable
      if(select(FD_SETSIZE, &ready_read_sockets,NULL,NULL,NULL)< 0)
      {
         cout<<"Error"<<endl; 
         exit(1);
      }

      // iterate through the available sockets to check for their status
      for (int i = 0; i <= max_socket_seen; ++i)
      {

         if(FD_ISSET(i, &ready_read_sockets)) // check if the ith socket is ready for reading
         {
            if(i==serverSocketFd && list_of_clientfd.size()==N) // chatroom full
            {
               cout<<"New connection request rejected as group chat is currently full"<<endl;
               int newClientFd = accept_connection(serverSocketFd);
               send(newClientFd, "#FULL", 6, 0);// signal that chatroom is full
               close(newClientFd);
               continue;
            }
            if(i==serverSocketFd)// new connection and space available
            {
               int newClientFd = accept_connection(serverSocketFd);
               // ask the client for username
               send(newClientFd, "Enter the name of user: ", 25, 0);
               // reset the rcv buffer before storing new data
               memset(&rcvDataBuf, 0, maxDataSize); 
               // receive username from client
               recv(newClientFd, &rcvDataBuf, maxDataSize, 0); 
               // for indicate start of conversation at the client
               send(newClientFd, "---------------------------------------", 41, 0); 
               // after the above step, the rcvDataBuf contains the new clients name
               // add the name to the vector
               client_names.push_back(convertToString(rcvDataBuf,sizeof(rcvDataBuf)/sizeof(char))); 
               cout<<rcvDataBuf<<" connected"<<endl;
               // prepare msg to be broadcasted for all other clients to let them know a new client has joined
               sendDataStr = "############## ";
               sendDataStr.append(rcvDataBuf); sendDataStr = sendDataStr+" connected ##############";
               // reset the rcv buffer
               memset(&rcvDataBuf, 0, maxDataSize); 
               // convert string to char array
               strcpy(rcvDataBuf, sendDataStr.c_str());
               // broadcast
               broadcast_msg_to_all_clients("", serverSocketFd, list_of_clientfd,client_names);
               // add the newClient to the list of active clients
               list_of_clientfd.push_back(newClientFd);
               FD_SET(newClientFd, &current_sockets); // add the new client to the watch list
               if(newClientFd>max_socket_seen) max_socket_seen = newClientFd;
            }
            else
            {
               //TASK 1: handle connection for client socket i i.e. read from whoever wants to speak
               memset(&rcvDataBuf, 0, maxDataSize); // reset the rcv buffer
               bytes_recv = handle_connection(i);

               if(bytes_recv==-1) continue;

               // check if the client i wants to leave the group chat
               if(!strcmp(rcvDataBuf, "bye"))
               {
                  close(i);
                  // remove the socket i from watch list
                  FD_CLR(i,&current_sockets);
                  int index_of_socket_i = getIndex(list_of_clientfd,i);
                  string leaving_client = client_names[getIndex(list_of_clientfd,i)];
                  // display msg on server showing that a client has left
                  cout<<leaving_client<<" left the group chat"<<endl;
                  // remove the client
                  list_of_clientfd.erase(list_of_clientfd.begin()+index_of_socket_i);
                  client_names.erase(client_names.begin()+index_of_socket_i);
                  // broadcast the leaving msg on all active clients
                  memset(&rcvDataBuf, 0, maxDataSize); 
                  broadcast_msg_to_all_clients(leaving_client+" left the group chat", 
                                               serverSocketFd, list_of_clientfd,client_names);
                  continue;
               }                  

               //TASK 2: Then broadcast this msg to all the clients
               broadcast_msg_to_all_clients(client_names[getIndex(list_of_clientfd,i)], serverSocketFd, list_of_clientfd, client_names);            
            }
         }
      }

   }
}
