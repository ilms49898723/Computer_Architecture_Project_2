/*
 * InstData.h
 *
 *  Created on: 2016/03/09
 *      Author: LittleBird
 */

#ifndef INSTDATASTR_H_
#define INSTDATASTR_H_

#include <string>
#include "InstUtility.h"
#include "InstEnum.h"

namespace lb {

// Data structure to store readable Instruction set using "String"
// Use getXXX() function to access private members

class InstDataStr {
public:
    InstDataStr();
    virtual ~InstDataStr();
    InstType getType() const;
    std::string getOpCode() const;
    std::string getRs() const;
    std::string getRt() const;
    std::string getRd() const;
    std::string getC() const;
    std::string getFunct() const;
    void setType(const InstType& val);
    void setOpCode(const std::string& val);
    void setRs(const std::string &val);
    void setRt(const std::string &val);
    void setRd(const std::string &val);
    void setC(const std::string &val);
    void setFunct(const std::string &val);
    std::string toString() const;

private:
    InstType type;
    std::string opCode;
    std::string rs;
    std::string rt;
    std::string rd;
    std::string c;
    std::string funct;
};

} /* namespace lb */

#endif /* INSTDATASTR_H_ */
