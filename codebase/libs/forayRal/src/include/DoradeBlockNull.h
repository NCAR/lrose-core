//
//
//
//
//

#ifndef DORADEBLOCKNULL_H
#define DORADEBLOCKNULL_H

#include <string>

#include "Fault.h"
#include "Buffer.h"
#include "DoradeBlock.h"


class DoradeBlockNull : public DoradeBlock {
public:
    DoradeBlockNull();
    ~DoradeBlockNull();

    static bool  test(ForayUtility::Buffer &buffer)   throw(ForayUtility::Fault);
    static int   write_size()                         throw(ForayUtility::Fault);

    bool         decode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    void         encode(ForayUtility::Buffer &buffer) throw(ForayUtility::Fault);
    std::string  listEntry();

private:
    ForayUtility::Buffer blockData_;

    static std::string id_;
    static int         length_;

};

#endif  // DORADEBLOCKNULL_H
