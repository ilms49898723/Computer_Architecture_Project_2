/*
 * InstDecoder.cpp
 *
 *  Created on: 2016/03/09
 *      Author: LittleBird
 */

#include "InstDecoder.h"

namespace lb {

InstDataStr InstDecoder::decodeInstStr(const unsigned& src) {
    unsigned opCode;
    unsigned rs, rt, rd;
    unsigned c;
    unsigned funct;
    std::string opCodeStr;
    std::string rsStr, rtStr, rdStr;
    std::string cStr;
    std::string functStr;
    opCode = getBitsInRange(src, 26, 32);
    opCodeStr = InstLookUp::opCodeLookUp(opCode);
    if (opCodeStr == "undef") {
        return InstDataStr();
    }
    else if (opCode == 0x0u) {
        funct = getBitsInRange(src, 0, 6);
        rs = getBitsInRange(src, 21, 26);
        rt = getBitsInRange(src, 16, 21);
        rd = getBitsInRange(src, 11, 16);
        c = getBitsInRange(src, 6, 11);
        rsStr = InstLookUp::registerLookUpNumber(rs);
        rtStr = InstLookUp::registerLookUpNumber(rt);
        rdStr = InstLookUp::registerLookUpNumber(rd);
        cStr = toHexString(c);
        functStr = InstLookUp::functLookUp(funct);
        InstDataStr ret;
        ret.setType(InstType::R);
        ret.setOpCode(opCodeStr);
        ret.setRs(rsStr);
        ret.setRt(rtStr);
        ret.setRd(rdStr);
        ret.setC(cStr);
        ret.setFunct(functStr);
        return ret;
    }
    else if (opCode == 0x02u || opCode == 0x03u) {
        c = getBitsInRange(src, 0, 26);
        cStr = toHexString(c);
        InstDataStr ret;
        ret.setType(InstType::J);
        ret.setOpCode(opCodeStr);
        ret.setC(cStr);
        return ret;
    }
    else if (opCode == 0x3Fu) {
        opCodeStr = InstLookUp::opCodeLookUp(opCode);
        InstDataStr ret;
        ret.setType(InstType::S);
        ret.setOpCode(opCodeStr);
        return ret;
    }
    else {
        rs = getBitsInRange(src, 21, 26);
        rt = getBitsInRange(src, 16, 21);
        c = getBitsInRange(src, 0, 16);
        rsStr = InstLookUp::registerLookUpNumber(rs);
        rtStr = InstLookUp::registerLookUpNumber(rt);
        cStr = toHexString(c);
        InstDataStr ret;
        ret.setType(InstType::I);
        ret.setOpCode(opCodeStr);
        ret.setRs(rsStr);
        ret.setRt(rtStr);
        ret.setC(cStr);
        return ret;
    }
}

InstDataStr InstDecoder::decodeInstStr(const unsigned* src) {
    const unsigned argu = (src[0] << 16) | src[1];
    return InstDecoder::decodeInstStr(argu);
}

InstDataBin InstDecoder::decodeInstBin(const unsigned& src) {
    unsigned opCode;
    unsigned rs, rt, rd;
    unsigned c;
    unsigned funct;
    std::string opCodeStr;
    opCode = getBitsInRange(src, 26, 32);
    opCodeStr = InstLookUp::opCodeLookUp(opCode);
    if (opCodeStr == "undef") {
        return InstDataBin();
    }
    else if (opCode == 0x0u) {
        funct = getBitsInRange(src, 0, 6);
        rs = getBitsInRange(src, 21, 26);
        rt = getBitsInRange(src, 16, 21);
        rd = getBitsInRange(src, 11, 16);
        c = getBitsInRange(src, 6, 11);
        InstDataBin ret;
        ret.setInst(src);
        ret.setType(InstType::R);
        ret.setOpCode(opCode);
        ret.setRs(rs);
        ret.setRt(rt);
        ret.setRd(rd);
        ret.setC(c);
        ret.setFunct(funct);
        return ret;
    }
    else if (opCode == 0x02u || opCode == 0x03u) {
        c = getBitsInRange(src, 0, 26);
        InstDataBin ret;
        ret.setInst(src);
        ret.setType(InstType::J);
        ret.setOpCode(opCode);
        ret.setC(c);
        return ret;
    }
    else if (opCode == 0x3Fu) {
        opCodeStr = InstLookUp::opCodeLookUp(opCode);
        InstDataBin ret;
        ret.setInst(src);
        ret.setType(InstType::S);
        ret.setOpCode(opCode);
        return ret;
    }
    else {
        rs = getBitsInRange(src, 21, 26);
        rt = getBitsInRange(src, 16, 21);
        c = getBitsInRange(src, 0, 16);
        InstDataBin ret;
        ret.setInst(src);
        ret.setType(InstType::I);
        ret.setOpCode(opCode);
        ret.setRs(rs);
        ret.setRt(rt);
        ret.setC(c);
        return ret;
    }
}

InstDataBin InstDecoder::decodeInstBin(const unsigned* src) {
    const unsigned argu = (src[0] << 16) | src[1];
    return InstDecoder::decodeInstBin(argu);
}

} /* namespace lb */
