all : ser par

ser : serial.c timer.c
	gcc -O2 -c timer.c
	gcc -O2 -c serial.c
	gcc -O2 -o ser serial.o timer.o

par : parallel.c timer.c
	gcc -O2 -c timer.c
	gcc -O2 -c parallel.c
	gcc -O2 -o par parallel.o timer.o -lpthread

clean :
	rm -f par ser timer.o parallel.o serial.o
