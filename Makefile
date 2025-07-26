CC = gcc
CFLAGS = -Wall -g -fsanitize=address 

all : autograder diff

autograder : src/autograder.c
	${CC} $(CFLAGS) -o autograder src/autograder.c

diff : src/diff.c
	${CC} $(CFLAGS) -o diff src/diff.c

