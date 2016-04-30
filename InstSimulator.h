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
    constexpr static int MAXN = 4096;

private:
    const static unsigned IF;
    const static unsigned ID;
    const static unsigned EX;
    const static unsigned DM;
    const static unsigned WB;

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

    void instSetDependency();

    void instSetDependencyID();

    void instSetDependencyEX();

    bool instPredictBranch();

    unsigned instALUR(const InstDataBin& inst);

    unsigned instALUI(const InstDataBin& inst);

    unsigned instALUJ(const unsigned& instPc);

    unsigned instMemLoad(const unsigned& addr, const InstDataBin& inst);

    void instMemStore(const unsigned& addr, const unsigned& val, const InstDataBin& inst);

    bool isNOP(const InstDataBin& inst);

    bool isHalt(const InstDataBin& inst);

    bool isFinished();

    bool isMemoryLoad(const InstDataBin& inst);

    bool isMemoryStore(const InstDataBin& inst);

    bool isBranch(const InstDataBin& inst);

    bool isBranchR(const InstDataBin& inst);

    bool isBranchI(const InstDataBin& inst);

    bool isBranchJ(const InstDataBin& inst);

    bool hasToStall(const unsigned& dependency, const std::vector<unsigned>& dEX,
                    const std::vector<unsigned>& dDM);

    unsigned getDependency(std::vector<unsigned>& dEX, std::vector<unsigned>& dDM);

    InstState checkIDDependency();

    InstAction detectWriteRegZero(const unsigned& addr);

    InstAction detectNumberOverflow(const int& a, const int& b, const InstDataBin& inst);

    InstAction detectMemAddrOverflow(const unsigned& addr, const InstDataBin& inst);

    InstAction detectDataMisaligned(const unsigned& addr, const InstDataBin& inst);
};

} /* namespace lb */

#endif /* INSTSIMULATOR_H_ */
