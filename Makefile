all:
	mkdir -p bin
	gcc -Wall -c common.c
	gcc -Wall -c client.c
	gcc -Wall -c server.c
	gcc -Wall client.o common.o -o bin/client
	gcc -Wall server.o common.o -o bin/server

clean:
	rm -f *.o
	rm -rf bin
