/*
 * InstMemory.cpp
 *
 *  Created on: 2016/03/10
 *      Author: LittleBird
 */

#include "InstMemory.h"

namespace lb {

InstMemory::InstMemory() {
    memset(this->reg, 0, sizeof(unsigned) * 32);
    memset(this->mem, 0, sizeof(unsigned char) * 1024);
}

InstMemory::~InstMemory() {

}

void InstMemory::init() {
    memset(reg, 0, sizeof(unsigned) * 32);
    memset(mem, 0, sizeof(unsigned char) * 1024);
}

unsigned InstMemory::getRegister(const unsigned& addr, const InstSize& type) const {
    if (type == InstSize::WORD) {
        return reg[addr];
    }
    else if (type == InstSize::HALF) {
        return reg[addr] & 0x0000FFFFu;
    }
    else {
        return reg[addr] & 0x000000FFu;
    }
}

void InstMemory::setRegister(const unsigned& addr, const unsigned& val, const InstSize& type) {
    if (addr == 0u) {
        return;
    }
    if (type == InstSize::WORD) {
        reg[addr] = val;
    }
    else if (type == InstSize::HALF) {
        reg[addr] = val & 0x0000FFFFu;
    }
    else {
        reg[addr] = val & 0x000000FFu;
    }
}

unsigned InstMemory::getMemory(const unsigned& addr, const InstSize& type) const {
    if (type == InstSize::WORD) {
        return (mem[addr] << 24) | (mem[addr + 1] << 16) | (mem[addr + 2] << 8) | mem[addr + 3];
    }
    else if (type == InstSize::HALF) {
        return (mem[addr] << 8) | (mem[addr + 1]);
    }
    else {
        return mem[addr];
    }
}

void InstMemory::setMemory(const unsigned& addr, const unsigned& val, const InstSize& type) {
    if (type == InstSize::WORD) {
        mem[addr] = static_cast<unsigned char>((val >> 24) & 0xFFu);
        mem[addr + 1] = static_cast<unsigned char>((val >> 16) & 0xFFu);
        mem[addr + 2] = static_cast<unsigned char>((val >> 8) & 0xFFu);
        mem[addr + 3] = static_cast<unsigned char>(val & 0xFFu);
    }
    else if (type == InstSize::HALF) {
        mem[addr] = static_cast<unsigned char>((val >> 8) & 0xFFu);
        mem[addr + 1] = static_cast<unsigned char>(val & 0xFFu);
    }
    else {
        mem[addr] = static_cast<unsigned char>(val & 0xFFu);
    }
}

} /* namespace lb */
