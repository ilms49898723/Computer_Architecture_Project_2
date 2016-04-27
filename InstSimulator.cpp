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
    memory.setRegister(29, sp, InstSize::WORD);
    for (unsigned i = 0; i < len; ++i) {
        memory.setMemory(i * 4, src[i], InstSize::WORD);
    }
}

void InstSimulator::simulate(FILE* snapshot, FILE* errorDump) {
    this->snapshot = snapshot;
    this->errorDump = errorDump;
    for (int i = 0; i < 5; ++i) {
        pipeline.push_back(InstPipelineData::nop);
    }
    while (!isFinished()) {
        printf("size of pipeline %llu\n", pipeline.size());
        pcUpdated = false;
        instWB();
        instDM();
        instEX();
        instID();
        instIF();
        dumpSnapshot(stdout);
        dumpSnapshot(snapshot);
        instCleanUp();
        ++cycle;
        if (!pcUpdated) {
            pc += 4;
        }
        pcUpdated = false;
    }
}

void InstSimulator::dumpSnapshot(FILE* fp) {
    fprintf(fp, "cycle %d\n", cycle);
    for (unsigned i = 0; i < 32; ++i) {
        fprintf(fp, "$%02d: 0x%08X\n", i, memory.getRegister(i));
    }
    fprintf(fp, "PC: 0x%08X\n", pc);
    fprintf(fp, "IF: 0x%08X", pipeline.at(sIF).getInst().getInst());
    dumpPipelineInfo(fp, sIF);
    fprintf(fp, "\n");
    fprintf(fp, "ID: %s", pipeline.at(sID).getInst().getInstName().c_str());
    dumpPipelineInfo(fp, sID);
    fprintf(fp, "\n");
    fprintf(fp, "EX: %s", pipeline.at(sEX).getInst().getInstName().c_str());
    dumpPipelineInfo(fp, sEX);
    fprintf(fp, "\n");
    fprintf(fp, "DM: %s\n", pipeline.at(sDM).getInst().getInstName().c_str());
    fprintf(fp, "WB: %s\n", pipeline.at(sWB).getInst().getInstName().c_str());
    fprintf(fp, "\n\n");
}

void InstSimulator::dumpPipelineInfo(FILE* fp, const int stage) {
    switch (stage) {
        case sIF:
            if (pipeline.at(sIF).isFlushed()) {
                fprintf(fp, " to_be_flushed");
            }
            break;
        case sID:
            if (pipeline.at(sID).isStalled()) {
                fprintf(fp, " to_be_stalled");
            }
            else {
                for (const auto& item : idForward) {
                    fprintf(fp, " fwd_EX-DM_%s_$%d", item.type == InstElementType::RS ? "rs" : "rt", item.val);
                }
            }
            break;
        case sEX:
            for (const auto& item : exForward) {
                fprintf(fp, " fwd_EX-DM_%s_$%d", item.type == InstElementType::RS ? "rs" : "rt", item.val);
            }
        default:
            break;
    }
}

void InstSimulator::instIF() {
    if (pipeline.size() <= sStages) {
        instPush(pc);
    }
    else {
        pcUpdated = true;
    }
}

void InstSimulator::instID() {
    InstPipelineData& current = pipeline.at(sID);
    const InstDataBin& currentInst = pipeline.at(sID).getInst();
    if (isNOP(currentInst) || isHalt(currentInst)) {
        return;
    }
    InstState op = checkInstDependency(currentInst);
    // for stall
    if (op == InstState::STALL) {
        instStall();
        return;
    }
    // check forwarding, get register value
    if (op == InstState::FORWARD) {
        const std::vector<InstElement>& exWrite = pipeline.at(sEX).getInst().getRegWrite();
        for (const auto& item : currentInst.getRegRead()) {
            if (item.val == exWrite.at(0).val) {
                if (item.type == InstElementType::RS) {
                    current.setValRs(pipeline.at(sEX).getALUOut());
                }
                else if (item.type == InstElementType::RT) {
                    current.setValRt(pipeline.at(sEX).getALUOut());
                }
                if (isBranch(currentInst)) {
                    idForward.push_back(item);
                }
                else {
                    exForward.push_back(item);
                }
            }
        }
    }
    else {
        const unsigned valRs = memory.getRegister(currentInst.getRs());
        const unsigned valRt = memory.getRegister(currentInst.getRt());
        current.setValRs(valRs);
        current.setValRt(valRt);
    }
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
    const InstDataBin& current = pipeline.at(sEX).getInst();
    if (isNOP(current) || isHalt(current) || isBranch(current)) {
        return;
    }
    else if (current.getInstType() == InstType::R && current.getFunct() != 0x08u) {
        // type-R, not jr
        pipeline.at(sEX).setALUOut(instALUR(current.getFunct()));
    }
    else if (current.getInstType() == InstType::I && current.getOpCode() != 0x04u && current.getOpCode() != 0x05u && current.getOpCode() != 0x07u) {
        // type-I, not beq, bne, bgtz
        pipeline.at(sEX).setALUOut(instALUI(current.getOpCode()));
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
        memory.setRegister(current.getInst().getRegWrite().at(0).val, current.getALUOut());
    }
}

void InstSimulator::instPush(const unsigned& pc) {
    pipeline.push_front(instSet[pc >> 2]);
}

void InstSimulator::instPop() {
    pipeline.pop_back();
}

void InstSimulator::instStall() {
    pipeline.at(sIF).setStalled(true);
    pipeline.at(sID).setStalled(true);
}

void InstSimulator::instUnstall() {
    pipeline.at(sIF).setStalled(false);
    pipeline.at(sID).setStalled(false);
}

void InstSimulator::instFlush() {
    pipeline.at(sIF).setFlushed(true);
}

void InstSimulator::instCleanUp() {
    if (pipeline.at(sIF).isFlushed()) {
        pipeline[sIF] = InstPipelineData::nop;
    }
    if (pipeline.at(sID).isStalled()) {
        pipeline.insert(pipeline.begin() + 2, InstPipelineData::nop);
    }
    instUnstall();
    instPop();
    idForward.clear();
    exForward.clear();
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

bool InstSimulator::checkInst(const InstDataBin& inst) {
    if (inst.getInstType() == InstType::R) {
        // write $0 error
        if (inst.getFunct() != 0x08u && !isNOP(inst)) { // jr
            detectWriteRegZero(inst.getRd());
        }
        // number overflow
        if (inst.getFunct() == 0x20u) { // add
            int rs, rt;
            rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
            rt = toSigned(memory.getRegister(inst.getRt(), InstSize::WORD));
            detectNumberOverflow(rs, rt, InstOpType::ADD);
        }
        if (inst.getFunct() == 0x22u) { // sub
            int rs, rt;
            rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
            rt = toSigned(memory.getRegister(inst.getRt(), InstSize::WORD));
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
            rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
            c = toSigned(inst.getC(), 16);
            detectNumberOverflow(rs, c, InstOpType::ADD);
        }
        // Address Memory overflow
        if (inst.getOpCode() == 0x23u || // lw
            inst.getOpCode() == 0x2Bu) { // sw
            int rs, c;
            rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstSize::WORD);
        }
        if (inst.getOpCode() == 0x21u || // lh
            inst.getOpCode() == 0x25u || // lhu
            inst.getOpCode() == 0x29u) { // sh
            int rs, c;
            rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstSize::HALF);
        }
        if (inst.getOpCode() == 0x20u || // lb
            inst.getOpCode() == 0x24u || // lbu
            inst.getOpCode() == 0x28u) { // sb
            int rs, c;
            rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
            c = toSigned(inst.getC(), 16);
            detectMemAddrOverflow(toUnsigned(rs + c), InstSize::BYTE);
        }
        // Data misaligned
        switch (inst.getOpCode()) {
            case 0x23u: {
                // lw
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::WORD);
                break;
            }
            case 0x21u: {
                // lh
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::HALF);
                break;
            }
            case 0x25u: {
                // lhu
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::HALF);
                break;
            }
            case 0x20u: {
                // lb
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::BYTE);
                break;
            }
            case 0x24u: {
                // lbu
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::BYTE);
                break;
            }
            case 0x2Bu: {
                // sw
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::WORD);
                break;
            }
            case 0x29u: {
                // sh
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::HALF);
                break;
            }
            case 0x28u: {
                // sb
                int rs, c;
                rs = toSigned(memory.getRegister(inst.getRs(), InstSize::WORD));
                c = toSigned(inst.getC(), 16);
                detectDataMisaligned(toUnsigned(rs + c), InstSize::BYTE);
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
    const std::vector<InstElement>& dmWrite = pipeline.at(sDM).getInst().getRegWrite();
    for (const auto& item : inst.getRegRead()) {
        if (!dmWrite.empty() && item.val && item.val == dmWrite.at(0).val) {
            return true;
        }
    }
    return false;
}

bool InstSimulator::hasDependency(const InstDataBin& inst) {
    const std::vector<InstElement>& exWrite = pipeline.at(sEX).getInst().getRegWrite();
    const std::vector<InstElement>& dmWrite = pipeline.at(sDM).getInst().getRegWrite();
    for (const auto& item : inst.getRegRead()) {
        if (!exWrite.empty() && item.val && item.val == exWrite.at(0).val) {
            return true;
        }
        if (!dmWrite.empty() && item.val && item.val == dmWrite.at(0).val) {
            return true;
        }
    }
    return false;
}

InstState InstSimulator::checkInstDependency(const InstDataBin& inst) {
    if (hasDependency(inst)) {
        if (hasToStall(inst)) {
            return InstState::STALL;
        }
        else {
            return InstState::FORWARD;
        }
    }
    else {
        return InstState::NORMAL;
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

InstAction InstSimulator::detectMemAddrOverflow(const unsigned& addr, const InstSize& type) {
    if (!InstErrorDetector::isValidMemoryAddr(addr, type)) {
        fprintf(errorDump, "In cycle %d: Address Overflow\n", cycle);
        isAlive = false;
        return InstAction::HALT;
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectDataMisaligned(const unsigned& addr, const InstSize& type) {
    if (!InstErrorDetector::isAlignedAddr(addr, type)) {
        fprintf(errorDump, "In cycle %d: Misalignment Error\n", cycle);
        isAlive = false;
        return InstAction::HALT;
    }
    return InstAction::CONTINUE;
}

} /* namespace lb */
