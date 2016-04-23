/*
 * InstPipelineData.cpp
 *
 *  Created on: 2016/04/23
 *      Author: LittleBird
 */

#include "InstPipelineData.h"

namespace lb {

const InstPipelineData InstPipelineData::nop = InstPipelineData(InstDecoder::decodeInstBin(0u), 0u);

InstPipelineData::InstPipelineData() {
    this->inst = InstDataBin();
}

InstPipelineData::InstPipelineData(const InstDataBin& inst) {
    this->inst = inst;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst, const unsigned& data) {
    this->inst = inst;
    this->data = data;
}

InstPipelineData::~InstPipelineData() {

}

InstDataBin InstPipelineData::getInst() const {
    return inst;
}

} /* namespace lb */
