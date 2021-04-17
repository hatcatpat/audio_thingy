all: main

clean:
	rm main

main: main.c
	gcc -Wall -lpthread -lm -ldl -D DEBUG -o main main.c -g
