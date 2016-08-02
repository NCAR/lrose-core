//
//
//
//

#ifndef DORADEBLOCKSWIB_H
#define DORADEBLOCKSWIB_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockSwib : public DoradeBlock {
public:
    DoradeBlockSwib();
    ~DoradeBlockSwib();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockSwib *castToDoradeBlockSwib();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKSWIB_H
