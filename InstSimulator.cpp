/*
 * InstSimulator.cpp
 *
 *  Created on: 2016/03/11
 *      Author: LittleBird
 */

#include "InstSimulator.h"

namespace lb {

const unsigned InstSimulator::IF = 0u;
const unsigned InstSimulator::ID = 1u;
const unsigned InstSimulator::EX = 2u;
const unsigned InstSimulator::DM = 3u;
const unsigned InstSimulator::WB = 4u;

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
        instList[i] = InstDecoder::decodeInstBin(0u);
    }
}

void InstSimulator::loadImageI(const unsigned* src, const unsigned& len, const unsigned& pc) {
    this->pcOriginal = pc;
    unsigned instSetIdx = pc >> 2;
    for (unsigned i = 0; i < len; ++i) {
        instList[instSetIdx] = InstDecoder::decodeInstBin(src[i]);
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
        fprintf(stderr, "Can\'t open output files\n");
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
        instWB();
        instDM();
        instEX();
        instID();
        instIF();
        instPop();
        if (!alive) {
            break;
        }
        idForward.clear();
        exForward.clear();
        instSetDependency();
        dumpSnapshot(snapshot);
        ++cycle;
        if (!pipeline.at(IF).isStalled()) {
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
    if (pipeline.at(IF).isFlushed()) {
        pipeline.at(IF) = InstPipelineData::nop;
    }
    if (!pipeline.at(IF).isStalled()) {
        pipeline.push_front(InstPipelineData(instList[pc >> 2], pc));
    }
    else {
        pipeline.insert(pipeline.begin() + 2, InstPipelineData::nop);
    }
    instUnstall();
}

void InstSimulator::instID() {
    InstPipelineData& pipelineData = pipeline.at(ID);
    const InstDataBin& inst = pipeline.at(ID).getInst();
    if (isNOP(inst) || isHalt(inst) || !isBranch(inst)) {
        return;
    }
    if (pipelineData.getBranchResult()) {
        if (isBranchR(inst)) {
            pc = pipelineData.getValRs();
        }
        else if (isBranchI(inst)) {
            int newPc = toSigned(pipelineData.getInstPc()) + 4 + 4 * toSigned(pipelineData.getValC(), 16);
            pc = toUnsigned(newPc);
        }
        else {
            if (inst.getOpCode() == 0x03u) {
                pipelineData.setALUOut(instALUJ(pipelineData.getInstPc()));
            }
            pc = ((pipelineData.getInstPc() + 4) & 0xF0000000u) | (pipelineData.getValC() * 4);
        }
    }
}

void InstSimulator::instEX() {
    InstPipelineData& pipelineData = pipeline.at(EX);
    const InstDataBin& inst = pipeline.at(EX).getInst();
    if (isNOP(inst) || isHalt(inst) || isBranch(inst)) {
        return;
    }
    if (inst.getInstType() == InstType::R) {
        // type-R, not jr(isBranch)
        pipelineData.setALUOut(instALUR(inst));
    }
    else if (inst.getInstType() == InstType::I) {
        // type-I, not beq, bne, bgtz(isBranch)
        pipelineData.setALUOut(instALUI(inst));
    }
    else {
        return;
    }
}

void InstSimulator::instDM() {
    InstPipelineData& pipelineData = pipeline.at(DM);
    const InstDataBin& inst = pipeline.at(DM).getInst();
    if (isNOP(inst) || isHalt(inst)) {
        return;
    }
    if (isMemoryLoad(inst)) {
        const unsigned& ALUOut = pipelineData.getALUOut();
        InstAction action[2];
        action[0] = detectMemAddrOverflow(ALUOut, inst);
        action[1] = detectDataMisaligned(ALUOut, inst);
        if (action[0] == InstAction::HALT || action[1] == InstAction::HALT) {
            return;
        }
        const unsigned& MDR = instMemLoad(ALUOut, inst);
        pipelineData.setMDR(MDR);
    }
    else if (isMemoryStore(inst)) {
        const unsigned& ALUOut = pipelineData.getALUOut();
        InstAction action[2];
        action[0] = detectMemAddrOverflow(ALUOut, inst);
        action[1] = detectDataMisaligned(ALUOut, inst);
        if (action[0] == InstAction::HALT || action[1] == InstAction::HALT) {
            return;
        }
        const unsigned& val = memory.getRegister(inst.getRt());
        instMemStore(ALUOut, val, inst);
    }
}

void InstSimulator::instWB() {
    const InstPipelineData& pipelineData = pipeline.at(WB);
    const InstDataBin& inst = pipeline.at(WB).getInst();
    if (isNOP(inst) || isHalt(inst) || isMemoryStore(inst)) {
        return;
    }
    if (inst.getRegWrite().empty()) {
        return;
    }
    const unsigned& targetAddress = inst.getRegWrite().at(0).val;
    detectWriteRegZero(targetAddress);
    if (isMemoryLoad(inst)) {
        memory.setRegister(targetAddress, pipelineData.getMDR());
    }
    else {
        memory.setRegister(targetAddress, pipelineData.getALUOut());
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

void InstSimulator::instSetDependency() {
    instSetDependencyID();
    instSetDependencyEX();
}

void InstSimulator::instSetDependencyID() {
    InstPipelineData& pipelineData = pipeline.at(ID);
    const InstDataBin& inst = pipeline.at(ID).getInst();
    const std::vector<InstElement>& dmWrite = pipeline.at(DM).getInst().getRegWrite();
    const std::vector<InstElement>& idRead = pipeline.at(ID).getInst().getRegRead();
    if (isNOP(inst) || isHalt(inst)) {
        return;
    }
    InstState action = checkIDDependency();
    if (action == InstState::STALL) {
        instStall();
        return;
    }
    else if (isBranch(inst)) {
        const InstPipelineData& wbData = pipeline.at(WB);
        if (action == InstState::FORWARD) {
            for (const auto& item : idRead) {
                if (!dmWrite.empty() && item.val && item.val == dmWrite.at(0).val) {
                    pipelineData.setVal(pipeline.at(DM).getALUOut(), item.type);
                    idForward.push_back(item);
                }
                else {
                    if (!wbData.getInst().getRegWrite().empty() && item.val &&
                        wbData.getInst().getRegWrite().at(0).val == item.val) {
                        if (isMemoryLoad(wbData.getInst())) {
                            pipelineData.setVal(wbData.getMDR(), item.type);
                        }
                        else {
                            pipelineData.setVal(wbData.getALUOut(), item.type);
                        }
                    }
                    else {
                        pipelineData.setVal(memory.getRegister(item.val), item.type);
                    }
                }
            }
        }
        else {
            for (const auto& item : idRead) {
                if (!wbData.getInst().getRegWrite().empty() && item.val &&
                    wbData.getInst().getRegWrite().at(0).val == item.val) {
                    if (isMemoryLoad(wbData.getInst())) {
                        pipelineData.setVal(wbData.getMDR(), item.type);
                    }
                    else {
                        pipelineData.setVal(wbData.getALUOut(), item.type);
                    }
                }
                else {
                    pipelineData.setVal(memory.getRegister(item.val), item.type);
                }
            }
        }
        bool result = instPredictBranch();
        pipelineData.setBranchResult(result);
        if (result) {
            instFlush();
        }
    }
    else {
        return;
    }
}

void InstSimulator::instSetDependencyEX() {
    InstPipelineData& pipelineData = pipeline.at(EX);
    const InstDataBin& inst = pipeline.at(EX).getInst();
    const std::vector<InstElement>& dmWrite = pipeline.at(DM).getInst().getRegWrite();
    const std::vector<InstElement>& exRead = pipeline.at(EX).getInst().getRegRead();
    if (isNOP(inst) || isHalt(inst) || isBranch(inst)) {
        return;
    }
    for (const auto& item : exRead) {
        if (!dmWrite.empty() && item.val && item.val == dmWrite.at(0).val) {
            pipelineData.setVal(pipeline.at(DM).getALUOut(), item.type);
            exForward.push_back(item);
        }
        else {
            pipelineData.setVal(memory.getRegister(item.val), item.type);
        }
    }
}

bool InstSimulator::instPredictBranch() {
    const InstPipelineData& pipelineData = pipeline.at(ID);
    const InstDataBin& inst = pipeline.at(ID).getInst();
    if (isBranchR(inst)) {
        return true;
    }
    else if (isBranchI(inst)) {
        bool result;
        switch (inst.getOpCode()) {
            case 0x04u:
                result = (pipelineData.getValRs() == pipelineData.getValRt());
                break;
            case 0x05u:
                result = (pipelineData.getValRs() != pipelineData.getValRt());
                break;
            case 0x07u:
                result = (toSigned(pipelineData.getValRs()) > 0);
                break;
            default:
                result = false;
                break;
        }
        return result;
    }
    else {
        return true;
    }
}

unsigned InstSimulator::instALUR(const InstDataBin& inst) {
    const InstPipelineData& pipelineData = pipeline.at(EX);
    const unsigned& valRs = pipelineData.getValRs();
    const unsigned& valRt = pipelineData.getValRt();
    const unsigned& valC = pipelineData.getValC();
    detectNumberOverflow(toSigned(valRs), toSigned(valRt), inst);
    switch (inst.getFunct()) {
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
            return static_cast<unsigned>(toSigned(valRs) < toSigned(valRt));
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

unsigned InstSimulator::instALUI(const InstDataBin& inst) {
    const InstPipelineData& pipelineData = pipeline.at(EX);
    const unsigned& valRs = pipelineData.getValRs();
    const unsigned& valC = pipelineData.getValC();
    detectNumberOverflow(toSigned(valRs), toSigned(valC, 16), inst);
    switch (inst.getOpCode()) {
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

unsigned InstSimulator::instALUJ(const unsigned& instPc) {
    return instPc + 4;
}

unsigned InstSimulator::instMemLoad(const unsigned& addr, const InstDataBin& inst) {
    switch (inst.getOpCode()) {
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

void InstSimulator::instMemStore(const unsigned& addr, const unsigned& val, const InstDataBin& inst) {
    switch (inst.getOpCode()) {
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

bool InstSimulator::isMemoryLoad(const InstDataBin& inst) {
    switch (inst.getOpCode()) {
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

bool InstSimulator::isMemoryStore(const InstDataBin& inst) {
    switch (inst.getOpCode()) {
        case 0x2Bu:
        case 0x29u:
        case 0x28u:
            return true;
        default:
            return false;
    }
}

bool InstSimulator::isBranch(const InstDataBin& inst) {
    return isBranchR(inst) ||
           isBranchI(inst) ||
           isBranchJ(inst);
}

bool InstSimulator::isBranchR(const InstDataBin& inst) {
    return inst.getInstType() == InstType::R && inst.getFunct() == 0x08u;
}

bool InstSimulator::isBranchI(const InstDataBin& inst) {
    if (inst.getInstType() != InstType::I) {
        return false;
    }
    return inst.getOpCode() == 0x04u || inst.getOpCode() == 0x05u || inst.getOpCode() == 0x07u;
}

bool InstSimulator::isBranchJ(const InstDataBin& inst) {
    if (inst.getInstType() != InstType::J) {
        return false;
    }
    return inst.getOpCode() == 0x02u || inst.getOpCode() == 0x03u;
}

bool InstSimulator::hasToStall(const unsigned& dependency, const std::vector<unsigned>& dEX,
                               const std::vector<unsigned>& dDM) {
    const InstDataBin& inst = pipeline.at(ID).getInst();
    // no dependency
    if (dependency == 0u) {
        return false;
    }
    // load memory -> stall
    if (!dEX.empty() && isMemoryLoad(pipeline.at(EX).getInst())) {
        return true;
    }
    if (!dDM.empty() && isMemoryLoad(pipeline.at(DM).getInst())) {
        return true;
    }
    // depends on both ex, dm
    if (!dEX.empty() && !dDM.empty()) {
        return true;
    }
    if (isBranch(inst)) {
        return !dEX.empty();
    }
    else {
        return !dDM.empty();
    }
}

unsigned InstSimulator::getDependency(std::vector<unsigned>& dEX, std::vector<unsigned>& dDM) {
    // return 0: no dependency,
    // & (1u << EX) == 1: on ex
    // & (1u << DM) == 1: on dm
    const std::vector<InstElement>& exWrite = pipeline.at(EX).getInst().getRegWrite();
    const std::vector<InstElement>& dmWrite = pipeline.at(DM).getInst().getRegWrite();
    const std::vector<InstElement>& idRead = pipeline.at(ID).getInst().getRegRead();
    unsigned stage = 0u;
    for (const auto& item : idRead) {
        if (!exWrite.empty() && item.val && item.val == exWrite.at(0).val) {
            stage |= (1u << EX);
            dEX.push_back(item.val);
        }
        else if (!dmWrite.empty() && item.val && item.val == dmWrite.at(0).val) {
            stage |= (1u << DM);
            dDM.push_back(item.val);
        }
    }
    return stage;
}

InstState InstSimulator::checkIDDependency() {
    std::vector<unsigned> dEX;
    std::vector<unsigned> dDM;
    unsigned dependency = getDependency(dEX, dDM);
    if (dependency == 0u) {
        return InstState::NONE;
    }
    else {
        bool stall = hasToStall(dependency, dEX, dDM);
        if (stall) {
            return InstState::STALL;
        }
        else {
            return InstState::FORWARD;
        }
    }
}

InstAction InstSimulator::detectWriteRegZero(const unsigned& addr) {
    if (!InstErrorDetector::isRegWritable(addr)) {
        fprintf(errorDump, "In cycle %u: Write $0 Error\n", cycle);
    }
    return InstAction::CONTINUE;
}

InstAction InstSimulator::detectNumberOverflow(const int& a, const int& b, const InstDataBin& inst) {
    if (inst.getInstType() == InstType::R) {
        switch (inst.getFunct()) {
            case 0x20u:
                if (InstErrorDetector::isOverflowed(a, b, InstOpType::ADD)) {
                    fprintf(errorDump, "In cycle %u: Number Overflow\n", cycle);
                }
                return InstAction::CONTINUE;
            case 0x22u:
                if (InstErrorDetector::isOverflowed(a, b, InstOpType::SUB)) {
                    fprintf(errorDump, "In cycle %u: Number Overflow\n", cycle);
                }
                return InstAction::CONTINUE;
            default:
                return InstAction::CONTINUE;
        }
    }
    else if (inst.getInstType() == InstType::I) {
        switch (inst.getOpCode()) {
            case 0x08u:
            case 0x23u:
            case 0x21u:
            case 0x25u:
            case 0x20u:
            case 0x24u:
            case 0x2Bu:
            case 0x29u:
            case 0x28u:
                if (InstErrorDetector::isOverflowed(a, b, InstOpType::ADD)) {
                    fprintf(errorDump, "In cycle %u: Number Overflow\n", cycle);
                }
                return InstAction::CONTINUE;
            default:
                return InstAction::CONTINUE;
        }
    }
    else {
        return InstAction::CONTINUE;
    }
}

InstAction InstSimulator::detectMemAddrOverflow(const unsigned& addr, const InstDataBin& inst) {
    switch (inst.getOpCode()) {
        case 0x23u:
        case 0x2Bu:
            if (!InstErrorDetector::isValidMemoryAddr(addr, InstSize::WORD)) {
                fprintf(errorDump, "In cycle %u: Address Overflow\n", cycle);
                alive = false;
                return InstAction::HALT;
            }
            return InstAction::CONTINUE;
        case 0x21u:
        case 0x25u:
        case 0x29u:
            if (!InstErrorDetector::isValidMemoryAddr(addr, InstSize::HALF)) {
                fprintf(errorDump, "In cycle %u: Address Overflow\n", cycle);
                alive = false;
                return InstAction::HALT;
            }
            return InstAction::CONTINUE;
        case 0x20u:
        case 0x24u:
        case 0x28u:
            if (!InstErrorDetector::isValidMemoryAddr(addr, InstSize::BYTE)) {
                fprintf(errorDump, "In cycle %u: Address Overflow\n", cycle);
                alive = false;
                return InstAction::HALT;
            }
            return InstAction::CONTINUE;
        default:
            return InstAction::CONTINUE;
    }
}

InstAction InstSimulator::detectDataMisaligned(const unsigned& addr, const InstDataBin& inst) {
    switch (inst.getOpCode()) {
        case 0x23u:
        case 0x2Bu:
            if (!InstErrorDetector::isAlignedAddr(addr, InstSize::WORD)) {
                fprintf(errorDump, "In cycle %u: Misalignment Error\n", cycle);
                alive = false;
                return InstAction::HALT;
            }
            return InstAction::CONTINUE;
        case 0x21u:
        case 0x25u:
        case 0x29u:
            if (!InstErrorDetector::isAlignedAddr(addr, InstSize::HALF)) {
                fprintf(errorDump, "In cycle %u: Misalignment Error\n", cycle);
                alive = false;
                return InstAction::HALT;
            }
            return InstAction::CONTINUE;
        case 0x20u:
        case 0x24u:
        case 0x28u:
            if (!InstErrorDetector::isAlignedAddr(addr, InstSize::BYTE)) {
                fprintf(errorDump, "In cycle %u: Misalignment Error\n", cycle);
                alive = false;
                return InstAction::HALT;
            }
            return InstAction::CONTINUE;
        default:
            return InstAction::CONTINUE;
    }
}

} /* namespace lb */
