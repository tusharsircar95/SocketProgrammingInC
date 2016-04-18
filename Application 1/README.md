#Stop And Wait Protocol With CRC (using CRC-8)

##Application 1
Aim is to implement a simple Stop-and-Wait based data link layer level logical channel between two nodes A and B using socket API, where node A and node B are the client and the server for the socket interface respectively. Error detection should be performed using Cyclic Redundancy Code (using CRC-8 as generator polynomial)



A brief summary of the working of the client and server: 

###Client
- Takes as input the server port and IP address(localhost 127.0.0.1 in this case) and a BER from  the user.
- Socket is created using the socket() call and then connection to the server is established with the connect() call. A option is set in the clientSocket to timeout after 5 sec if no message is recieved. Data is read and written using recv() and send() respectively.
- User is allowed to send n number of messages and each of them appended with CRC bits. Next message is sent only after an ACK is recieved else re-transmissions are done in case of NACK/timeout.
- Client closes its connection after sending these messages.

###Server
- Server takes as input the port and IP address(localhost) and a packet drop probability from the user.
- Socket is created using the socket() call and is binded to the address using bind(). A connection from client is accepted using accept(). Data is read and written using read() and send() respectively.
- Every message recieved is validated using CRC and ACK/NACK is sent accordingly. Server refrains from sending these based on the packet drop probability thus triggering a timeout at the client.
- fork() is used to create child processes that handle multiple clients.

##Implementation Details

###Generation Of T(x)
- User enters a message containing alphanumeric characters. Each character is then converted to its binary representation of 8 bits to generate the binary representation of the message.
- The given CRC polynomial is used to generate the CRC bits which is then appended to this message.
- A random number generator generates values in (0,1) say x for each bit in T(x) and if x <= BER then that bit is flipped(error). This message is sent to server.


###Stop And Wait
After sending a message the client waits for a reply from the server.
- If this reply is "ACK" then it proceeds to send the next message.
- If this reply is "NACK" then it implies that there was error in the message sent. Client re-transmits the message in this case.
- If no reply is recived for some time (set manually) then the timer expires and the recv() function returns -1. This implies that the reply packet was dropped. Client re-transmits the message in this case.

###Message Reception At Server
- Server reads the binary coded message sent by the client. It divides this by the CRC polynomial and calculates the remainder. If it's 0, then there was no error else the message was error free.
- The error free message is then read 8 bits at a time to convert it back to alpha-numeric form and displayed.
- Based on whether the message had error or not, ACK/NACK is sent back to the client. Similar to the client, a random number generator is used to decide whether this ACK/NACK packet will be dropped.

###Concurrent Server
- Concurrent server means that the server accepts connections from multiple clients and serves them concurrently.
- Whenever there is an incoming connection, a child process is forked using the fork() system call which then handles the connection.
- The parent process continues in the main loop to accept more connections and handles them similarly.

###Signal Handling
- When a server accepts a connection from a client then it reserves certain resources for the connection. Not releasing these sockets on termination will leave them blocked.
- signal-callback() is called whenever a kill signal (cntrl + C) is passed and this closes the connection and allows future servers to bind to the same port number.

