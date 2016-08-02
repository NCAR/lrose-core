//
//
//
//

#ifndef DORADEBLOCKNDDS_H
#define DORADEBLOCKNDDS_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockNdds : public DoradeBlock {
public:
    DoradeBlockNdds();
    ~DoradeBlockNdds();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKNDDS_H
