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
    this->MDR = 0u;
    this->valRs = 0u;
    this->valRt = 0u;
    this->valC = 0u;
    this->stalled = false;
    this->flushed = false;
    this->valid = true;
}

InstPipelineData::InstPipelineData(const bool& valid) {
    this->inst = InstDataBin();
    this->ALUOut = 0u;
    this->MDR = 0u;
    this->valRs = 0u;
    this->valRt = 0u;
    this->valC = 0u;
    this->stalled = false;
    this->flushed = false;
    this->valid = valid;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst) {
    this->inst = inst;
    this->ALUOut = 0u;
    this->MDR = 0u;
    this->valRs = 0u;
    this->valRt = 0u;
    this->valC = inst.getC();
    this->stalled = false;
    this->flushed = false;
    this->valid = true;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst, const unsigned& data) {
    this->inst = inst;
    this->ALUOut = data;
    this->MDR = 0u;
    this->valRs = 0u;
    this->valRt = 0u;
    this->valC = inst.getC();
    this->stalled = false;
    this->flushed = false;
    this->valid = true;
}

InstPipelineData::~InstPipelineData() {

}

void InstPipelineData::setALUOut(const unsigned& data) {
    this->ALUOut = data;
}

void InstPipelineData::setMDR(const unsigned& mdr) {
    this->MDR = mdr;
}

void InstPipelineData::setVal(const unsigned& src, const InstElementType& type) {
    if (type == InstElementType::RS) {
        setValRs(src);
    }
    else if (type == InstElementType::RT) {
        setValRt(src);
    }
    else if (type == InstElementType::C) {
        setValC(src);
    }
}

void InstPipelineData::setValRs(const unsigned& rs) {
    this->valRs = rs;
}

void InstPipelineData::setValRt(const unsigned& rt) {
    this->valRt = rt;
}

void InstPipelineData::setValC(const unsigned& c) {
    this->valC = c;
}

void InstPipelineData::setStalled(const bool& stalled) {
    this->stalled = stalled;
}

void InstPipelineData::setFlushed(const bool& flushed) {
    this->flushed = flushed;
}

unsigned InstPipelineData::getALUOut() const {
    return ALUOut;
}

unsigned InstPipelineData::getMDR() const {
    return MDR;
}

unsigned InstPipelineData::getVal(const InstElementType& type) const {
    if (type == InstElementType::RS) {
        return getValRs();
    }
    else if (type == InstElementType::RT) {
        return getValRt();
    }
    else if (type == InstElementType::C) {
        return getValC();
    }
}

unsigned InstPipelineData::getValRs() const {
    return valRs;
}

unsigned InstPipelineData::getValRt() const {
    return valRt;
}

unsigned InstPipelineData::getValC() const {
    return valC;
}

bool InstPipelineData::isStalled() const {
    return stalled;
}

bool InstPipelineData::isFlushed() const {
    return flushed;
}

bool InstPipelineData::isValid() const {
    return valid;
}

InstDataBin InstPipelineData::getInst() const {
    return inst;
}

} /* namespace lb */
