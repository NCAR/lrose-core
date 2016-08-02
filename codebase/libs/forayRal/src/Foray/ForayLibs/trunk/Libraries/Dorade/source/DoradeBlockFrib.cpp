//
//
//
//

#include <stdio.h>

#include "DoradeBlockFrib.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockFrib::id_("FRIB");
int     DoradeBlockFrib::length_(264);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockFrib::DoradeBlockFrib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockFrib::~DoradeBlockFrib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockFrib::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockFrib::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockFrib::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_["id"]          = buffer.get_string_from_char ( 0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer( 4);
    }catch(Fault re){
	re.add_msg("DoradeBlockFrib::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockFrib::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockFrib::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);

    }catch(Fault &re){
	re.add_msg("DoradeBlockFrib::encode : caught Fault \n");
	throw re;
    }


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockFrib::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}



