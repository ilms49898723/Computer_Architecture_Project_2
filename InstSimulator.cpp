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
    memory.init();
    cycle = 0;
    snapshot = nullptr;
    errorDump = nullptr;
    isAlive = true;
    for (int i = 0; i < InstSimulator::maxn; ++i) {
        instSet[i] = InstDecoder::decodeInstBin(0u);
    }
}

void InstSimulator::loadImageI(const unsigned* src, const unsigned& len, const unsigned& pc) {
    memory.setPc(pc);
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
    isAlive = true;
    cycle = 0;
    int currentInstIdx = memory.getPc() >> 2;
    {
        // initial state dump
        dumpMemoryInfo(cycle);
        ++cycle;
    }
    while (instSet[currentInstIdx].getOpCode() != 0x3Fu) {
        InstDataBin& current = instSet[currentInstIdx];
        if (!checkInst(current)) {
            break;
        }
        if (current.getType() == InstType::R) {
            simulateTypeR(current);
        }
        else if (current.getType() == InstType::I) {
            simulateTypeI(current);
        }
        else if (current.getType() == InstType::J) {
            simulateTypeJ(current);
        }
        dumpMemoryInfo(cycle);
        currentInstIdx = memory.getPc() >> 2;
        ++cycle;
    }
}

void InstSimulator::dumpMemoryInfo(const int& cycle) {
    fprintf(snapshot, "cycle %d\n", cycle);
    for (unsigned i = 0; i < 32; ++i) {
        fprintf(snapshot, "$%02d: 0x%08X\n", i, memory.getRegValue(i, InstMemLen::WORD));
    }
    fprintf(snapshot, "PC: 0x%08X\n", memory.getPc());
    fprintf(snapshot, "\n\n");
}

void InstSimulator::simulateTypeR(const InstDataBin& inst) {
    switch (inst.getFunct()) {
        case 0x20u: {
            // add, rd = rs + rt
            int rs, rt;
            unsigned rd;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegValue(inst.getRt(), InstMemLen::WORD));
            rd = toUnsigned(rs + rt);
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x21u: {
            // addu, rd = rs + rt
            unsigned rs, rt, rd;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            rd = rs + rt;
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x22u: {
            // sub, rd = rs - rt
            int rs, rt;
            unsigned rd;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegValue(inst.getRt(), InstMemLen::WORD));
            rd = toUnsigned(rs - rt);
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x24u: {
            // and, rd = rs & rt
            unsigned rs, rt, rd;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            rd = rs & rt;
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x25u: {
            // or, rd = rs | rt
            unsigned rs, rt, rd;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            rd = rs | rt;
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x26u: {
            // xor, rd = rs ^ rt
            unsigned rs, rt, rd;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            rd = rs ^ rt;
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x27u: {
            // nor, rd = ~(rs | rt)
            unsigned rs, rt, rd;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            rd = ~(rs | rt);
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x28u: {
            // nand, rd = ~(rs & rt)
            unsigned rs, rt, rd;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            rd = ~(rs & rt);
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x2Au: {
            // slt, rd = (rs < rt)
            int rs, rt;
            unsigned rd;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            rt = toSigned(memory.getRegValue(inst.getRt(), InstMemLen::WORD));
            rd = toUnsigned(rs < rt);
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x00u: {
            // sll, rd = rt << c
            // here no need to check write to $zero if sll $0, $0, 0(nop)
            unsigned rd, rt, c;
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            c = inst.getC();
            rd = rt << c;
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x02u: {
            // srl, rd = rt >> c
            unsigned rd, rt, c;
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            c = inst.getC();
            rd = rt >> c;
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x03u: {
            // sra, rd = rt >> c(with sign bit)
            int rt, c;
            unsigned rd;
            rt = toSigned(memory.getRegValue(inst.getRt(), InstMemLen::WORD));
            c = toSigned(inst.getC());
            rd = toUnsigned(rt >> c);
            memory.setRegValue(inst.getRd(), rd, InstMemLen::WORD);
            break;
        }
        case 0x08u: {
            // jr, pc = rs
            unsigned rs;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            memory.setPc(rs);
            // new pc has already been set, return, not break
            return;
        }
        default: {
            // undefined operation
            break;
        }
    }
    memory.setPc(memory.getPc() + 4);
}

void InstSimulator::simulateTypeI(const InstDataBin& inst) {
    switch(inst.getOpCode()) {
        case 0x08u: {
            // addi, rt = rs + c
            int rs, c;
            unsigned rt;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            rt = toUnsigned(rs + c);
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x09u: {
            // addiu, rt = rs + c(unsigned)
            unsigned rt, rs, c;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            c = toUnsigned(toSigned(inst.getC(), 16));
            rt = rs + c;
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x23u: {
            // lw, rt = mem[rs + c], 4 bytes
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getMemValue(addr, InstMemLen::WORD);
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x21u: {
            // lh, rt = mem[rs + c], 2 bytes, signed
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getMemValue(addr, InstMemLen::HALF);
            rt = toUnsigned(toSigned(rt, InstMemLen::HALF));
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x25u: {
            // lhu, rt = mem[rs + c], 2 bytes, unsigned
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getMemValue(addr, InstMemLen::HALF);
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x20u: {
            // lb, rt = mem[rs + c], 1 byte, signed
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getMemValue(addr, InstMemLen::BYTE);
            rt = toUnsigned(toSigned(rt, InstMemLen::BYTE));
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x24u: {
            // lbu, rt = mem[rs + c], 1 byte, unsigned
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getMemValue(addr, InstMemLen::BYTE);
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x2Bu: {
            // sw, mem[rs + c] = rt, 4 bytes
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            memory.setMemValue(addr, rt, InstMemLen::WORD);
            break;
        }
        case 0x29u: {
            // sh, mem[rs + c] = rt, 2 bytes
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            memory.setMemValue(addr, rt, InstMemLen::HALF);
            break;
        }
        case 0x28u: {
            // sb, mem[rs + c] = rt, 1 byte
            int rs, c;
            unsigned rt;
            unsigned addr;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            addr = toUnsigned(rs + c);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            memory.setMemValue(addr, rt, InstMemLen::BYTE);
            break;
        }
        case 0x0Fu: {
            // lui, rt = c << 16
            unsigned rt;
            rt = inst.getC() << 16;
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x0Cu: {
            // andi, rt = rs & c
            unsigned rt, rs, c;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            c = inst.getC();
            rt = rs & c;
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x0Du: {
            // ori, rt = rs | c
            unsigned rt, rs, c;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            c = inst.getC();
            rt = rs | c;
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x0Eu: {
            // nori, rt = ~(rs | c)
            unsigned rt, rs, c;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            c = inst.getC();
            rt = ~(rs | c);
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x0Au: {
            // slti, rt = (rs < c(signed))
            int rs, c;
            unsigned rt;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            rt = toUnsigned(rs < c);
            memory.setRegValue(inst.getRt(), rt, InstMemLen::WORD);
            break;
        }
        case 0x04u: {
            // beq, if (rs == rt), go to (pc + 4 + 4 * c(signed))
            unsigned rs, rt;
            int c;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            c = toSigned(inst.getC(), 16);
            unsigned result = static_cast<unsigned>(toSigned(rs) == toSigned(rt));
            if (result == 1u) {
                memory.setPc(memory.getPc() + 4 + 4 * c);
                return;
            }
            break;
        }
        case 0x05u: {
            // bne, if (rs != rt), go to (pc + 4 + 4 * c(signed))
            unsigned rs, rt;
            int c;
            rs = memory.getRegValue(inst.getRs(), InstMemLen::WORD);
            rt = memory.getRegValue(inst.getRt(), InstMemLen::WORD);
            c = toSigned(inst.getC(), 16);
            unsigned result = static_cast<unsigned>(toSigned(rs) != toSigned(rt));
            if (result == 1u) {
                memory.setPc(memory.getPc() + 4 + 4 * c);
                return;
            }
            break;
        }
        case 0x07u: {
            // bgtz, if (rs > 0), go to (pc + 4 + 4 * c(signed))
            int rs;
            int c;
            rs = toSigned(memory.getRegValue(inst.getRs(), InstMemLen::WORD));
            c = toSigned(inst.getC(), 16);
            unsigned rt = toUnsigned(rs > 0);
            if (rt == 1u) {
                memory.setPc(memory.getPc() + 4 + 4 * c);
                return;
            }
            break;
        }
        default: {
            // undefined operation
            break;
        }
    }
    memory.setPc(memory.getPc() + 4);
}

void InstSimulator::simulateTypeJ(const InstDataBin& inst) {
    switch (inst.getOpCode()) {
        case 0x02u: {
            // j, pc = (pc + 4)[31:28] | 4 * c(unsigned)
            unsigned c = inst.getC();
            unsigned pc = (getBitsInRange(memory.getPc() + 4, 28, 32) << 28) | (c << 2);
            memory.setPc(pc);
            break;
        }
        case 0x03u: {
            // jal, $31 = pc + 4
            //      pc = (pc + 4)[31:28] | 4 * c(unsigned)
            memory.setRegValue(31, memory.getPc() + 4, InstMemLen::WORD);
            unsigned c = inst.getC();
            unsigned pc = (getBitsInRange(memory.getPc() + 4, 28, 32) << 28) | (c << 2);
            memory.setPc(pc);
            break;
        }
        default: {
            break;
        }
    }
}

bool InstSimulator::checkInst(const InstDataBin& inst) {
    if (inst.getType() == InstType::R) {
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
    else if (inst.getType() == InstType::I) {
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
    else if (inst.getType() == InstType::J) {
        return isAlive;
    }
    else {
        return isAlive;
    }
    return isAlive;
}

bool InstSimulator::isNOP(const InstDataBin &inst) {
    return !inst.getOpCode() &&
           !inst.getRt() &&
           !inst.getRd() &&
           !inst.getC() &&
           !inst.getFunct();
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
