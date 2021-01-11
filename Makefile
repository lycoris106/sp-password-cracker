all: cracker.c
	gcc -o cracker cracker.c -lcrypto -pthread

