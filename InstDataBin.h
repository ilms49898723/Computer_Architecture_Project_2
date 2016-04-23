/*
 * InstDataBin.h
 *
 *  Created on: 2016/03/13
 *      Author: LittleBird
 */

#ifndef INSTDATABIN_H_
#define INSTDATABIN_H_

#include <string>
#include "InstLookUp.h"
#include "InstUtility.h"
#include "InstEnum.h"

namespace lb {

class InstDataBin {
public:
    InstDataBin();

    virtual ~InstDataBin();

    InstType getInstType() const;

    unsigned getOpCode() const;

    unsigned getRs() const;

    unsigned getRt() const;

    unsigned getRd() const;

    unsigned getC() const;

    unsigned getFunct() const;

    unsigned getInst() const;

    std::string getInstName() const;

    void setInstType(const InstType& val);

    void setOpCode(const unsigned& val);

    void setRs(const unsigned& val);

    void setRt(const unsigned& val);

    void setRd(const unsigned& val);

    void setC(const unsigned& val);

    void setFunct(const unsigned& val);

    void setInst(const unsigned& val);

    void setInstName(const unsigned& val);

private:
    InstType instType;
    unsigned opCode;
    unsigned rs;
    unsigned rt;
    unsigned rd;
    unsigned c;
    unsigned funct;
    unsigned inst;
    std::string instName;
};

} /* namespace lb */

#endif /* INSTDATABIN_H_ */
