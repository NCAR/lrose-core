//
//
//
//

#ifndef DORADEBLOCKRADD_H
#define DORADEBLOCKRADD_H

#include <string>

#include <vector>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockRadd : public DoradeBlock {
public:
    DoradeBlockRadd();
    ~DoradeBlockRadd();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         validate()                           throw(ForayUtility::Fault);
    std::string  listEntry();

    virtual DoradeBlockRadd * castToDoradeBlockRadd();


private:

    const static std::string id_;
    const static int         lengthA_;
    const static int         lengthB_;

};


#endif // DORADEBLOCKRADD_H
