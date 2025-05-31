all:
	gcc -Wall -c common.c
	gcc -Wall -c client.c
	gcc -Wall -c server.c
	gcc -Wall client.o common.o -o client
	gcc -Wall server.o common.o -o server

clean:
	rm -f *.o client server