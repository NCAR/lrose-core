//
//
//
//

#ifndef DORADEBLOCKPARM_H
#define DORADEBLOCKPARM_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockParm : public DoradeBlock {
public:
    DoradeBlockParm();
    ~DoradeBlockParm();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockParm * castToDoradeBlockParm();

private:

    static std::string id_;
    static int         lengthA_;
    static int         lengthB_;

};


#endif // DORADEBLOCKPARM_H
