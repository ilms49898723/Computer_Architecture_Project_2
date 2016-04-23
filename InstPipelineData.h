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

    InstDataBin getInst() const;

    void setData(const unsigned& data);

    unsigned getData() const;

private:
    InstDataBin inst;
    unsigned data;
};

} /* namespace lb */

#endif /* INSTPIPELINEDATA_H_ */
