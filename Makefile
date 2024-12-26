CC=gcc
CFLAGS=-Wall -Wextra -pedantic
TARGET=bfi
SRC=main.c

default:
	${CC} -o ${TARGET} ${SRC} ${CFLAGS}

debug:
	${CC} -o ${TARGET} ${SRC} ${CFLAGS} -ggdb
debug_ast:
	${CC} -o ${TARGET} ${SRC} ${CFLAGS} -ggdb -DAST_DEBUG
