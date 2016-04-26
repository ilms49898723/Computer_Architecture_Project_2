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
    memory.setRegister(29, sp, InstMemLen::WORD);
    for (unsigned i = 0; i < len; ++i) {
        memory.setMemory(i * 4, src[i], InstMemLen::WORD);
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
        // TODO: pipeline
        instPop();
        instWB();
        instDM();
        instEX();
        instID();
        instIF();
        dumpSnapshot(cycle);
        ++cycle;
    }
}

void InstSimulator::dumpSnapshot(const int& cycle) {
    fprintf(snapshot, "cycle %d\n", cycle);
    for (unsigned i = 0; i < 32; ++i) {
        fprintf(snapshot, "$%02d: 0x%08X\n", i, memory.getRegister(i));
    }
    // TODO: HAS NOT FINISHED INFORMATION
    fprintf(snapshot, "PC: 0x%08X\n", pc);
    fprintf(snapshot, "IF: 0x%08X\n", pipeline.at(sIF).getInst().getInst());
    fprintf(snapshot, "ID: %s\n", pipeline.at(sID).getInst().getInstName().c_str());
    fprintf(snapshot, "EX: %s\n", pipeline.at(sEX).getInst().getInstName().c_str());
    fprintf(snapshot, "DM: %s\n", pipeline.at(sDM).getInst().getInstName().c_str());
    fprintf(snapshot, "WB: %s\n", pipeline.at(sWB).getInst().getInstName().c_str());
    fprintf(snapshot, "\n\n");
}

void InstSimulator::instIF() {
    if (pipeline.size() < sStages) {
        instPush(pc);
    }
}

void InstSimulator::instID() {
    const InstDataBin& current = pipeline.at(sID).getInst();
    // TODO: forwording detect
    // TODO: BRANCH
}

void InstSimulator::instEX() {
    const InstDataBin& current = pipeline.at(sEX).getInst();
    if (current.getInst() == 0u) {
        // nop
    }
    else if (current.getInstType() == InstType::R && current.getFunct() != 0x08u) {
        // type-R, not jr
        pipeline.at(sEX).setALUOut(instALUR(current.getFunct()));
    }
    else if (current.getInstType() == InstType::I && current.getOpCode() != 0x04u && current.getOpCode() != 0x05u && current.getOpCode() != 0x07u) {
        // type-I, not beq, bne, bgtz
        pipeline.at(sEX).setALUOut(instALUI(current.getOpCode()));
    }
    else if (current.getInstType() == InstType::J) {
        // jumps
        // TODO: jumps not implemented
        pc = (pc & 0xF0000000u) | current.getC();
    }
    else if (current.getInstType() == InstType::S) {
        // halt
    }
    else {
        // branch(jr, beq, bne, bgtz)
    }
}

void InstSimulator::instDM() {
    const InstDataBin& current = pipeline.at(sDM).getInst();
    if (isMemoryLoad(current.getOpCode())) {
        const unsigned& ALUOut = pipeline.at(sDM).getALUOut();
        const unsigned MDR = instMemLoad(ALUOut, current.getOpCode());
        pipeline.at(sDM).setMDR(MDR);
    }
}

void InstSimulator::instWB() {
    const InstPipelineData& current = pipeline.at(sWB);
    if (isMemoryStore(current.getInst().getOpCode())) {
        unsigned val = memory.getRegister(current.getInst().getRt());
        instMemStore(current.getALUOut(), val, current.getInst().getOpCode());
    }
    else {
        instMemStore(current.getInst().getRegWrite().at(0), current.getALUOut(), current.getInst().getOpCode());
    }
}

void InstSimulator::instPush(const unsigned& pc) {
    if (pipeline.size() < sStages) {
        pipeline.push_front(instSet[pc >> 2]);
    }
}

void InstSimulator::instPop() {
    pipeline.pop_back();
}

unsigned InstSimulator::instALUR(const unsigned& funct) {
    const unsigned& valRs = memory.getRegister(pipeline.at(sEX).getInst().getRs());
    const unsigned& valRt = memory.getRegister(pipeline.at(sEX).getInst().getRt());
    const unsigned& valC = pipeline.at(sEX).getInst().getC();
    switch (funct) {
        case 0x20u: // add
            return valRs + valRt;
        case 0x21u: // addu
            return valRs + valRt;
        case 0x22u: // sub
            return valRs - valRt;
        case 0x24u: // and
            return valRs & valRt;
        case 0x25u: // or
            return valRs | valRt;
        case 0x26u: // xor
            return valRs ^ valRt;
        case 0x27u: // nor
            return ~(valRs | valRt);
        case 0x28u: // nand
            return ~(valRs & valRt);
        case 0x2Au: // slt
            return static_cast<unsigned>(valRs < valRt);
        case 0x00u: // sll
            return valRt << valC;
        case 0x02u: // srl
            return valRt >> valC;
        case 0x03u: // sra
            return static_cast<unsigned>(static_cast<int>(valRt) >> static_cast<int>(valC));
        default:
            return 0u;
    }
}

unsigned InstSimulator::instALUI(const unsigned& opCode) {
    const unsigned& valRs = memory.getRegister(pipeline.at(sEX).getInst().getRs());
    const unsigned& valC = pipeline.at(sEX).getInst().getC();
    switch (opCode) {
        case 0x08u: // addi
            return toUnsigned(toSigned(valRs) + toSigned(valC, 16));
        case 0x09u: // addiu
            return valRs + toUnsigned(toSigned(valC, 16));
        case 0x23u: // lw
        case 0x21u: // lh
        case 0x25u: // lhu
        case 0x20u: // lb
        case 0x24u: // lbu
        case 0x2Bu: // sw
        case 0x29u: // sh
        case 0x28u: // sb
            return toUnsigned(toSigned(valRs) + toSigned(valC, 16));
        case 0x0Fu: // lui // special // maybe incorrect
            return valC << 16;
        case 0x0Cu: // andi
            return valRs & valC;
        case 0x0Du: // ori
            return valRs | valC;
        case 0x0Eu: // nori
            return ~(valRs | valC);
        case 0x0Au: // slti
            return static_cast<unsigned>(toSigned(valRs) < toSigned(valC, 16));
        default:
            return 0u;
    }
}

unsigned InstSimulator::instMemLoad(const unsigned& addr, const unsigned& opCode) {
    switch (opCode) {
        case 0x23u:
            return memory.getMemory(addr, InstMemLen::WORD);
        case 0x21u:
            return toUnsigned(toSigned(memory.getMemory(addr, InstMemLen::HALF), InstMemLen::HALF));
        case 0x25u:
            return memory.getMemory(addr, InstMemLen::HALF);
        case 0x20u:
            return toUnsigned(toSigned(memory.getMemory(addr, InstMemLen::BYTE), InstMemLen::BYTE));
        case 0x24u:
            return memory.getMemory(addr, InstMemLen::BYTE);
        default:
            return 0u;
    }
}

void InstSimulator::instMemStore(const unsigned& addr, const unsigned& val, const unsigned& opCode) {
    switch (opCode) {
        case 0x2Bu:
            memory.setMemory(addr, val, InstMemLen::WORD);
            return;
        case 0x29u:
            memory.setMemory(addr, val, InstMemLen::HALF);
            return;
        case 0x28u:
            memory.setMemory(addr, val, InstMemLen::BYTE);
            return;
        default:
            return;
    }
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
            rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegister(inst.getRt(), InstMemLen::WORD));
            detectNumberOverflow(rs, rt, InstOpType::ADD);
        }
        if (inst.getFunct() == 0x22u) { // sub
            int rs, rt;
            rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegister(inst.getRt(), InstMemLen::WORD));
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
            rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectNumberOverflow(rs, c, InstOpType::ADD);
        }
        // Address Memory overflow
        if (inst.getOpCode() == 0x23u || // lw
            inst.getOpCode() == 0x2Bu) { // sw
            int rs, c;
            rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstMemLen::WORD);
        }
        if (inst.getOpCode() == 0x21u || // lh
            inst.getOpCode() == 0x25u || // lhu
            inst.getOpCode() == 0x29u) { // sh
            int rs, c;
            rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstMemLen::HALF);
        }
        if (inst.getOpCode() == 0x20u || // lb
            inst.getOpCode() == 0x24u || // lbu
            inst.getOpCode() == 0x28u) { // sb
            int rs, c;
            rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstMemLen::BYTE);
        }
        // Data misaligned
        switch (inst.getOpCode()) {
            case 0x23u: {
                // lw
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::WORD);
                break;
            }
            case 0x21u: {
                // lh
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::HALF);
                break;
            }
            case 0x25u: {
                // lhu
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::HALF);
                break;
            }
            case 0x20u: {
                // lb
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::BYTE);
                break;
            }
            case 0x24u: {
                // lbu
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::BYTE);
                break;
            }
            case 0x2Bu: {
                // sw
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::WORD);
                break;
            }
            case 0x29u: {
                // sh
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstMemLen::HALF);
                break;
            }
            case 0x28u: {
                // sb
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstMemLen::WORD));
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

bool InstSimulator::isMemoryLoad(const unsigned& opCode) {
    switch (opCode) {
        case 0x23u:
        case 0x21u:
        case 0x25u:
        case 0x20u:
        case 0x24u:
            return true;
        default:
            return false;
    }
}

bool InstSimulator::isMemoryStore(const unsigned& opCode) {
    switch (opCode) {
        case 0x2Bu:
        case 0x29u:
        case 0x28u:
            return true;
        default:
            return false;
    }
}

bool InstSimulator::isBranchR(const unsigned& funct) {
    return funct == 0x08u;
}

bool InstSimulator::isBranchI(const unsigned& opCode) {
    return opCode == 0x04u || opCode == 0x05u || opCode == 0x07u;
}

bool InstSimulator::hasToStall(const InstDataBin& inst) {
    for (const auto& item : inst.getRegRead()) {
        const std::vector<unsigned>& dmWrite = pipeline.at(sDM).getInst().getRegWrite();
        if (!dmWrite.empty() && !item && item == dmWrite.at(0)) {
            return true;
        }
    }
    return false;
}

bool InstSimulator::hasDependency(const InstDataBin& inst) {
    for (const auto& item : inst.getRegRead()) {
        const std::vector<unsigned>& exWrite = pipeline.at(sEX).getInst().getRegWrite();
        const std::vector<unsigned>& dmWrite = pipeline.at(sDM).getInst().getRegWrite();
        if (!exWrite.empty() && !item && item == exWrite.at(0)) {
            return true;
        }
        if (!dmWrite.empty() && !item && item == dmWrite.at(0)) {
            return true;
        }
    }
    return false;
}

InstStage InstSimulator::checkForward(const InstDataBin& inst) {
    if (hasDependency(inst)) {
        if (hasToStall(inst)) {
            return InstStage::STALL;
        }
        else {
            return InstStage::EX;
        }
    }
    else {
        return InstStage::NONE;
    }
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
