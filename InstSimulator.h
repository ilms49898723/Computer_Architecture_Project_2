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
#include "InstDataBin.h"
#include "InstErrorDetector.h"
#include "InstType.h"
#include "InstPipelineData.h"

namespace lb {

class InstSimulator {
private:
    constexpr static int MAXN = 2048;

private:
    const static unsigned IF;
    const static unsigned ID;
    const static unsigned EX;
    const static unsigned DM;
    const static unsigned WB;
    const static unsigned STAGES;

public:
    InstSimulator();

    virtual ~InstSimulator();

    void init();

    void loadImageI(const unsigned* src, const unsigned& len, const unsigned& pc);

    void loadImageD(const unsigned* src, const unsigned& len, const unsigned& sp);

    void setLogFile(FILE* snapshot, FILE* errorDump);

    void simulate();

private:
    bool alive;
    bool pcUpdated;
    unsigned pc;
    unsigned pcOriginal;
    unsigned cycle;
    FILE* snapshot;
    FILE* errorDump;
    InstMemory memory;
    InstDataBin instList[MAXN];

private:
    std::deque<InstPipelineData> pipeline;
    std::deque<InstElement> idForward;
    std::deque<InstElement> exForward;

private:
    void dumpSnapshot(FILE* fp);

    void dumpPipelineInfo(FILE* fp, const int stage);

    void instIF();

    void instID();

    void instEX();

    void instDM();

    void instWB();

    void instPop();

    void instStall();

    void instUnstall();

    void instFlush();

    void instCleanUp();

    void instSetDependency();

    void instSetDependencyID();

    void instSetDependencyEX();

    bool instPredictBranch();

    unsigned instALUR(const unsigned& funct);

    unsigned instALUI(const unsigned& opCode);

    unsigned instALUJ();

    unsigned instMemLoad(const unsigned& addr, const unsigned& opCode);

    void instMemStore(const unsigned& addr, const unsigned& val, const unsigned& opCode);

    bool isNOP(const InstDataBin& inst);

    bool isHalt(const InstDataBin& inst);

    bool isFinished();

    bool isMemoryLoad(const unsigned& opCode);

    bool isMemoryStore(const unsigned& opCode);

    bool isBranch(const InstDataBin& inst);

    bool isBranchR(const InstDataBin& inst);

    bool isBranchI(const InstDataBin& inst);

    bool isBranchJ(const InstDataBin& inst);

    bool hasToStall(const unsigned long long& dependency);

    unsigned long long getDependency();

    InstState checkIDDependency();

    InstAction detectWriteRegZero(const unsigned& addr);

    InstAction detectNumberOverflow(const int& a, const int& b, const InstOpType& op);

    InstAction detectMemAddrOverflow(const unsigned& addr, const InstSize& type);

    InstAction detectDataMisaligned(const unsigned& addr, const InstSize& type);
};

} /* namespace lb */

#endif /* INSTSIMULATOR_H_ */
