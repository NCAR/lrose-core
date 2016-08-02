//
//
//
//
//
//

#ifndef DORADEFB_H
#define DORADEFB_H

#include <stdio.h>

#include "Fault.h"
#include "Buffer.h"
#include "Decoder.h"
#include "FortranBinary.h"
#include "DoradeBlock.h"

class DoradeFortranBinary : public ForayUtility::FortranBinary {
public:
    DoradeFortranBinary();
    ~DoradeFortranBinary();

    DoradeBlock *read_next_block() throw(ForayUtility::Fault);

private:

    bool read_buffer(ForayUtility::Buffer &) throw(ForayUtility::Fault);

    ForayUtility::Buffer currentRecord_;
    int    recordBytesUsed_;


};

#endif // DORADEFB_H
