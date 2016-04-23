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
    this->data = 0u;
    this->stalled = false;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst) {
    this->inst = inst;
    this->data = 0u;
    this->stalled = false;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst, const unsigned& data) {
    this->inst = inst;
    this->data = data;
    this->stalled = false;
}

InstPipelineData::~InstPipelineData() {

}

void InstPipelineData::setData(const unsigned& data) {
    this->data = data;
}

void InstPipelineData::setStalled(const bool& stalled) {
    this->stalled = stalled;
}

unsigned InstPipelineData::getData() const {
    return data;
}

bool InstPipelineData::isStalled() const {
    return stalled;
}

InstDataBin InstPipelineData::getInst() const {
    return inst;
}

} /* namespace lb */
