//
//
//
//

#include <stdio.h>

#include "DoradeBlockWave.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockWave::id_("WAVE");
int     DoradeBlockWave::length_(364);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockWave::DoradeBlockWave(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockWave::~DoradeBlockWave(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockWave::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockWave::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockWave::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_["id"]          = buffer.get_string_from_char(0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer(4);
    }catch(Fault re){
	re.add_msg("DoradeBlockWave::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockWave::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockWave::encode(Buffer &buffer) throw(Fault){

    // not coded yet

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockWave::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}



