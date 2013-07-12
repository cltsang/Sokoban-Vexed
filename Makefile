C = gcc
CFLAGS = -Wall
CLIB = ncurses
EXE = SokobanVexed MagicalSokobanVexed

all: ${EXE}

compile:
	${CC} ${CFLAGS} SokobanVexed.c -o SokobanVexed -l ${CLIB}
	${CC} ${CFLAGS} MagicalSokobanVexed.c -o MagicalSokobanVexed -l ${CLIB}

SokobanVexed:
	${CC} ${CFLAGS} SokobanVexed.c -o SokobanVexed -l ${CLIB}
	${CC} ${CFLAGS} MagicalSokobanVexed.c -o MagicalSokobanVexed -l ${CLIB}

clean:
	rm -f ${EXE}
