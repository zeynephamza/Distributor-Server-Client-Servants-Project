all: main

main: servant.c server.c client.c distributor.c
	gcc -c -lpthread -Wall servant.c

	gcc -c -lpthread -Wall server.c
	gcc -c -lpthread -Wall client.c
	gcc -c -lpthread -Wall distributor.c
	gcc -o servant servant.c -lpthread -Wall

	gcc -o server server.c -pthread -Wall
	gcc -o client client.c -pthread -Wall
	gcc -o distributor distributor.c -lpthread -Wall
clean:
	rm *.o server servant client distributor *fifo*
