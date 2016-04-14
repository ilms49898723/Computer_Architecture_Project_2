/*
 * InstSimulator.h
 *
 *  Created on: 2016/03/11
 *      Author: LittleBird
 */

#ifndef INSTSIMULATOR_H_
#define INSTSIMULATOR_H_

#include <cstdio>
#include <cstdlib>
#include "InstDecoder.h"
#include "InstMemory.h"
#include "InstDataStr.h"
#include "InstErrorDetector.h"
#include "InstEnum.h"

namespace lb {

class InstSimulator {
private:
    constexpr static int maxn = 2048;

public:
    InstSimulator();
    virtual ~InstSimulator();
    void init();
    void loadImageI(const unsigned* src, const unsigned& len, const unsigned& pc);
    void loadImageD(const unsigned* src, const unsigned& len, const unsigned& sp);
    void simulate(FILE* snapshot, FILE* errorDump);

private:
    bool isAlive;
    int cycle;
    FILE* snapshot;
    FILE* errorDump;
    InstMemory memory;
    InstDataBin instSet[maxn];

private:
    void dumpMemoryInfo(const int& cycle);
    void simulateTypeR(const InstDataBin& inst);
    void simulateTypeI(const InstDataBin& inst);
    void simulateTypeJ(const InstDataBin& inst);
    bool checkInst(const InstDataBin& inst);
    bool isNOP(const InstDataBin& inst);
    InstAction detectWriteRegZero(const unsigned& addr);
    InstAction detectNumberOverflow(const int& a, const int& b, const InstOpType& op);
    InstAction detectMemAddrOverflow(const unsigned& addr, const InstMemLen& type);
    InstAction detectDataMisaligned(const unsigned& addr, const InstMemLen& type);
};

} /* namespace lb */

#endif /* INSTSIMULATOR_H_ */
