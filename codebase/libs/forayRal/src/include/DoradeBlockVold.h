//
//
//
//

#ifndef DORADEBLOCKVOLD_H
#define DORADEBLOCKVOLD_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockVold : public DoradeBlock {
public:
    DoradeBlockVold();
    ~DoradeBlockVold();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockVold * castToDoradeBlockVold();


private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKVOLD_H
