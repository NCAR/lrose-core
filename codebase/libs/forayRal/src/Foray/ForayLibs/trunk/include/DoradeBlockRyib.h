//
//
//
//

#ifndef DORADEBLOCKRYIB_H
#define DORADEBLOCKRYIB_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockRyib : public DoradeBlock {
public:
    DoradeBlockRyib();
    ~DoradeBlockRyib();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockRyib  * castToDoradeBlockRyib();


private:

    const static std::string id_;
    const static int         length_;
    const static char       *rayStatus_[];

};


#endif // DORADEBLOCKRYIB_H
