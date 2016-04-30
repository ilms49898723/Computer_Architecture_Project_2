/*
 * InstPipelineData.h
 *
 *  Created on: 2016/04/23
 *      Author: LittleBird
 */

#ifndef INSTPIPELINEDATA_H_
#define INSTPIPELINEDATA_H_

#include "InstDataBin.h"
#include "InstDecoder.h"

namespace lb {

class InstPipelineData {
public:
    const static InstPipelineData nop;

public:
    InstPipelineData();

    InstPipelineData(const InstDataBin& inst);

    InstPipelineData(const InstDataBin& inst, const unsigned& instPc);

    virtual ~InstPipelineData();

    void setInstPc(const unsigned& instPc);

    void setALUOut(const unsigned& ALUOut);

    void setMDR(const unsigned& MDR);

    void setVal(const unsigned& src, const InstElementType& type);

    void setValRs(const unsigned& rs);

    void setValRt(const unsigned& rt);

    void setBranchResult(const bool& branchResult);

    void setStalled(const bool& stalled);

    void setFlushed(const bool& flushed);

    unsigned getInstPc() const;

    unsigned getALUOut() const;

    unsigned getMDR() const;

    unsigned getVal(const InstElementType& type) const;

    unsigned getValRs() const;

    unsigned getValRt() const;

    unsigned getValC() const;

    bool getBranchResult() const;

    bool isStalled() const;

    bool isFlushed() const;

    const InstDataBin& getInst() const;

private:
    InstDataBin inst;
    unsigned instPc;
    unsigned ALUOut;
    unsigned MDR;
    unsigned valRs;
    unsigned valRt;
    unsigned valC;
    bool branchResult;
    bool stalled;
    bool flushed;
};

} /* namespace lb */

#endif /* INSTPIPELINEDATA_H_ */
