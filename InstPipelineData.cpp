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
}

InstPipelineData::InstPipelineData(const InstDataBin& inst) {
    this->inst = inst;
    this->ALUOut = 0u;
    this->MDR = 0u;
    this->valRs = 0u;
    this->valRt = 0u;
    this->valC = inst.getC();
    this->stalled = false;
}

InstPipelineData::InstPipelineData(const InstDataBin& inst, const unsigned& data) {
    this->inst = inst;
    this->ALUOut = data;
    this->MDR = 0u;
    this->valRs = 0u;
    this->valRt = 0u;
    this->valC = inst.getC();
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

unsigned InstPipelineData::getALUOut() const {
    return ALUOut;
}

unsigned InstPipelineData::getMDR() const {
    return MDR;
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

InstDataBin InstPipelineData::getInst() const {
    return inst;
}

} /* namespace lb */
