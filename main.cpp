/*
 * main.cpp
 *
 *  Created on: 2016/03/08
 *      Author: LittleBird
 */

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "InstSimulator.h"
#include "InstImageReader.h"

int main() {
    // constant string filenames
    const std::string iimageFilename = "iimage.bin";
    const std::string dimageFilename = "dimage.bin";
    const std::string snapshotFilename = "snapshot.rpt";
    const std::string errorDumpFilename = "error_dump.rpt";
    // load iimage, dimage
    unsigned iLen, dLen;
    unsigned pc, sp;
    unsigned inst[2048], memory[2048];
    iLen = lb::InstImageReader::readImageI(iimageFilename.c_str(), inst, &pc);
    dLen = lb::InstImageReader::readImageD(dimageFilename.c_str(), memory, &sp);
    // open output file
    FILE* snapShot;
    FILE* errorDump;
    snapShot = fopen(snapshotFilename.c_str(), "w");
    if (!snapShot) {
        fprintf(stderr, "%s: %s\n", snapshotFilename.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }
    errorDump = fopen(errorDumpFilename.c_str(), "w");
    if (!errorDump) {
        fprintf(stderr, "%s: %s\n", errorDumpFilename.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }
    // set simulator, start simulate
    lb::InstSimulator simulator;
    simulator.loadImageI(inst, iLen, pc);
    simulator.loadImageD(memory, dLen, sp);
    simulator.setLogFile(snapShot, errorDump);
    simulator.simulate();
    fclose(snapShot);
    fclose(errorDump);
    return 0;
}
