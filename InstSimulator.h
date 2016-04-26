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

private:
    constexpr static int sIF = 0;
    constexpr static int sID = 1;
    constexpr static int sEX = 2;
    constexpr static int sDM = 3;
    constexpr static int sWB = 4;
    constexpr static unsigned long long sStages = 5ll;

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
    void dumpSnapshot(const int& cycle);

    void instIF();

    void instID();

    void instEX();

    void instDM();

    void instWB();

    void instPush(const unsigned& pc);

    void instPop();

    unsigned instALUR(const unsigned& funct);

    unsigned instALUI(const unsigned& opCode);

    unsigned instMemLoad(const unsigned& addr, const unsigned& opCode);

    void instMemStore(const unsigned& addr, const unsigned& val, const unsigned& opCode);

    bool checkInst(const InstDataBin& inst);

    bool isNOP(const InstDataBin& inst);

    bool isHalt(const InstDataBin& inst);

    bool isFinished();

    bool isMemoryLoad(const unsigned& opCode);

    bool isMemoryStore(const unsigned& opCode);

    bool isBranchR(const unsigned& funct);

    bool isBranchI(const unsigned& opCode);

    bool hasToStall(const InstDataBin& inst);

    bool hasDependency(const InstDataBin& inst);

    InstStage checkForward(const InstDataBin& inst);

    InstAction detectWriteRegZero(const unsigned& addr);

    InstAction detectNumberOverflow(const int& a, const int& b, const InstOpType& op);

    InstAction detectMemAddrOverflow(const unsigned& addr, const InstMemLen& type);

    InstAction detectDataMisaligned(const unsigned& addr, const InstMemLen& type);
};

} /* namespace lb */

#endif /* INSTSIMULATOR_H_ */
