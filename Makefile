all: client echo-server

client : client.c
	gcc client.c -o client

echo-server : echo-server.c
	gcc echo-server.c -o echo-server

