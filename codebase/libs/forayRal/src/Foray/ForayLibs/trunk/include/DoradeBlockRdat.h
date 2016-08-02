//
//
//
//

#ifndef DORADEBLOCKRDAT_H
#define DORADEBLOCKRDAT_H

#include <string>

#include "vectorDefs.h"

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockRdat : public DoradeBlock {
public:
    DoradeBlockRdat();
    ~DoradeBlockRdat();
    
    static bool  test  (ForayUtility::Buffer &buffer)                 throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer)                 throw(ForayUtility::Fault);

    void         encode(ForayUtility::Buffer &buffer)                 throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer, const int binaryFormat,
			RayIntegers &ri)                              throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer, const int binaryFormat,
			RayDoubles &rd)                               throw(ForayUtility::Fault);

    virtual std::string  listEntry();
    virtual DoradeBlockRdat * castToDoradeBlockRdat();

    bool         dataIsInteger(const int binaryFormat) throw(ForayUtility::Fault);  // should be static method

    void         decodeIntegerData(const int binaryFormat,RayIntegers * const ri) throw(ForayUtility::Fault);


				   


private:

    static std::string id_;

    ForayUtility::Buffer buffer_;

};


#endif // DORADEBLOCKRDAT_H
