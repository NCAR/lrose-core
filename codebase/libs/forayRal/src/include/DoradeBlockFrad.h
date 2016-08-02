//
//
//
//

#ifndef DORADEBLOCKFRAD_H
#define DORADEBLOCKFRAD_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockFrad : public DoradeBlock {
public:
    DoradeBlockFrad();
    ~DoradeBlockFrad();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKFRAD_H
