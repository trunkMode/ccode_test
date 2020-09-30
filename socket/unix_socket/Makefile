all: server client

server:
	$(CC) unix_socket_server.c -o $@ -lpthread

client:
	$(CC) unix_socket_client.c -o $@ -lpthread

clean:
	rm -rf *.o server client
