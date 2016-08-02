//
//
//
//

#ifndef DORADEBLOCKCSFD_H
#define DORADEBLOCKCSFD_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockCsfd : public DoradeBlock {
public:
    DoradeBlockCsfd();
    ~DoradeBlockCsfd();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockCsfd * castToDoradeBlockCsfd();

private:

    static std::string id_;
    static int         length_;
    static int         maxSegments_;

};


#endif // DORADEBLOCKCSFD_H
