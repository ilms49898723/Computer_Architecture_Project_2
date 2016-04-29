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
}

InstSimulator::~InstSimulator() {

}

void InstSimulator::init() {
    pipeline.clear();
    idForward.clear();
    exForward.clear();
    memory.init();
    pcOriginal = 0u;
    snapshot = nullptr;
    errorDump = nullptr;
    for (int i = 0; i < InstSimulator::maxn; ++i) {
        instSet[i] = InstDecoder::decodeInstBin(0u);
    }
}

void InstSimulator::loadImageI(const unsigned* src, const unsigned& len, const unsigned& pc) {
    this->pcOriginal = pc;
    unsigned instSetIdx = pc >> 2;
    for (unsigned i = 0; i < len; ++i) {
        instSet[instSetIdx] = InstDecoder::decodeInstBin(src[i]);
        ++instSetIdx;
    }
}

void InstSimulator::loadImageD(const unsigned* src, const unsigned& len, const unsigned& sp) {
    // $sp -> $29
    memory.setRegister(29, sp, InstSize::WORD);
    for (unsigned i = 0; i < len; ++i) {
        memory.setMemory(i * 4, src[i], InstSize::WORD);
    }
}

void InstSimulator::simulate(FILE* snapshot, FILE* errorDump) {
    this->snapshot = snapshot;
    this->errorDump = errorDump;
    cycle = 0u;
    pc = pcOriginal;
    alive = true;
    for (int i = 0; i < 5; ++i) {
        pipeline.push_back(InstPipelineData::nop);
    }
    while (!isFinished()) {
        // TODO: REWRITE
        pcUpdated = false;
        instWB();
        instDM();
        instEX();
        instID();
        instIF();
        instPop();
        dumpSnapshot(snapshot);
        ++cycle;
        if (!pcUpdated) {
            pc += 4;
        }
    }
}

void InstSimulator::dumpSnapshot(FILE* fp) {
    fprintf(fp, "cycle %u\n", cycle);
    for (unsigned i = 0; i < 32; ++i) {
        fprintf(fp, "$%02d: 0x%08X\n", i, memory.getRegister(i));
    }
    fprintf(fp, "PC: 0x%08X\n", pc);
    fprintf(fp, "IF: 0x%08X", pipeline.at(IF).getInst().getInst());
    dumpPipelineInfo(fp, IF);
    fprintf(fp, "\n");
    fprintf(fp, "ID: %s", pipeline.at(ID).getInst().getInstName().c_str());
    dumpPipelineInfo(fp, ID);
    fprintf(fp, "\n");
    fprintf(fp, "EX: %s", pipeline.at(EX).getInst().getInstName().c_str());
    dumpPipelineInfo(fp, EX);
    fprintf(fp, "\n");
    fprintf(fp, "DM: %s\n", pipeline.at(DM).getInst().getInstName().c_str());
    fprintf(fp, "WB: %s\n", pipeline.at(WB).getInst().getInstName().c_str());
    fprintf(fp, "\n\n");
}

void InstSimulator::dumpPipelineInfo(FILE* fp, const int stage) {
    switch (stage) {
        case IF:
            if (pipeline.at(IF).isFlushed()) {
                fprintf(fp, " to_be_flushed");
            }
            else if (pipeline.at(IF).isStalled()) {
                fprintf(fp, " to_be_stalled");
            }
            break;
        case ID:
            if (pipeline.at(ID).isStalled()) {
                fprintf(fp, " to_be_stalled");
            }
            else {
                for (const auto& item : idForward) {
                    fprintf(fp, " fwd_EX-DM_%s_$%d", item.type == InstElementType::RS ? "rs" : "rt", item.val);
                }
            }
            break;
        case EX:
            for (const auto& item : exForward) {
                fprintf(fp, " fwd_EX-DM_%s_$%d", item.type == InstElementType::RS ? "rs" : "rt", item.val);
            }
        default:
            break;
    }
}

void InstSimulator::instIF() {
    instPush();
    if (pipeline.at(IF).isStalled()) {
        pcUpdated = true;
    }
}

void InstSimulator::instID() {
    InstPipelineData& current = pipeline.at(ID);
    const InstDataBin& currentInst = pipeline.at(ID).getInst();
    if (isNOP(currentInst) || isHalt(currentInst)) {
        return;
    }
    // TODO: REWRITE FORWARDING

    // for branch
    if (isBranchR(currentInst.getFunct())) {
        pc = current.getValRs();
        pcUpdated = true;
        instFlush();
    }
    else if (isBranchI(currentInst.getOpCode())) {
        bool result;
        switch (current.getInst().getOpCode()) {
            case 0x04u:
                result = current.getValRs() == current.getValRt();
                break;
            case 0x05u:
                result = current.getValRs() != current.getValRt();
                break;
            case 0x07u:
                result = current.getValRs() > 0u;
                break;
            default:
                result = false;
                break;
        }
        if (result) {
            pc += 4 + 4 * toSigned(current.getValC(), 16);
            pcUpdated = true;
            instFlush();
        }
    }
    else if (isBranchJ(currentInst.getOpCode())) {
        if (currentInst.getOpCode() == 0x03u) {
            current.setALUOut(instALUJ());
        }
        pc = (pc & 0xF0000000u) | (current.getValC() * 4);
        pcUpdated = true;
        instFlush();
    }
}

void InstSimulator::instEX() {
    const InstDataBin& current = pipeline.at(EX).getInst();
    if (isNOP(current) || isHalt(current) || isBranch(current)) {
        return;
    }
    else if (current.getInstType() == InstType::R && current.getFunct() != 0x08u) {
        // type-R, not jr
        pipeline.at(EX).setALUOut(instALUR(current.getFunct()));
    }
    else if (current.getInstType() == InstType::I && current.getOpCode() != 0x04u && current.getOpCode() != 0x05u && current.getOpCode() != 0x07u) {
        // type-I, not beq, bne, bgtz
        pipeline.at(EX).setALUOut(instALUI(current.getOpCode()));
    }
}

void InstSimulator::instDM() {
    const InstDataBin& current = pipeline.at(DM).getInst();
    if (isMemoryLoad(current.getOpCode())) {
        const unsigned& ALUOut = pipeline.at(DM).getALUOut();
        const unsigned MDR = instMemLoad(ALUOut, current.getOpCode());
        pipeline.at(DM).setMDR(MDR);
    }
}

void InstSimulator::instWB() {
    const InstPipelineData& current = pipeline.at(WB);
    if (isNOP(current.getInst()) || isHalt(current.getInst())) {
        return;
    }
    if (current.getInst().getRegWrite().empty()) {
        return;
    }
    if (isMemoryStore(current.getInst().getOpCode())) {
        unsigned val = memory.getRegister(current.getInst().getRt());
        instMemStore(current.getALUOut(), val, current.getInst().getOpCode());
    }
    else {
        const unsigned& targetAddress = current.getInst().getRegWrite().at(0).val;
        if (isMemoryLoad(current.getInst().getOpCode())) {
            memory.setRegister(targetAddress, current.getMDR());
        }
        else {
            memory.setRegister(targetAddress, current.getALUOut());
        }
    }
}

void InstSimulator::instPush() {
    if (pipeline.at(IF).isStalled()) {
        pipeline.insert(pipeline.begin() + 2, InstPipelineData::nop);
    }
    else if (pipeline.at(IF).isFlushed()) {
        pipeline.at(IF) = InstPipelineData::nop;
        pipeline.push_front(instSet[pc >> 2]);
    }
    else {
        pipeline.push_front(instSet[pc >> 2]);
    }
}

void InstSimulator::instPop() {
    // if size of pipeline > 5, pop the last one
    if (pipeline.size() > 5) {
        pipeline.pop_back();
    }
}

void InstSimulator::instStall() {
    pipeline.at(IF).setStalled(true);
    pipeline.at(ID).setStalled(true);
}

void InstSimulator::instUnstall() {
    pipeline.at(IF).setStalled(false);
    pipeline.at(ID).setStalled(false);
}

void InstSimulator::instFlush() {
    pipeline.at(IF).setFlushed(true);
}

void InstSimulator::instForward(const InstDataBin& inst) {
    // TODO: REWRITE
}

unsigned InstSimulator::instALUR(const unsigned& funct) {
    const unsigned& valRs = memory.getRegister(pipeline.at(EX).getInst().getRs());
    const unsigned& valRt = memory.getRegister(pipeline.at(EX).getInst().getRt());
    const unsigned& valC = pipeline.at(EX).getInst().getC();
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
    const unsigned& valRs = memory.getRegister(pipeline.at(EX).getInst().getRs());
    const unsigned& valC = pipeline.at(EX).getInst().getC();
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

unsigned InstSimulator::instALUJ() {
    return pc + 4;
}

unsigned InstSimulator::instMemLoad(const unsigned& addr, const unsigned& opCode) {
    switch (opCode) {
        case 0x23u:
            return memory.getMemory(addr, InstSize::WORD);
        case 0x21u:
            return toUnsigned(toSigned(memory.getMemory(addr, InstSize::HALF), InstSize::HALF));
        case 0x25u:
            return memory.getMemory(addr, InstSize::HALF);
        case 0x20u:
            return toUnsigned(toSigned(memory.getMemory(addr, InstSize::BYTE), InstSize::BYTE));
        case 0x24u:
            return memory.getMemory(addr, InstSize::BYTE);
        default:
            return 0u;
    }
}

void InstSimulator::instMemStore(const unsigned& addr, const unsigned& val, const unsigned& opCode) {
    switch (opCode) {
        case 0x2Bu:
            memory.setMemory(addr, val, InstSize::WORD);
            return;
        case 0x29u:
            memory.setMemory(addr, val, InstSize::HALF);
            return;
        case 0x28u:
            memory.setMemory(addr, val, InstSize::BYTE);
            return;
        default:
            return;
    }
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

bool InstSimulator::isBranch(const InstDataBin& inst) {
    return isBranchR(inst.getFunct()) ||
           isBranchI(inst.getOpCode()) ||
           isBranchJ(inst.getOpCode());
}

bool InstSimulator::isBranchR(const unsigned& funct) {
    return funct == 0x08u;
}

bool InstSimulator::isBranchI(const unsigned& opCode) {
    return opCode == 0x04u || opCode == 0x05u || opCode == 0x07u;
}

bool InstSimulator::isBranchJ(const unsigned& opCode) {
    return opCode == 0x02u || opCode == 0x03u;
}

bool InstSimulator::hasToStall(const InstDataBin& inst) {
    // TODO: REWRITE
    return false;
}

bool InstSimulator::hasDependency(const InstDataBin& inst) {
    // TODO: REWRITE
    return false;
}

InstState InstSimulator::checkInstDependency(const InstDataBin& inst) {
    // TODO: REWRITE
    return InstState::NORMAL;
}

InstAction InstSimulator::detectWriteRegZero(const unsigned& addr) {
    if (!InstErrorDetector::isRegWritable(addr)) {
        fprintf(errorDump, "In cycle %u: Write $0 Error\n", cycle);
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectNumberOverflow(const int& a, const int& b, const InstOpType& op) {
    if (InstErrorDetector::isOverflowed(a, b, op)) {
        fprintf(errorDump, "In cycle %u: Number Overflow\n", cycle);
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectMemAddrOverflow(const unsigned& addr, const InstSize& type) {
    if (!InstErrorDetector::isValidMemoryAddr(addr, type)) {
        fprintf(errorDump, "In cycle %u: Address Overflow\n", cycle);
        alive = false;
        return InstAction::HALT;
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectDataMisaligned(const unsigned& addr, const InstSize& type) {
    if (!InstErrorDetector::isAlignedAddr(addr, type)) {
        fprintf(errorDump, "In cycle %u: Misalignment Error\n", cycle);
        alive = false;
        return InstAction::HALT;
    }
    return InstAction::CONTINUE;
}

} /* namespace lb */
