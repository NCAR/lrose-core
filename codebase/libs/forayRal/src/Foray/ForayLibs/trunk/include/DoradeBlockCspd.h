//
//
//
//

#ifndef DORADEBLOCKCSPD_H
#define DORADEBLOCKCSPD_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"
 
class DoradeBlockCspd : public DoradeBlock {
public:
    DoradeBlockCspd();
    ~DoradeBlockCspd();
    
    static bool  test(ForayUtility::Buffer &buffer  ) throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKCSPD_H
