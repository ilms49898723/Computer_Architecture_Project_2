/*
 * InstLookUp.h
 *
 *  Created on: 2016/03/09
 *      Author: LittleBird
 */

#ifndef INSTLOOKUP_H_
#define INSTLOOKUP_H_

#include <string>
#include "InstUtility.h"

namespace lb {

class InstLookUp {
public:
    /**
     * translate opCode -> readable string
     * @param src opCode to translate
     */
    static std::string opCodeLookUp(const unsigned& src);

    /**
     * translate funct -> readable string
     * @param src funct to translate
     */
    static std::string functLookUp(const unsigned& src);

    /**
     * translate register name -> numbers
     * @param src register name to translate
     */
    static std::string registerLookUpNumber(const unsigned& src);

private:
    const static std::string opCodeLookUpTable[];
    const static std::string functLookUpTable[];
};

} /* namespace lb */

#endif /* INSTLOOKUP_H_ */
