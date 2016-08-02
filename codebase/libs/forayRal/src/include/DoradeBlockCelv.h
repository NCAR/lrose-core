//
//
//
//

#ifndef DORADEBLOCKCELV_H
#define DORADEBLOCKCELV_H

#include <string>

#include "Fault.h"
#include "Buffer.h"

#include "DoradeBlock.h"

class DoradeBlockCelv : public DoradeBlock {
public:
    DoradeBlockCelv();
    ~DoradeBlockCelv();
    
    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size(int numberOfCells)        throw(ForayUtility::Fault);
    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();
    void         validate()                           throw(ForayUtility::Fault);

    virtual DoradeBlockCelv * castToDoradeBlockCelv();


private:

    static std::string id_;
    int blockSize_;

};


#endif // DORADEBLOCKCELV_H
