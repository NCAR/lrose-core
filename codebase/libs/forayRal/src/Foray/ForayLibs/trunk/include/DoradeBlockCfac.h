//
//
//
//

#ifndef DORADEBLOCKCFAC_H
#define DORADEBLOCKCFAC_H

#include <string>

#include "Fault.h"
#include "Buffer.h"

#include "DoradeBlock.h"

class DoradeBlockCfac : public DoradeBlock {
public:
    DoradeBlockCfac();
    ~DoradeBlockCfac();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()			      throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKCFAC_H
