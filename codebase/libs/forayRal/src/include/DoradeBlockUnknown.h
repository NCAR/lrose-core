//
//
//
//
//

#ifndef DORADEBLOCKUNKNOWN_H
#define DORADEBLOCKUNKNOWN_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"


class DoradeBlockUnknown : public DoradeBlock {
public:
    DoradeBlockUnknown();
    ~DoradeBlockUnknown();
    
    bool        decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void        encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string listEntry();

private:
    ForayUtility::Buffer blockData_;

};

#endif  // DORADEBLOCKUNKNOWN_H
