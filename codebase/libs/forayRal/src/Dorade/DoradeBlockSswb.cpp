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

#include "DoradeBlockSswb.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockSswb::id_("SSWB");
int     DoradeBlockSswb::lengthA_(200);
int     DoradeBlockSswb::lengthB_(196);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSswb::DoradeBlockSswb(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSswb::~DoradeBlockSswb(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockSswb::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	int length = buffer.get_four_byte_integer(4);

	if((length != lengthA_) && (length != lengthB_)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockSswb::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockSswb::write_size() throw(Fault){

    return lengthB_;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockSswb::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]                 = buffer.get_string_from_char ( 0,4);
	// we can accept blocks of lengthA_ or lengthB_, but write them out as lengthB_
	integerValues_["block_size"]         = lengthB_ ; // buffer.get_four_byte_integer( 4);
	integerValues_["start_time"]         = buffer.get_four_byte_integer(12);
	integerValues_["stop_time"]          = buffer.get_four_byte_integer(16);
	integerValues_["size_of_file"]       = buffer.get_four_byte_integer(20);
	integerValues_["comprssion_flag"]    = buffer.get_four_byte_integer(24);
	integerValues_["number_of_fields"]   = buffer.get_four_byte_integer(32);
	stringValues_ ["radar_name"]         = buffer.get_string_from_char (36,8);
	doubleValues_ ["start_time"]         = buffer.get_eight_byte_float (44);
	doubleValues_ ["stop_time"]          = buffer.get_eight_byte_float (52);
	integerValues_["version_num"]        = buffer.get_four_byte_integer(60);
	integerValues_["num_key_tables"]     = buffer.get_four_byte_integer(64);

	// Ignor num_keys and decode all keys.
	for(int index = 0; index < 8; index++){
	    int keyLocation = 100 + (index * 12);
	    set_integer("key_offset",index, buffer.get_four_byte_integer(keyLocation    ));
	    set_integer("key_size"  ,index, buffer.get_four_byte_integer(keyLocation + 4));
	    set_integer("key_type"  ,index, buffer.get_four_byte_integer(keyLocation + 8));
	}

    }catch(Fault re){
	re.add_msg("DoradeBlockSswb::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockSswb::decode: caught exception \n");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockSswb::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	validate();

	int blockSize = lengthB_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_four_byte_integer( 12,(int)get_double ("start_time"));
	buffer.set_four_byte_integer( 16,(int)get_double ("stop_time"));
	buffer.set_four_byte_integer( 20,     get_integer("size_of_file"));
	buffer.set_four_byte_integer( 24,     get_integer("compression_flag"));
	buffer.set_four_byte_integer( 32,     get_integer("number_of_fields"));
	buffer.set_string           ( 36,     get_string ("radar_name"),8);
	buffer.set_eight_byte_float ( 44,     get_double ("start_time"));
	buffer.set_eight_byte_float ( 52,     get_double ("stop_time"));
	buffer.set_four_byte_integer( 60,1);  // Version number
	buffer.set_four_byte_integer( 64,     get_integer("num_key_tables"));  
	
	int numKeyTables = get_integer("num_key_tables");
	for(int index = 0; index < numKeyTables; index++){
	    int keyLocation = 100 + (index * 12);
	    buffer.set_four_byte_integer(keyLocation     ,get_integer("key_offset",index));
	    buffer.set_four_byte_integer(keyLocation + 4 ,get_integer("key_size"  ,index));
	    buffer.set_four_byte_integer(keyLocation + 8 ,get_integer("key_type"  ,index));
	}


    }catch(Fault &re){
	re.add_msg("DoradeBlockSswb::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockSswb::listEntry(){

    string returnString("");
    char lineChar[1024];

    sprintf(lineChar,"%4s %5d \n", stringValues_["id"].c_str(),integerValues_["block_size"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tSize of File: %d\n",integerValues_["size_of_file"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tNumber of Fields: %d\n",integerValues_["number_of_fields"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tradar name: %s\n",get_string("radar_name").c_str());
    returnString += string(lineChar);

    sprintf(lineChar,"\tNumber of Key Tables: %d\n",integerValues_["num_key_tables"]);
    returnString += string(lineChar);

    int numberKeyTables = integerValues_["num_key_tables"];
    for(int index = 0; index < numberKeyTables; index++){
	sprintf(lineChar,"\n\tkey_offset: %d\n",get_integer("key_offset",index));
	returnString += string(lineChar);

	sprintf(lineChar,"\tkey_size: %d\n",get_integer("key_size",index));
	returnString += string(lineChar);

	sprintf(lineChar,"\tkey_type: %d\n",get_integer("key_type",index));
	returnString += string(lineChar);

    }

    sprintf(lineChar,"\n");
    returnString += string(lineChar);

    return returnString;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockSswb::validate() throw(Fault){

    validate_double ("DoradeBlockSswb","start_time");
    validate_double ("DoradeBlockSswb","stop_time");
    validate_integer("DoradeBlockSswb","size_of_file");
    validate_integer("DoradeBlockSswb","number_of_fields");
    validate_string ("DoradeBlockSswb","radar_name");

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSswb * DoradeBlockSswb::castToDoradeBlockSswb(){
    return this;
}



