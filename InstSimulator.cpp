/*
 * InstSimulator.cpp
 *
 *  Created on: 2016/03/11
 *      Author: LittleBird
 */

#include "InstSimulator.h"

namespace lb {

InstSimulator::InstSimulator() {
    init();
    pipeline.clear();
}

InstSimulator::~InstSimulator() {

}

void InstSimulator::init() {
    memory.init();
    cycle = 0;
    pc = 0u;
    snapshot = nullptr;
    errorDump = nullptr;
    isAlive = true;
    for (int i = 0; i < InstSimulator::maxn; ++i) {
        instSet[i] = InstDecoder::decodeInstBin(0u);
    }
}

void InstSimulator::loadImageI(const unsigned* src, const unsigned& len, const unsigned& pc) {
    this->pc = pc;
    unsigned instSetIdx = pc >> 2;
    for (unsigned i = 0; i < len; ++i) {
        instSet[instSetIdx] = InstDecoder::decodeInstBin(src[i]);
        ++instSetIdx;
    }
}

void InstSimulator::loadImageD(const unsigned* src, const unsigned& len, const unsigned& sp) {
    // $sp -> $29
    memory.setRegValue(29, sp, InstMemLen::WORD);
    for (unsigned i = 0; i < len; ++i) {
        memory.setMemValue(i * 4, src[i], InstMemLen::WORD);
    }
}

void InstSimulator::simulate(FILE* snapshot, FILE* errorDump) {
    this->snapshot = snapshot;
    this->errorDump = errorDump;
    // insert nop to pipeline
    // from 0 to 4: IF, ID, EX, MEM, WB
    for (int i = 0; i < 5; ++i) {
        pipeline.push_back(InstPipelineData::nop);
    }
    // initial state dump
    dumpSnapshot(cycle);
    ++cycle;
    while (!isFinished()) {
        // TODO: pipeline, dump info
    }
}

void InstSimulator::dumpSnapshot(const int& cycle) {
    fprintf(snapshot, "cycle %d\n", cycle);
    for (unsigned i = 0; i < 32; ++i) {
        fprintf(snapshot, "$%02d: 0x%08X\n", i, memory.getRegValue(i, InstMemLen::WORD));
    }
    fprintf(snapshot, "PC: 0x%08X\n", pc);
    fprintf(snapshot, "IF: 0x%08X\n", pipeline.at(0).getInst().getInst());
    fprintf(snapshot, "ID: %s\n", pipeline.at(1).getInst().getInstName().c_str());
    fprintf(snapshot, "EX: %s\n", pipeline.at(2).getInst().getInstName().c_str());
    fprintf(snapshot, "DM: %s\n", pipeline.at(3).getInst().getInstName().c_str());
    fprintf(snapshot, "WB: %s\n", pipeline.at(4).getInst().getInstName().c_str());
    fprintf(snapshot, "\n\n");
}

void InstSimulator::instIF() {
    pipeline.push_front(instSet[pc >> 2]);
}

void InstSimulator::instID() {

}

void InstSimulator::instEX() {

}

void InstSimulator::instDM() {

}

void InstSimulator::instWB() {

}

void InstSimulator::pop() {
    pipeline.pop_back();
}

bool InstSimulator::checkInst(const InstDataBin& inst) {
    if (inst.getInstType() == InstType::R) {
        // write $0 error
        if (inst.getFunct() != 0x08u && !isNOP(inst)) { // jr
            detectWriteRegZero(inst.getRd());
        }
        // number overflow
        if (inst.getFunct() == 0x20u) { // add
            int rs, rt;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegValue(inst.getRt(), InstMemLen::WORD));
            detectNumberOverflow(rs, rt, InstOpType::ADD);
        }
        if (inst.getFunct() == 0x22u) { // sub
            int rs, rt;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegValue(inst.getRt(), InstMemLen::WORD));
            detectNumberOverflow(rs, rt, InstOpType::SUB);
        }
    }
    else if (inst.getInstType() == InstType::I) {
        // write $0 error
        if (inst.getOpCode() != 0x2Bu && // sw
            inst.getOpCode() != 0x29u && // sh
            inst.getOpCode() != 0x28u && // sb
            inst.getOpCode() != 0x04u && // beq
            inst.getOpCode() != 0x05u && // bne
            inst.getOpCode() != 0x07u) { // bgtz
            detectWriteRegZero(inst.getRt());
        }
        // number overflow
        if (inst.getOpCode() == 0x08u || // addi
            inst.getOpCode() == 0x23u || // lw
            inst.getOpCode() == 0x21u || // lh
            inst.getOpCode() == 0x25u || // lhu
            inst.getOpCode() == 0x20u || // lb
            inst.getOpCode() == 0x24u || // lbu
            inst.getOpCode() == 0x2Bu || // sw
            inst.getOpCode() == 0x29u || // sh
            inst.getOpCode() == 0x28u) { // sb
            int rs, c;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectNumberOverflow(rs, c, InstOpType::ADD);
        }
        // Address Memory overflow
        if (inst.getOpCode() == 0x23u || // lw
            inst.getOpCode() == 0x2Bu) { // sw
            int rs, c;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstMemLen::WORD);
        }
        if (inst.getOpCode() == 0x21u || // lh
            inst.getOpCode() == 0x25u || // lhu
            inst.getOpCode() == 0x29u) { // sh
            int rs, c;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstMemLen::HALF);
        }
        if (inst.getOpCode() == 0x20u || // lb
            inst.getOpCode() == 0x24u || // lbu
            inst.getOpCode() == 0x28u) { // sb
            int rs, c;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstMemLen::BYTE);
        }
        // Data misaligned
        switch (inst.getOpCode()) {
            case 0x23u: {
                // lw
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::WORD);
                break;
            }
            case 0x21u: {
                // lh
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::HALF);
                break;
            }
            case 0x25u: {
                // lhu
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::HALF);
                break;
            }
            case 0x20u: {
                // lb
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::BYTE);
                break;
            }
            case 0x24u: {
                // lbu
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::BYTE);
                break;
            }
            case 0x2Bu: {
                // sw
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::WORD);
                break;
            }
            case 0x29u: {
                // sh
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::HALF);
                break;
            }
            case 0x28u: {
                // sb
                int rs, c;
                rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::BYTE);
                break;
            }
            default: {
                break;
            }
        }
    }
    else if (inst.getInstType() == InstType::J) {
        return isAlive;
    }
    else {
        return isAlive;
    }
    return isAlive;
}

bool InstSimulator::isNOP(const InstDataBin& inst) {
    return !inst.getOpCode() &&
           !inst.getRt() &&
           !inst.getRd() &&
           !inst.getC() &&
           !inst.getFunct();
}

bool InstSimulator::isHalt(const InstDataBin& inst) {
    return inst.getOpCode() == 0x3Fu;
}

bool InstSimulator::isFinished() {
    return isHalt(pipeline.at(0).getInst()) &&
           isHalt(pipeline.at(1).getInst()) &&
           isHalt(pipeline.at(2).getInst()) &&
           isHalt(pipeline.at(3).getInst()) &&
           isHalt(pipeline.at(4).getInst());
}

InstAction InstSimulator::detectWriteRegZero(const unsigned& addr) {
    if (!InstErrorDetector::isRegWritable(addr)) {
        fprintf(errorDump, "In cycle %d: Write $0 Error\n", cycle);
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectNumberOverflow(const int& a, const int& b, const InstOpType& op) {
    if (InstErrorDetector::isOverflowed(a, b, op)) {
        fprintf(errorDump, "In cycle %d: Number Overflow\n", cycle);
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectMemAddrOverflow(const unsigned& addr, const InstMemLen& type) {
    if (!InstErrorDetector::isValidMemoryAddr(addr, type)) {
        fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle);
        isAlive = false;
        return InstAction::HALT;
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectDataMisaligned(const unsigned& addr, const InstMemLen& type) {
    if (!InstErrorDetector::isAlignedAddr(addr, type)) {
        fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle);
        isAlive = false;
        return InstAction::HALT;
    }
    return InstAction::CONTINUE;
}

} /* namespace lb */
