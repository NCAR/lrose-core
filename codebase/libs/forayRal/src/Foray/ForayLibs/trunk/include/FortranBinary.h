//
//
// $Id: FortranBinary.h,v 1.1 2008/10/23 05:06:20 dixon Exp $
//

#ifndef FORTRANBINARY_H
#define FORTRANBINARY_H

#include <stdio.h>

#include "Fault.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Buffer.h"

namespace ForayUtility {

class FortranBinary {
public:
    FortranBinary();
    ~FortranBinary();

    void open_file   (const char *filename)   throw(ForayUtility::Fault);
    void create_file (const char *filename)   throw(ForayUtility::Fault);
    void close_file  ();
    bool read_record (ForayUtility::Buffer &) throw(ForayUtility::Fault);
    void write_record(ForayUtility::Buffer &) throw(ForayUtility::Fault);

    bool opened_for_writing();

private:
    FILE    *file_;
    bool    read_only_;
    ForayUtility::Decoder *decoderHead_;
    ForayUtility::Encoder *encoderHead_;

};
}



#endif  // FORTRANBINARY_H
