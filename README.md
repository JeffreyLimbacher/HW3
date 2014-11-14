COMP 429 Fall 2014 Programming Assignment 2
End-to-End Detection of Network Compression
Due: December 7th, 2014 at 21:00
Your final project has been broken up modularly to simplify your progression and development of the complete
system. Through this project, you will have an application that will be able to detect compression on intermediate
nodes between you and a host.
Phase I: (Mini Ping Program) Send and Receive an ICMP Message
(suggested deadline: 11/17/14)
In this phase, you will implement a single application that sends an echo ICMP message, and then listens for
and receives an ICMP echo reply message. Using a raw socket implementation, you will send a single ICMP echo
message to a given host. Immediately before sending the message, create and save a timestamp to record the time
the message is being sent. Next, you will wait and listen for an ICMP \Echo Reply" response from the host. When
you receive a response, save the message with a timestamp immediately. Then, check the contents of the ICMP
message header and conrm that it is an \Echo Reply" message. In the header, an \Echo Reply" messages has
its type set to 0. Finally, nd the dierence between the time you received the message and the time you sent the
message and print that out to the screen. This emulates a simplied version of ping!
Since you must use the raw socket to send an ICMP message, you must construct your own ICMP header. This
involves calculating the IP checksum, which will be provided in the skeleton code. You will also be provided with
a function to help you timestamp the packet accurately. To send an ICMP message, look into the functionality
of the sendto() function from the GNU Linux C library. Understand what the parameters of the function are
and how to set these parameters to accomplish the goal of this phase (sending an ICMP echo message). Also, you
should study the structure of an ICMP header to both understand how to create the header to send as well as how
to check the type of an ICMP reply.
This application will take as input from the command line the host address that you are sending a message to.
You will need to take the IP address from the command line and use it in you program as the host's IP address.
This application will print out the time dierence between sending and receiving an ICMP message for the given
host (which is an estimated RTT).
In this phase, you may assume that you will receive a reply from the host. Although it is possible the message you
send or the message you are waiting to receive will be dropped, you do not need to implement a timeout function
to handle this case. Simply assume that a reply will be received (and in nearly all cases, you will receive a reply).
Here is how to run the program with the host IP address set to 98.138.253.109 (if you want to ping one of the
Yahoo servers!):
./sender1 "98.138.253.109"
Phase II: Traceroute: Sending UDP Messages with Dierent TTLs
(suggested deadline: 11/17/14)
This phase builds upon the previous phase. Using raw socket programming, you must create two UDP messages.
In the rst message, you will create an IP header so that the TTL is very high (it is 8 bits long, so you can set it to
255). You will also create a random payload for the UDP packet. Using /dev/urandom, create a random payload
that is n bytes long.
Then, record the current time (just like the previous phase) and send this message to the host at port 9876, which
is not a well-known port. Because the port is not well known, the sent message will instigate a \Port Unreachable"
response from the host, much like traceroute. Using the same receiver from the previous phase, capture the
incoming ICMP packet, timestamp the packet, and check its type. The \Port Unreachable" response has the type
in the header set to 3. You also have to check the code in the header of the ICMP reply to determine if this is a
\Port Unreachable" reply. The code for a \Port Unreachable" reply is also 3. Calculate and print to the screen
the dierence between the time the reply was received and the time the time the message was sent.
Once the rst packet has been sent and the reply has been received and timestamped, change the IP header of
the UDP packet so that the TTL is now equal to 3. Again, record the time and send this message to the same
host and port, and wait for a reply. Since the TTL is low, the reply should be a \Time Exceeded" reply. A \Time
Exceeded" ICMP reply has the type set to 11. Save the incoming reply, timestamp the message, and conrm that
you have received a \Time Exceeded" ICMP reply. Calculate and print to the screen the dierence between the
time the reply was received and the time the time the message was sent.
This application will take as input from the command line the host address that you are sending a message to and
the value of n which is the number of bytes the random UDP payload will be. This application will print out the
time dierence between sending a UDP packet and receiving a reply when the TTL is both high and low.
Here is how to run the program with the host IP address set to 98.138.253.109 and the size of the UDP payload
equal to 1100:
./sender2 "98.138.253.109" 1100
Good luck!
