gcc client.c -o client
gcc server.c -o server
./server 9876 &
./client localhost 9876 'Hello, World!' 5
