//
//
//
//

#ifndef DORADEBLOCKWAVE_H
#define DORADEBLOCKWAVE_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockWave : public DoradeBlock {
public:
    DoradeBlockWave();
    ~DoradeBlockWave();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKWAVE_H
