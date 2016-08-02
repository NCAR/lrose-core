// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
//
//
//
//

#include <errno.h>
extern int errno;
#include <string.h>

#include <iostream>
using namespace std;

#include "DoradeBlock.h"
#include "DoradeBlockVold.h"
#include "DoradeBlockWave.h"
#include "DoradeBlockRadd.h"
#include "DoradeBlockFrib.h"
#include "DoradeBlockCspd.h"
#include "DoradeBlockParm.h"
#include "DoradeBlockNdds.h"
#include "DoradeBlockSitu.h"
#include "DoradeBlockRyib.h"
#include "DoradeBlockAsib.h"
#include "DoradeBlockFrad.h"
#include "DoradeBlockIndf.h"
#include "DoradeBlockTime.h"
#include "DoradeBlockUnknown.h"


#include "DoradeFortranBinary.h"
using namespace ForayUtility;


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeFortranBinary::DoradeFortranBinary() {

    recordBytesUsed_ = -1;
    currentRecord_.is_big_endian(true);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeFortranBinary::~DoradeFortranBinary(){

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeFortranBinary::read_buffer(Buffer &buffer) throw(Fault){

    char msg[2048];

    buffer.is_big_endian(true);

    if((recordBytesUsed_ < 0) || 
       (recordBytesUsed_ >= currentRecord_.current_size())){
	
	try {

	    if(!read_record(currentRecord_)){
		return false;
	    }

	    recordBytesUsed_ = 0;
	}catch(Fault &re){
	    sprintf(msg,"DoradeFortranBinary::read_buffer: Caught Fault.\n");
	    re.add_msg(msg);
	    throw re;
	}catch(...){
	    sprintf(msg,"DoradeFortranBinary::read_buffer: Caught default exception.\n");
	    throw Fault(msg);
	}
    }

    try {

	string id = currentRecord_.get_string_from_char(recordBytesUsed_,4);

	int bufferSize = currentRecord_.get_four_byte_integer(recordBytesUsed_ + 4);

	if(bufferSize == 0){
	    //Assume that rest of record is a Dorade Block.
	    bufferSize = currentRecord_.current_size() - recordBytesUsed_;
	}

	if(bufferSize > 102400){
	    throw Fault("DoradeFortranBinary::read_buffer: bufferSize too big (correct byte endian ?)\n");
	}

	const unsigned char *recordData = currentRecord_.data(recordBytesUsed_);
	unsigned char *bufferData  = buffer.new_data(bufferSize);
	memcpy(bufferData,recordData,bufferSize);
	recordBytesUsed_ += bufferSize;

    }catch(Fault &re){
	sprintf(msg,"DoradeFortranBinary::read_buffer: caught Fault \n");
	re.add_msg(msg);
	throw re;
    }catch(...){
	sprintf(msg,"DoradeFortranBinary::read_buffer: caught default exception.\n");
	throw Fault(msg);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlock *DoradeFortranBinary::read_next_block() throw(Fault){

    Buffer buffer;
    DoradeBlock *returnBlock(NULL);

    try {

	if(read_buffer(buffer)){

	    if(DoradeBlockVold::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockVold();
	    }else if(DoradeBlockWave::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockWave();
	    }else if(DoradeBlockRadd::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockRadd();
	    }else if(DoradeBlockFrib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockFrib();
	    }else if(DoradeBlockCspd::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockCspd();
	    }else if(DoradeBlockParm::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockParm();
	    }else if(DoradeBlockNdds::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockNdds();
	    }else if(DoradeBlockSitu::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockSitu();
	    }else if(DoradeBlockRyib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockRyib();
	    }else if(DoradeBlockAsib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockAsib();
	    }else if(DoradeBlockFrad::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockFrad();
	    }else if(DoradeBlockIndf::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockIndf();
	    }else if(DoradeBlockTime::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockTime();
	    }else{
		returnBlock = (DoradeBlock *) new DoradeBlockUnknown();
	    }

	    returnBlock->decode(buffer);
	}

    }catch(Fault &re){
	re.add_msg("DoradeFortranBinary::read_next_block: caught Fault\n");
	throw re;
    }catch(...){
	throw Fault("DoradeFortranBinary::read_next_block: caught exception\n" );
    }


    return returnBlock;
}


