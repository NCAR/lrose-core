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

#include "DoradeBlockSwib.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockSwib::id_("SWIB");
int     DoradeBlockSwib::length_(40);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSwib::DoradeBlockSwib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSwib::~DoradeBlockSwib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockSwib::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockSwib::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockSwib::write_size() throw(Fault){

    return length_;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockSwib::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]          = buffer.get_string_from_char  ( 0,4);
	integerValues_["block_size"]  = buffer.get_four_byte_integer ( 4);
	stringValues_ ["radar_name"]  = buffer.get_string_from_char  ( 8,8);
	integerValues_["sweep_num"]   = buffer.get_four_byte_integer (16);
	integerValues_["num_rays"]    = buffer.get_four_byte_integer (20);
	doubleValues_ ["start_angle"] = buffer.get_four_byte_float   (24);
	doubleValues_ ["stop_angle"]  = buffer.get_four_byte_float   (28);
	doubleValues_ ["fixed_angle"] = buffer.get_four_byte_float   (32);
	integerValues_["filter_flag"] = buffer.get_four_byte_integer (36);

    }catch(Fault re){
	re.add_msg("DoradeBlockSwib::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockSwib::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockSwib::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {

	validate();

	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_string           (  8,get_string ("radar_name"),8);
	buffer.set_four_byte_integer( 16,get_integer("sweep_num"));
	buffer.set_four_byte_integer( 20,get_integer("num_rays"));
	buffer.set_four_byte_float  ( 24,get_double ("start_angle"));
	buffer.set_four_byte_float  ( 28,get_double ("stop_angle"));
	buffer.set_four_byte_float  ( 32,get_double ("fixed_angle"));
	buffer.set_four_byte_integer( 36,get_integer("filter_flag"));

    }catch(Fault &re){
	re.add_msg("DoradeBlockSwib::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockSwib::listEntry(){

    string returnString("");
    char lineChar[1024];

    sprintf(lineChar,"%4s %5d \n", stringValues_["id"].c_str(),integerValues_["block_size"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tNumber of Rays: %d\n",integerValues_["num_rays"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tSweep Number: %d\n",integerValues_["sweep_num"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tFixed Angle: %6.2f\n",doubleValues_["fixed_angle"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tStart Angle: %6.2f\n",doubleValues_["start_angle"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tStop Angle: %6.2f\n",doubleValues_["stop_angle"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\n");
    returnString += string(lineChar);

    return returnString;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockSwib::validate() throw(Fault){

    validate_string ("DoradeBlockSwib","radar_name");
    validate_integer("DoradeBlockSwib","sweep_num");
    validate_integer("DoradeBlockSwib","num_rays");
    validate_double ("DoradeBlockSwib","start_angle");
    validate_double ("DoradeBlockSwib","stop_angle");
    validate_double ("DoradeBlockSwib","fixed_angle");
    validate_integer("DoradeBlockSwib","filter_flag");

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockSwib * DoradeBlockSwib::castToDoradeBlockSwib() { 
    return this;
}


