/*
 * main.cpp
 *
 *  Created on: 2016/03/08
 *      Author: LittleBird
 */

#include <cstdio>
#include <cstdlib>
#include "InstSimulator.h"
#include "InstImageReader.h"

int main(int argc, char **argv) {
    // load iimage, dimage
    unsigned iLen, dLen;
    unsigned pc, sp;
    unsigned inst[2048], memory[2048];
    iLen = lb::InstImageReader::readImageI("iimage.bin", inst, &pc);
    dLen = lb::InstImageReader::readImageD("dimage.bin", memory, &sp);
    // simulate
    FILE *snapShot, *errorDump;
    snapShot = fopen("snapshot.rpt", "w");
    errorDump = fopen("error_dump.rpt", "w");
    if (!snapShot || !errorDump) {
        exit(EXIT_FAILURE);
    }
    lb::InstSimulator simulator;
    simulator.loadImageI(inst, iLen, pc);
    simulator.loadImageD(memory, dLen, sp);
    simulator.simulate(snapShot, errorDump);
    fclose(snapShot);
    fclose(errorDump);
    return 0;
}
