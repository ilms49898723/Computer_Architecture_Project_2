/*
 * InstData.cpp
 *
 *  Created on: 2016/03/09
 *      Author: LittleBird
 */

#include "InstDataStr.h"

namespace lb {

InstDataStr::InstDataStr() {
    this->instType = InstType::UNDEF;
    this->opCode = "";
    this->rs = "";
    this->rt = "";
    this->rd = "";
    this->c = "";
    this->funct = "";
}

InstDataStr::~InstDataStr() {

}

InstType InstDataStr::getInstType() const {
    return instType;
}

std::string InstDataStr::getOpCode() const {
    return opCode;
}

std::string InstDataStr::getRs() const {
    if (instType == InstType::J || instType == InstType::S) {
        return "";
    }
    return rs;
}

std::string InstDataStr::getRt() const {
    if (instType == InstType::J || instType == InstType::S) {
        return "";
    }
    return rt;
}

std::string InstDataStr::getRd() const {
    if (instType != InstType::R) {
        return "";
    }
    return rd;
}

std::string InstDataStr::getC() const {
    return c;
}

std::string InstDataStr::getFunct() const {
    if (instType != InstType::R) {
        return "";
    }
    return funct;
}

void InstDataStr::setInstType(const InstType& val) {
    instType = val;
}

void InstDataStr::setOpCode(const std::string& val) {
    opCode = val;
}

void InstDataStr::setRs(const std::string& val) {
    rs = val;
}

void InstDataStr::setRt(const std::string& val) {
    rt = val;
}

void InstDataStr::setRd(const std::string& val) {
    rd = val;
}

void InstDataStr::setC(const std::string& val) {
    c = val;
}

void InstDataStr::setFunct(const std::string& val) {
    funct = val;
}

std::string InstDataStr::toString() const {
    if (instType == InstType::R) {
        if (funct == "jr") {
            return funct + " $" + rs;
        }
        else if (funct == "sll" || funct == "srl" || funct == "sra") {
            return funct + " $" + rd + ", $" + rt + ", " + c;
        }
        else {
            return funct + " $" + rd + ", $" + rs + ", $" + rt;
        }
    }
    else if (instType == InstType::I) {
        if (opCode == "lui") {
            return opCode + " $" + rt + ", " + c;
        }
        else if (opCode == "bgtz") {
            return opCode + " $" + rs + ", " + c;
        }
        else if (opCode == "addi" || opCode == "addiu" || opCode == "lui" ||
                 opCode == "andi" || opCode == "ori" || opCode == "nori" ||
                 opCode == "slti") {
            return opCode + " $" + rt + ", $" + rs + ", " + c;
        }
        else if (opCode == "beq" || opCode == "bne") {
            return opCode + " $" + rs + ", $" + rt + ", " + c;
        }
        else {
            return opCode + " $" + rt + ", " + c + "($" + rs + ")";
        }
    }
    else if (instType == InstType::J) {
        return opCode + " " + c;
    }
    else if (instType == InstType::S) {
        return opCode;
    }
    else {
        return "undef";
    }
}

} /* namespace lb */
