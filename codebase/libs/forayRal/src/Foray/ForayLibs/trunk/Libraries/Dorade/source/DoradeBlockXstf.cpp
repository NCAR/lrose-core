//
//
//
//

#include <stdio.h>

#include "DoradeBlockXstf.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockXstf::id_("XSTF");


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockXstf::DoradeBlockXstf(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockXstf::~DoradeBlockXstf(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockXstf::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(buffer.current_size() != buffer.get_four_byte_integer(4)){
	    throw Fault("DoradeBlockXstf::test: Size of buffer != block length. \n");
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockXstf::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockXstf::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_["id"]          = buffer.get_string_from_char(0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer(4);
    }catch(Fault re){
	re.add_msg("DoradeBlockXstf::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockXstf::decode: caught exception \n");
    }

    // XSTF is one the few classes that copies its original
    // buffer.  
    buffer_ = buffer;
    contentPointer_ = buffer_.data(24);

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockXstf::encode(Buffer &buffer) throw(Fault){

    throw Fault("DoradeBlockXstf::encode: Encode method not implemented.");
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockXstf::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
const unsigned char * DoradeBlockXstf::content(){

    return contentPointer_;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockXstf * DoradeBlockXstf::castToDoradeBlockXstf(){
    return this;
}
