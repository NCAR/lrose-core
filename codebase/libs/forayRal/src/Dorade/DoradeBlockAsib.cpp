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

#include <stdio.h>

#include "DoradeBlockAsib.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockAsib::id_("ASIB");
int     DoradeBlockAsib::length_(80);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockAsib::DoradeBlockAsib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockAsib::~DoradeBlockAsib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockAsib::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockAsib::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockAsib::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_["id"]          = buffer.get_string_from_char (  0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer(  4);
    }catch(Fault &re){
	re.add_msg("DoradeBlockAsib::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockAsib::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockAsib::encode(Buffer &buffer) throw(Fault){

  // unsigned char *bufferData;

    try {
	int blockSize = length_;
	/* bufferData = */ buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,length_);
	
    }catch(Fault &re){
	re.add_msg("DoradeBlockAsib::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockAsib::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    string entry(charEntry);

    return entry;
}



