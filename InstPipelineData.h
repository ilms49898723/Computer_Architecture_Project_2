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

    void setData(const unsigned& data);

    void setStalled(const bool& stalled);

    unsigned getData() const;

    bool isStalled() const;

    InstDataBin getInst() const;

private:
    InstDataBin inst;
    unsigned data;
    bool stalled;
};

} /* namespace lb */

#endif /* INSTPIPELINEDATA_H_ */
