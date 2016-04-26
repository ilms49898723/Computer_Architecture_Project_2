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

    InstPipelineData(const InstDataBin& inst, const unsigned& data);

    virtual ~InstPipelineData();

    void setALUOut(const unsigned& ALUOut);

    void setMDR(const unsigned& MDR);

    void setValRs(const unsigned& rs);

    void setValRt(const unsigned& rt);

    void setValC(const unsigned& c);

    void setStalled(const bool& stalled);

    unsigned getALUOut() const;

    unsigned getMDR() const;

    unsigned getValRs() const;

    unsigned getValRt() const;

    unsigned getValC() const;

    bool isStalled() const;

    InstDataBin getInst() const;

private:
    InstDataBin inst;
    unsigned ALUOut;
    unsigned MDR;
    unsigned valRs;
    unsigned valRt;
    unsigned valC;
    bool stalled;
};

} /* namespace lb */

#endif /* INSTPIPELINEDATA_H_ */
