//
//
//
//

#include <stdio.h>

#include "DoradeBlockNull.h"
using namespace std;
using namespace ForayUtility;

// Static values
string  DoradeBlockNull::id_("NULL");
int     DoradeBlockNull::length_(8);



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockNull::DoradeBlockNull(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockNull::~DoradeBlockNull(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockNull::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockNull::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockNull::write_size() throw(Fault){

    return length_;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockNull::decode(Buffer &buffer) throw(Fault){

    try{
	stringValues_["id"]          = buffer.get_string_from_char(0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer(4);

    }catch(Fault re){
	re.add_msg("DoradeBlockNull::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockNull::decode: caught exception \n");
    }


    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockNull::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);

    }catch(Fault &re){
	re.add_msg("DoradeBlockNull::encode : caught Fault \n");
	throw re;
    }


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockNull::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}

