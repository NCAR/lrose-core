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

#include "DoradeBlockCelv.h"
using namespace std;
using namespace ForayUtility;

// Static values
string  DoradeBlockCelv::id_("CELV");


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCelv::DoradeBlockCelv(){

    blockSize_ = 12;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCelv::~DoradeBlockCelv(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockCelv::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockCelv::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockCelv::write_size(int numberOfCells) throw(Fault){

    return 12 + (numberOfCells * 4);
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockCelv::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]                   = buffer.get_string_from_char ( 0,4);
	blockSize_                             = buffer.get_four_byte_integer( 4);
	integerValues_["block_size"]           = blockSize_;

	int numberCells                        = buffer.get_four_byte_integer( 8);
	integerValues_["number_of_cells"]      = numberCells;
	doubleValues_ ["meters_to_first_cell"] = buffer.get_four_byte_float  (12);  // first value.

	for(int aa = 0; aa < numberCells; aa++){
	    int loc = 12 + (aa * 4);
	    doubleVector_.push_back(buffer.get_four_byte_float(loc));
	}

	validate();
	
    }catch(Fault re){
	re.add_msg("DoradeBlockCelv::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockCelv::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockCelv::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {

	validate();


	int numberCells =  get_integer("number_of_cells");
	int blockSize   = (numberCells * 4) + 12;

	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize_);
	buffer.set_four_byte_integer(  8,numberCells);

	
	doubleIterator_ = doubleVector_.begin();

	for(int cell = 0; cell < numberCells; cell++){
	    int loc = 12 + (cell * 4);
	    buffer.set_four_byte_float(loc,*doubleIterator_);
	    doubleIterator_++;
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockCelv::encode : caught Fault \n");
	throw re;
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockCelv::listEntry(){

    string returnString("");
    char   lineChar[4096];

    sprintf(lineChar,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);
    returnString += lineChar;

    sprintf(lineChar,"\tnumber_of_cells: %d\n",
	    get_integer("number_of_cells"));
    returnString += lineChar;

    sprintf(lineChar,"\n");
    returnString += string(lineChar);

    return returnString;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCelv *DoradeBlockCelv::castToDoradeBlockCelv(){
    return this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockCelv::validate() throw (Fault){

    validate_integer("DoradeBlockCelv","number_of_cells");

    int numberCells = get_integer("number_of_cells");

    if(doubleVector_.size() != get_integer("number_of_cells")){
	char msg[2048];
	sprintf(msg,"DoradeBlockCelv::validate : number_of_cells value of %d is not the same as doubleVector size of %d \n",
		numberCells,
		(int) doubleVector_.size());
	throw Fault(msg);

    }
}
