/*
 * InstDataBin.cpp
 *
 *  Created on: 2016/03/13
 *      Author: LittleBird
 */

#include "InstDataBin.h"

namespace lb {

InstDataBin::InstDataBin() {
    type = InstType::Undef;
    opCode = rs = rt = rd = c = funct = inst = 0u;
}

InstDataBin::~InstDataBin() {
}

InstType InstDataBin::getType() const {
    return type;
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

void InstDataBin::setType(const InstType& val) {
    type = val;
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

} /* namespace lb */
