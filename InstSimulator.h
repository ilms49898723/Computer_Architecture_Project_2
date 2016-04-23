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
#include <deque>
#include "InstDecoder.h"
#include "InstMemory.h"
#include "InstDataStr.h"
#include "InstErrorDetector.h"
#include "InstEnum.h"
#include "InstPipelineData.h"

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
    unsigned pc;
    FILE* snapshot;
    FILE* errorDump;
    InstMemory memory;
    InstDataBin instSet[maxn];

private:
    std::deque<InstPipelineData> pipeline;

private:
    void dumpMemoryInfo(const int& cycle);

    void instIF();

    void instID();

    void instEX();

    void instDM();

    void instWB();

    void pop();

    bool checkInst(const InstDataBin& inst);

    bool isNOP(const InstDataBin& inst);

    bool isHalt(const InstDataBin& inst);

    bool isFinished();

    InstAction detectWriteRegZero(const unsigned& addr);

    InstAction detectNumberOverflow(const int& a, const int& b, const InstOpType& op);

    InstAction detectMemAddrOverflow(const unsigned& addr, const InstMemLen& type);

    InstAction detectDataMisaligned(const unsigned& addr, const InstMemLen& type);
};

} /* namespace lb */

#endif /* INSTSIMULATOR_H_ */
