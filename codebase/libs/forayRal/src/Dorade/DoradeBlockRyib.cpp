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

#include "Dorade.h"
#include "DoradeBlockRyib.h"
using namespace std;
using namespace ForayUtility;


// Static values
const string  DoradeBlockRyib::id_("RYIB");
const int     DoradeBlockRyib::length_(44);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRyib::DoradeBlockRyib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRyib::~DoradeBlockRyib(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRyib::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockRyib::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockRyib::write_size() throw(Fault){

    return length_;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRyib::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]            = buffer.get_string_from_char ( 0,4);
	integerValues_["block_size"]    = buffer.get_four_byte_integer( 4);
	integerValues_["sweep_num"]     = buffer.get_four_byte_integer( 8);
	integerValues_["julian_day"]    = buffer.get_four_byte_integer(12);
	integerValues_["hour"]          = buffer.get_two_byte_integer (16);
	integerValues_["minute"]        = buffer.get_two_byte_integer (18);
	integerValues_["second"]        = buffer.get_two_byte_integer (20);
	integerValues_["millisecond"]   = buffer.get_two_byte_integer (22);
	doubleValues_ ["azimuth"]       = buffer.get_four_byte_float  (24);
	doubleValues_ ["elevation"]     = buffer.get_four_byte_float  (28);
	doubleValues_ ["peak_power"]    = buffer.get_four_byte_float  (32);
	doubleValues_ ["true_scan_rate"]= buffer.get_four_byte_float  (36);
	integerValues_["ray_status"]    = buffer.get_four_byte_integer(40);

	validate();

    }catch(Fault re){
	re.add_msg("DoradeBlockRyib::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockRyib::decode: caught exception \n");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRyib::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	validate();

	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_four_byte_integer(  8,get_integer("sweep_num"));
	buffer.set_four_byte_integer( 12,get_integer("julian_day"));
	buffer.set_two_byte_integer ( 16,get_integer("hour"));
	buffer.set_two_byte_integer ( 18,get_integer("minute"));
	buffer.set_two_byte_integer ( 20,get_integer("second"));
	buffer.set_two_byte_integer ( 22,get_integer("millisecond"));
	buffer.set_four_byte_float  ( 24,get_double ("azimuth"));
	buffer.set_four_byte_float  ( 28,get_double ("elevation"));
	buffer.set_four_byte_float  ( 32,get_double ("peak_power"));
	buffer.set_four_byte_float  ( 36,get_double ("true_scan_rate"));
	buffer.set_four_byte_integer( 40,get_integer("ray_status"));
	
    }catch(Fault &re){
	re.add_msg("DoradeBlockRyib::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockRyib::listEntry(){

    string returnString("");
    char lineChar[1024];

    sprintf(lineChar,"%4s %5d \n", stringValues_["id"].c_str(),integerValues_["block_size"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\televation: \t%6.2f\n",get_double("elevation"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tazimuth: \t%6.2f\n",get_double("azimuth"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tray status: \t%d (%s)\n",
	    get_integer("ray_status"),
	    Dorade::rayStatus[get_integer("ray_status")]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tjulian day: \t%d\n",get_integer("julian_day"));
    returnString += string(lineChar);

    sprintf(lineChar,"\thour: \t\t%d\n",get_integer("hour"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tminute: \t%d\n",get_integer("minute"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tsecond: \t%d\n",get_integer("second"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tmillisecond: \t%d\n",get_integer("millisecond"));
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
DoradeBlockRyib * DoradeBlockRyib::castToDoradeBlockRyib(){
    return this;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRyib::validate() throw (Fault){

    validate_integer("DoradeBlockRyib","sweep_num");
    validate_integer("DoradeBlockRyib","julian_day");
    validate_integer("DoradeBlockRyib","hour");
    validate_integer("DoradeBlockRyib","minute");
    validate_integer("DoradeBlockRyib","second");
    validate_integer("DoradeBlockRyib","millisecond");
    validate_double ("DoradeBlockRyib","azimuth");
    validate_double ("DoradeBlockRyib","elevation");
    validate_double ("DoradeBlockRyib","peak_power");
    validate_double ("DoradeBlockRyib","true_scan_rate");
    validate_integer("DoradeBlockRyib","ray_status");

    int rayStatus = get_integer("ray_status");
    if((rayStatus < Dorade::rayStatusNormal) || (rayStatus > Dorade::rayStatusBad)){
	char msg[2048];
	sprintf(msg,"DoradeBlockRyib::validate : ray status value of %d is invalid.\n",
		rayStatus);
	throw Fault(msg);
    }
}
