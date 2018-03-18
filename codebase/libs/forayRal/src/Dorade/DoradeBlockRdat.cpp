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
#include <string.h>

#include <iostream>

#include "Dorade.h"
#include "DoradeBlockRdat.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockRdat::id_("RDAT");



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRdat::DoradeBlockRdat(){
    

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRdat::~DoradeBlockRdat(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRdat::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(buffer.current_size() != buffer.get_four_byte_integer(4)){
	    throw Fault("DoradeBlockRdat::test : Size of buffer != block length. \n");
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockRdat::test : caught Fault.\n");
	throw re;
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRdat::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]         = buffer.get_string_from_char ( 0,4);
	integerValues_["block_size"] = buffer.get_four_byte_integer( 4);
	string fieldName             = buffer.get_string_from_char (  8,8);
	int    space = fieldName.find(" ");
	if(space == string::npos ){
	    stringValues_["pdata_name"]    = fieldName;
	}else{
	    stringValues_["pdata_name"]    = fieldName.substr(0,space);
	}

    }catch(Fault re){
	re.add_msg("DoradeBlockRdat::decode : caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockRdat::decode : caught exception \n");
    }

    // RDAT is one fo the few DoradeBlock classes that 
    // copies its original buffer. This is needed
    // so that the data can be decoded latter.
    // Decoding of the data can not now be decoded because
    ///the binary format of the data is not known.
    buffer_ = buffer;

    return true;

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockRdat::listEntry(){

    char charEntry[4096];

    sprintf(charEntry,
	    "%4s %5d %s \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"],
	    stringValues_["pdata_name"].c_str());

    string entry(charEntry);

    return entry;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRdat * DoradeBlockRdat::castToDoradeBlockRdat(){
    return this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRdat::dataIsInteger(const int binaryFormat) throw (Fault){

    if((binaryFormat < 1) || (binaryFormat > 5)){
	char msg[2048];
	sprintf(msg,"DoradeBlockRdat::dataIsInteger : Invalde format_value of %d. \n",binaryFormat);
	throw Fault(msg);
    }

    if((binaryFormat == Dorade::binaryFormat1ByteInt) ||
       (binaryFormat == Dorade::binaryFormat2ByteInt) ||
       (binaryFormat == Dorade::binaryFormat3ByteInt)){

	return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRdat::decodeIntegerData(const int binaryFormat,RayIntegers * const ri) throw(Fault){

    int dataSize = integerValues_["block_size"] - 16;

    if((binaryFormat < 1) || (binaryFormat > 5)){
	char msg[2048];
	sprintf(msg,"DoradeBlockRdat::decodeIntegerData : binary_format value of %d is invalid.\n", binaryFormat);
	throw Fault(msg);
    }

    //
    // Floats not decoded here.
    //
    if ((binaryFormat == Dorade::binaryFormat2ByteFloat) ||
	(binaryFormat == Dorade::binaryFormat4ByteFloat)){
	throw Fault("DoradeBlockRdat::decodeIntegerData : binary_format defines float values.\n");
    }

    //
    // 3 Byte Integers
    //
    if (binaryFormat == Dorade::binaryFormat3ByteInt){
	throw Fault("DoradeBlockRdat::decodeIntegerData : 3 byte int decoding not implemented.\n");
    }

    ri->clear();

    //
    // 1 Byte Integers
    //
    if(binaryFormat == Dorade::binaryFormat1ByteInt){
	int numberDataValues = dataSize;
	int value;
	int loc;
	ri->reserve(numberDataValues);
	for(int count = 0; count < numberDataValues; count++){
	    loc   = 16 + count;
	    value = buffer_.get_one_byte_integer(loc);
	    ri->push_back(value);
	}
    }

    //
    // 2 Byte Integers
    //
    if(binaryFormat == Dorade::binaryFormat2ByteInt){
	int numberDataValues = dataSize/2;
	int value;
	int loc;
	ri->reserve(numberDataValues);
	for(int count = 0; count < numberDataValues; count++){
	    loc   = 16 + (2 * count);
	    value = buffer_.get_two_byte_integer(loc);
	    ri->push_back(value);
	}
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRdat::encode(Buffer &buffer) throw(Fault){

    throw Fault("DoradeBlockRdat::encode: Need to call version of encode that passes RayIntegers or RayDoubles\n");

}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRdat::encode(Buffer &buffer, const int binaryFormat,RayIntegers &ri) throw(Fault){

    if((binaryFormat < 1) || (binaryFormat > 5)){
	char msg[2048];
	sprintf(msg,"DoradeBlockRdat::encode : binary_format value of %d is invalid.\n", binaryFormat);
	throw Fault(msg);
    }

    if((binaryFormat != Dorade::binaryFormat1ByteInt) &&
       (binaryFormat != Dorade::binaryFormat2ByteInt)){
	char msg[2048];
	sprintf(msg,"DoradeBlockRdat::encode : binary_format does not specify one or two byte integer encoding. \n");
	throw Fault(msg);
    }

    int numberOfBins = ri.size();
    int blockSize;

    if(binaryFormat == Dorade::binaryFormat1ByteInt){
	// One byte integer encoding
	blockSize = numberOfBins + 16;
    }else{
	// Must be two byte integer encoding.
	blockSize = (numberOfBins * 2) + 16;
    }
    
    integerValues_["block_size"] = blockSize;

    try {

        buffer.new_data(blockSize);

	buffer.set_string           ( 0,id_,4);
	buffer.set_four_byte_integer( 4,blockSize);
	buffer.set_string           ( 8,get_string("pdata_name"),8);

	RayIntegers::iterator dataIterator = ri.begin();

	if(binaryFormat == Dorade::binaryFormat1ByteInt){
	    // One byte integers
	    for(int index = 0; index < numberOfBins; index++){
		int value =  *dataIterator;
		dataIterator++;
		int location = 16 + index;
		buffer.set_one_byte_integer(location,value);
	    }
	}else{
	    // Two byte integers
	    for(int index = 0; index < numberOfBins; index++){
		int value = *dataIterator;
		dataIterator++;
		int location = 16 + (2 * index);
		buffer.set_two_byte_integer(location,value);
	    }
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockRdat::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRdat::encode(Buffer &buffer, const int binaryFormat,RayDoubles &rd) throw(Fault){

    if((binaryFormat < 1) || (binaryFormat > 5)){
	char msg[2048];
	sprintf(msg,"DoradeBlockRdat::encode : binary_format value of %d is invalid.\n", binaryFormat);
	throw Fault(msg);
    }

    if(binaryFormat != Dorade::binaryFormat4ByteFloat){
	char msg[2048];
	sprintf(msg,"DoradeBlockRdat::encode : binary_format does not specify 4 byte float encoding. \n");
	throw Fault(msg);
    }

    int numberOfBins = rd.size();
    int blockSize;

    blockSize = (numberOfBins * 4) + 16;
    
    integerValues_["block_size"] = blockSize;

    try {

        buffer.new_data(blockSize);

	buffer.set_string           ( 0,id_,4);
	buffer.set_four_byte_integer( 4,blockSize);
	buffer.set_string           ( 8,get_string("pdata_name"),8);

	RayDoubles::iterator dataIterator = rd.begin();

	for(int index = 0; index < numberOfBins; index++){
	    double value =  *dataIterator;
	    dataIterator++;
	    int location = 16 + (4 * index);
	    buffer.set_four_byte_float(location,value);
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockRdat::encode : caught Fault \n");
	throw re;
    }
}
