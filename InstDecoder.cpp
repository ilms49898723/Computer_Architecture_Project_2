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
        ret.setInstType(InstType::R);
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
        ret.setInstType(InstType::J);
        ret.setOpCode(opCodeStr);
        ret.setC(cStr);
        return ret;
    }
    else if (opCode == 0x3Fu) {
        opCodeStr = InstLookUp::opCodeLookUp(opCode);
        InstDataStr ret;
        ret.setInstType(InstType::S);
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
        ret.setInstType(InstType::I);
        ret.setOpCode(opCodeStr);
        ret.setRs(rsStr);
        ret.setRt(rtStr);
        ret.setC(cStr);
        return ret;
    }
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
        ret.setInstType(InstType::R);
        ret.setOpCode(opCode);
        ret.setRs(rs);
        ret.setRt(rt);
        ret.setRd(rd);
        ret.setC(c);
        ret.setFunct(funct);
        ret.setInstName(funct);
        switch (funct) {
            case 0x08u: // jr
                ret.setRegRead(InstElement(rs, InstElementType::RS));
                break;
            case 0x00u: // sll
            case 0x02u: // srl
            case 0x03u: // sra
                ret.setRegRead(InstElement(rt, InstElementType::RT));
                ret.setRegWrite(InstElement(rd, InstElementType::RD));
                break;
            default:
                ret.setRegRead(InstElement(rs, InstElementType::RS));
                ret.setRegRead(InstElement(rt, InstElementType::RT));
                ret.setRegWrite(InstElement(rd, InstElementType::RD));
                break;
        }
        return ret;
    }
    else if (opCode == 0x02u || opCode == 0x03u) {
        c = getBitsInRange(src, 0, 26);
        InstDataBin ret;
        ret.setInst(src);
        ret.setInstType(InstType::J);
        ret.setOpCode(opCode);
        ret.setC(c);
        ret.setInstName(opCode);
        if (opCode == 0x03u) {
            ret.setRegWrite(InstElement(31));
        }
        return ret;
    }
    else if (opCode == 0x3Fu) {
        InstDataBin ret;
        ret.setInst(src);
        ret.setInstType(InstType::S);
        ret.setOpCode(opCode);
        ret.setInstName(opCode);
        return ret;
    }
    else {
        rs = getBitsInRange(src, 21, 26);
        rt = getBitsInRange(src, 16, 21);
        c = getBitsInRange(src, 0, 16);
        InstDataBin ret;
        ret.setInst(src);
        ret.setInstType(InstType::I);
        ret.setOpCode(opCode);
        ret.setRs(rs);
        ret.setRt(rt);
        ret.setC(c);
        ret.setInstName(opCode);
        switch (opCode) {
            case 0x07u: // bgtz
                ret.setRegRead(InstElement(rs, InstElementType::RS));
                break;
            case 0x0Fu: // lui
                ret.setRegWrite(InstElement(rt, InstElementType::RT));
                break;
            case 0x04u: // beq
            case 0x05u: // bne
            case 0x2Bu: // sw
            case 0x29u: // sh
            case 0x28u: // sb
                ret.setRegRead(InstElement(rs, InstElementType::RS));
                ret.setRegRead(InstElement(rt, InstElementType::RT));
                break;
            default:
                ret.setRegRead(InstElement(rs, InstElementType::RS));
                ret.setRegWrite(InstElement(rt, InstElementType::RT));
                break;
        }
        return ret;
    }
}

} /* namespace lb */
