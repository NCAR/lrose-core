//
//
//
//

#ifndef DORADEBLOCKRKTB_H
#define DORADEBLOCKRKTB_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"

class DoradeBlockRktb : public DoradeBlock {
public:
    DoradeBlockRktb();
    ~DoradeBlockRktb();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size(const int numberOfRays)   throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    
    virtual DoradeBlockRktb *castToDoradeBlockRktb();

private:

    void         validate()                                    throw(ForayUtility::Fault);
    void         calculate_index_queue()                       throw(ForayUtility::Fault);

    static int    calculate_block_size(const int numberOfRays) throw(ForayUtility::Fault);

    static std::string id_;
    static int         length_;

};


#endif // DORADEBLOCKRKTB_H
