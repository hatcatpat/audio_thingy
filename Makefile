all: main

clean:
	rm main

main: main.c
	gcc -Wall -Wno-unused-function -lpthread -lm -ldl -D DEBUG -o main main.c
