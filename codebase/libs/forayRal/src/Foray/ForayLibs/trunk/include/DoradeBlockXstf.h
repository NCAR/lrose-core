//
//
//
//

#ifndef DORADEBLOCKXSTF_H
#define DORADEBLOCKXSTF_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockXstf : public DoradeBlock {
public:
    DoradeBlockXstf();
    ~DoradeBlockXstf();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

    virtual DoradeBlockXstf * castToDoradeBlockXstf();
    const unsigned char *content();

private:

    static std::string id_;

    const unsigned char *contentPointer_;

    ForayUtility::Buffer buffer_;
    

};


#endif // DORADEBLOCKXSTF_H
