//
//
//
//

#ifndef DORADEBLOCKASIB_H
#define DORADEBLOCKASIB_H

#include <string>

#include "Fault.h"
#include "Buffer.h"

#include "DoradeBlock.h"

class DoradeBlockAsib : public DoradeBlock {
public:
    DoradeBlockAsib();
    ~DoradeBlockAsib();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKASIB_H
