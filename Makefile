CC=gcc
CFLAGS=-Wall -Wextra -pedantic
TARGET=bfc
SRC=main.c

default:
	${CC} -o ${TARGET} ${SRC} ${CFLAGS}

debug:
	${CC} -o ${TARGET} ${SRC} ${CFLAGS} -ggdb -DAST_DEBUG
