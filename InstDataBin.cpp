/*
 * InstDataBin.cpp
 *
 *  Created on: 2016/03/13
 *      Author: LittleBird
 */

#include "InstDataBin.h"

namespace lb {

InstDataBin::InstDataBin() {
    this->instType = InstType::UNDEF;
    this->opCode = 0u;
    this->rs = 0u;
    this->rt = 0u;
    this->rd = 0u;
    this->c = 0u;
    this->funct = 0u;
    this->inst = 0u;
    this->regRead.clear();
    this->regWrite.clear();
}

InstDataBin::~InstDataBin() {

}

InstType InstDataBin::getInstType() const {
    return instType;
}

unsigned InstDataBin::getOpCode() const {
    return opCode;
}

unsigned InstDataBin::getRs() const {
    return rs;
}

unsigned InstDataBin::getRt() const {
    return rt;
}

unsigned InstDataBin::getRd() const {
    return rd;
}

unsigned InstDataBin::getC() const {
    return c;
}

unsigned InstDataBin::getFunct() const {
    return funct;
}

unsigned InstDataBin::getInst() const {
    return inst;
}

std::string InstDataBin::getInstName() const {
    return instName;
}

const std::vector<InstElement>& InstDataBin::getRegRead() const {
    return regRead;
}

const std::vector<InstElement>& InstDataBin::getRegWrite() const {
    return regWrite;
}

void InstDataBin::setInstType(const InstType& val) {
    instType = val;
}

void InstDataBin::setOpCode(const unsigned& val) {
    opCode = val;
}

void InstDataBin::setRs(const unsigned& val) {
    rs = val;
}

void InstDataBin::setRt(const unsigned& val) {
    rt = val;
}

void InstDataBin::setRd(const unsigned& val) {
    rd = val;
}

void InstDataBin::setC(const unsigned& val) {
    c = val;
}

void InstDataBin::setFunct(const unsigned& val) {
    funct = val;
}

void InstDataBin::setInst(const unsigned& val) {
    inst = val;
}

void InstDataBin::setInstName(const unsigned& val) {
    if (instType == InstType::UNDEF) {
        instName = "";
    }
    else if (instType == InstType::R) {
        if (inst == 0u) {
            instName = "NOP";
        }
        else if (rt == 0u && rd == 0u && c == 0u && funct == 0u) {
            instName = "NOP";
        }
        else {
            instName = toUpperString(InstLookUp::functLookUp(val));
        }
    }
    else {
        instName = toUpperString(InstLookUp::opCodeLookUp(val));
    }
}

void InstDataBin::setRegRead(const InstElement& reg) {
    regRead.push_back(reg);
}

void InstDataBin::setRegWrite(const InstElement& reg) {
    regWrite.push_back(reg);
}

} /* namespace lb */
