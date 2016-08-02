//
//
//
//

#include <stdio.h>

#include "DoradeBlockCfac.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockCfac::id_("CFAC");
int     DoradeBlockCfac::length_(72);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCfac::DoradeBlockCfac(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCfac::~DoradeBlockCfac(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockCfac::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockCfac::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockCfac::write_size() throw(Fault){

    return length_;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockCfac::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]         = buffer.get_string_from_char (  0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer(  4);
	
    }catch(Fault re){
	re.add_msg("DoradeBlockCfac::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockCfac::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockCfac::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,length_);
	int offset = 8;  // offset of first correction value
	// fill rest of CORRECTION structure with 0, the default correction
	for (int i = 0; i <16; ++i,offset+=4) {
	    buffer.set_four_byte_integer(offset, 0);
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockCfac::encode : caught Fault \n");
	throw re;
    }


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockCfac::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}



