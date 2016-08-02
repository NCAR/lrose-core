//
//
//
//

#ifndef DORADEBLOCKSSWB_H
#define DORADEBLOCKSSWB_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockSswb : public DoradeBlock {
public:
    DoradeBlockSswb();
    ~DoradeBlockSswb();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockSswb *castToDoradeBlockSswb();

private:

    static std::string id_;
    static int         lengthA_;
    static int         lengthB_;

};


#endif // DORADEBLOCKSSWB_H
