//
//
//
//

#include <stdio.h>

#include "DoradeBlockSitu.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockSitu::id_("SITU");
int     DoradeBlockSitu::length_(4108);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSitu::DoradeBlockSitu(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSitu::~DoradeBlockSitu(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockSitu::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockSitu::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockSitu::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_["id"]         = buffer.get_string_from_char(0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer(4);
    }catch(Fault re){
	re.add_msg("DoradeBlockSitu::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockSitu::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockSitu::encode(Buffer &buffer) throw(Fault){

    // not coded yet

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockSitu::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}



