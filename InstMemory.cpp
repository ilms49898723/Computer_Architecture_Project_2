/*
 * InstMemory.cpp
 *
 *  Created on: 2016/03/10
 *      Author: LittleBird
 */

#include "InstMemory.h"

namespace lb {

InstMemory::InstMemory() {
    pc = 0u;
    memset(reg, 0, sizeof(unsigned) * 32);
    memset(mem, 0, sizeof(unsigned char) * 1024);
}

InstMemory::InstMemory(const unsigned& initPc) {
    pc = initPc;
    memset(reg, 0, sizeof(unsigned) * 32);
    memset(mem, 0, sizeof(unsigned char) * 1024);
}

InstMemory::~InstMemory() {
}

void InstMemory::init(const unsigned& initPc) {
    pc = initPc;
    memset(reg, 0, sizeof(unsigned) * 32);
    memset(mem, 0, sizeof(unsigned char) * 1024);
}

unsigned InstMemory::getRegValue(const unsigned& addr, const InstMemLen& type) const {
    if (type == InstMemLen::WORD) {
        return reg[addr];
    }
    else if (type == InstMemLen::HALF) {
        return reg[addr] & 0x0000FFFFu;
    }
    else {
        return reg[addr] & 0x000000FFu;
    }
}

void InstMemory::setRegValue(const unsigned& addr, const unsigned& val, const InstMemLen& type) {
    if (addr == 0u) {
        return;
    }
    if (type == InstMemLen::WORD) {
        reg[addr] = val;
    }
    else if (type == InstMemLen::HALF) {
        reg[addr] = val & 0x0000FFFFu;
    }
    else {
        reg[addr] = val & 0x000000FFu;
    }
}

unsigned InstMemory::getMemValue(const unsigned& addr, const InstMemLen& type) const {
    if (type == InstMemLen::WORD) {
        return (mem[addr] << 24) | (mem[addr + 1] << 16) | (mem[addr + 2] << 8) | mem[addr + 3];
    }
    else if (type == InstMemLen::HALF) {
        return (mem[addr] << 8) | (mem[addr + 1]);
    }
    else {
        return mem[addr];
    }
}

void InstMemory::setMemValue(const unsigned& addr, const unsigned& val, const InstMemLen& type) {
    if (type == InstMemLen::WORD) {
        mem[addr] = static_cast<unsigned char>((val >> 24) & 0xFFu);
        mem[addr + 1] = static_cast<unsigned char>((val >> 16) & 0xFFu);
        mem[addr + 2] = static_cast<unsigned char>((val >> 8) & 0xFFu);
        mem[addr + 3] = static_cast<unsigned char>(val & 0xFFu);
    }
    else if (type == InstMemLen::HALF) {
        mem[addr] = static_cast<unsigned char>((val >> 8) & 0xFFu);
        mem[addr + 1] = static_cast<unsigned char>(val & 0xFFu);
    }
    else {
        mem[addr] = static_cast<unsigned char>(val & 0xFFu);
    }
}

unsigned InstMemory::getPc() const {
    return pc;
}

void InstMemory::setPc(const unsigned& val) {
    pc = val;
}

} /* namespace lb */
