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
    this->ALUOut = 0u;
    this->stalled = false;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst) {
    this->inst = inst;
    this->ALUOut = 0u;
    this->stalled = false;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst, const unsigned& data) {
    this->inst = inst;
    this->ALUOut = data;
    this->stalled = false;
}

InstPipelineData::~InstPipelineData() {

}

void InstPipelineData::setALUOut(const unsigned& data) {
    this->ALUOut = data;
}

void InstPipelineData::setMDR(const unsigned& mdr) {
    this->MDR = mdr;
}

void InstPipelineData::setStalled(const bool& stalled) {
    this->stalled = stalled;
}

unsigned InstPipelineData::getALUOut() const {
    return ALUOut;
}

unsigned InstPipelineData::getMDR() const {
    return MDR;
}

bool InstPipelineData::isStalled() const {
    return stalled;
}

InstDataBin InstPipelineData::getInst() const {
    return inst;
}

} /* namespace lb */
