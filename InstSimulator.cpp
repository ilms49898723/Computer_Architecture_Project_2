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
    for (int i = 0; i < InstSimulator::MAXN; ++i) {
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

void InstSimulator::setLogFile(FILE* snapshot, FILE* errorDump) {
    this->snapshot = snapshot;
    this->errorDump = errorDump;
}

void InstSimulator::simulate() {
    if (!snapshot || !errorDump) {
        fprintf(stderr, "Log File Setting is not correct.\n");
        return;
    }
    pc = pcOriginal;
    cycle = 0u;
    alive = true;
    // fill pipeline with nop
    for (int i = 0; i < 5; ++i) {
        pipeline.push_back(InstPipelineData::nop);
    }
    while (!isFinished()) {
        // TODO: REWRITE
        // reset pcUpdated flag
        pcUpdated = false;
        // wb
        instWB();
        // dm
        instDM();
        // ex
        instEX();
        // id
        instID();
        // if
        instIF();
        // pop the last one(wb)
        instPop();
        // deal with stall, flush
        instPreprocess();
        // TODO: check dependency in current pipeline
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
                    fprintf(fp, " fwd_EX-DM_%s_$%d", (item.type == InstElementType::RS) ? "rs" : "rt", item.val);
                }
            }
            break;
        case EX:
            for (const auto& item : exForward) {
                fprintf(fp, " fwd_EX-DM_%s_$%d", (item.type == InstElementType::RS) ? "rs" : "rt", item.val);
            }
        default:
            break;
    }
}

void InstSimulator::instIF() {
    if (!pipeline.at(IF).isStalled()) {
        pipeline.push_front(instSet[pc >> 2]);
    }
    else {
        pcUpdated = true;
    }
}

void InstSimulator::instID() {
    // TODO: rewrite
    InstPipelineData& pipelineData = pipeline.at(ID);
    const InstDataBin& inst = pipeline.at(ID).getInst();
    if (isNOP(inst) || isHalt(inst)) {
        return;
    }
}

void InstSimulator::instEX() {
    InstPipelineData& pipelineData = pipeline.at(EX);
    const InstDataBin& inst = pipeline.at(EX).getInst();
    if (isNOP(inst) || isHalt(inst) || isBranch(inst)) {
        return;
    }
    else if (inst.getInstType() == InstType::R && inst.getFunct() != 0x08u) {
        // type-R, not jr
        pipelineData.setALUOut(instALUR(inst.getFunct()));
    }
    else if (inst.getInstType() == InstType::I && inst.getOpCode() != 0x04u && inst.getOpCode() != 0x05u && inst.getOpCode() != 0x07u) {
        // type-I, not beq, bne, bgtz
        pipelineData.setALUOut(instALUI(inst.getOpCode()));
    }
    else {
        return;
    }
}

void InstSimulator::instDM() {
    InstPipelineData& pipelineData = pipeline.at(DM);
    const InstDataBin& inst = pipeline.at(DM).getInst();
    if (isMemoryLoad(inst.getOpCode())) {
        const unsigned& ALUOut = pipelineData.getALUOut();
        const unsigned& MDR = instMemLoad(ALUOut, inst.getOpCode());
        pipelineData.setMDR(MDR);
    }
}

void InstSimulator::instWB() {
    const InstPipelineData& pipelineData = pipeline.at(WB);
    const InstDataBin& inst = pipeline.at(WB).getInst();
    if (isNOP(inst) || isHalt(inst)) {
        return;
    }
    if (inst.getRegWrite().empty()) {
        return;
    }
    if (isMemoryStore(inst.getOpCode())) {
        unsigned val = memory.getRegister(inst.getRt());
        instMemStore(pipelineData.getALUOut(), val, inst.getOpCode());
    }
    else {
        const unsigned& targetAddress = inst.getRegWrite().at(0).val;
        if (isMemoryLoad(inst.getOpCode())) {
            memory.setRegister(targetAddress, pipelineData.getMDR());
        }
        else {
            memory.setRegister(targetAddress, pipelineData.getALUOut());
        }
    }
}

void InstSimulator::instPop() {
    pipeline.pop_back();
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

void InstSimulator::instPreprocess() {
    // check every pipeline stage(stall, flush)
    if (pipeline.at(IF).isFlushed()) {
        pipeline.at(IF) = InstPipelineData::nop;
    }
    if (pipeline.at(ID).isStalled()) {
        pipeline.insert(pipeline.begin() + 2, InstPipelineData::nop);
    }
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

bool InstSimulator::hasToStall(const unsigned long long& step, const unsigned long long& dependency) {
    if (step != ID) {
        return false;
    }
    const std::vector<InstElement>& exWrite = pipeline.at(EX).getInst().getRegWrite();
    const std::vector<InstElement>& dmWrite = pipeline.at(DM).getInst().getRegWrite();
    const std::vector<InstElement>& idRead = pipeline.at(step).getInst().getRegRead();
    const InstDataBin& inst = pipeline.at(step).getInst();
    // no dependency
    if (dependency == STAGES) {
        return false;
    }
    // lw or lh or lb. Because no MEM/WB to EX forwarding, need stall
    if (isMemoryLoad(inst.getOpCode())) {
        return true;
    }
    // if is branch instruction, only can get from EX/DM] stage
    // if not branch instruction, only can get from [EX/DM stage
    if (isBranch(inst) && dependency < DM) {
        return true;
    }
    else {
        return dependency != EX;
    }
}

unsigned long long InstSimulator::getDependency(const unsigned long long& step) {
    // return STAGES: no dependency, EX: on ex, DM: on dm
    if (step != ID && step != EX) {
        return STAGES;
    }
    const std::vector<InstElement>& exWrite = pipeline.at(EX).getInst().getRegWrite();
    const std::vector<InstElement>& dmWrite = pipeline.at(DM).getInst().getRegWrite();
    const std::vector<InstElement>& idRead = pipeline.at(step).getInst().getRegRead();
    unsigned long long stage = STAGES;
    for (const auto& item : idRead) {
        if (!exWrite.empty() && item.val && item.val == exWrite.at(0).val) {
            stage = std::min(stage, EX);
        }
        if (!dmWrite.empty() && item.val && item.val == dmWrite.at(0).val) {
            stage = std::min(stage, DM);
        }
    }
    return stage;
}

InstState InstSimulator::checkStepDependency(const unsigned long long& step) {
    unsigned long long dependency = getDependency(step);
    if (dependency == STAGES) {
        return InstState::NONE;
    }
    else {
        bool stall = hasToStall(step, dependency);
        if (stall) {
            return InstState::STALL;
        }
        else {
            if (dependency == EX) {
                return InstState::EX;
            }
            else {
                return InstState::DM;
            }
        }
    }
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
