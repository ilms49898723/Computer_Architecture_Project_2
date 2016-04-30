# makefile for Computer_Architecture_Project_2

CC := g++

CXXFLAGS := -std=c++11 -Os -Wall -Wextra

OBJS := InstDataBin.o \
        InstDataStr.o \
        InstDecoder.o \
        InstErrorDetector.o \
        InstImageReader.o \
        InstLookUp.o \
        InstMemory.o \
        InstPipelineData.o \
        InstSimulator.o \
        InstUtility.o \
        main.o

OUTPUT := pipeline

.SUFFIXS:
.SUFFIXS: .cpp .o

.PHONY: all pipeline clean

all: pipeline

pipeline: ${OBJS}
	${CC} ${CXXFLAGS} -o $@ ${OBJS}

.cpp.o:
	${CC} ${CXXFLAGS} -c $<

clean:
	-rm -f ${OBJS} ${OUTPUT}
