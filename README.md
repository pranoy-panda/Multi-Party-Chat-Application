# Multi-Party-Chat-Application
C++ based TCP Group Chat application using Socket Programming 

![](/screenshot.png)

## How to run
1. cd groupchat-TCP/
2. Compile:
Run the following commands in your terminal to compile the server and client programs
```
g++ server.cpp -std=c++11 -o server
g++ client.cpp -std=c++11 -pthread -o client
```
3. To run the server application, use this command in the terminal :
```
./server
```

* After this just follow the instructions that is displayed on the terminal to setup the server
* Always on.

4. Now, open another terminal and use this command to run the client application :
```
./client
```

* After this just follow the instructions that is displayed on the terminal to connect with the server. 
* Start of conversation of a client is denoted by a string of 40 dashes(i.e. ---------------------------------------)
* Client can just type in the message in terminal and hit enter to send message to all the clients.
* The messages from other clients would appear on the terminal as and when they arrive. 
* Sending and receiving of data takes place asynchronously due to the usage of threads.
* To leave the group chat, type in "bye". All the other clients will be notified of the same.

5. For opening multiple client applications, repeat step 4.

(You can have a look at the screenshot provided in the groupchat-TCP/ folder to better understand the client-server communication)
