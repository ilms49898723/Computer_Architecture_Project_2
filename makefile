# makefile for Computer_Architecture_Project_1

CC := g++
CFLAGS := -std=c++11 -O3 -Wall
OBJS := InstDataBin.o InstDataStr.o InstDecoder.o InstErrorDetector.o \
        InstImageReader.o InstLookUp.o InstMemory.o InstSimulator.o \
        InstUtility.o main.o

.SUFFIXS:
.SUFFIXS: .cpp .o

.PHONY: clean

all: single_cycle

single_cycle: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS}

.cpp.o:
	${CC} ${CFLAGS} -c $<

clean:
	-rm -f *.o single_cycle
