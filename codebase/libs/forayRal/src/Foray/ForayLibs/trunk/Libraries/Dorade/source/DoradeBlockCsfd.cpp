//
//
//
//

#include <stdio.h>

#include "DoradeBlockCsfd.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockCsfd::id_("CSFD");
int     DoradeBlockCsfd::length_(64);
int     DoradeBlockCsfd::maxSegments_(8);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCsfd::DoradeBlockCsfd(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockCsfd::~DoradeBlockCsfd(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockCsfd::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockCsfd::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockCsfd::write_size() throw(Fault){

    return length_;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockCsfd::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]                      = buffer.get_string_from_char ( 0,4);
	integerValues_["block_size"]              = buffer.get_four_byte_integer( 4);
	int num_segments                          = buffer.get_four_byte_integer( 8);
	integerValues_["number_of_cell_segments"] = num_segments;
	doubleValues_ ["meters_to_first_cell"]    = buffer.get_four_byte_float  (12);

	char   key[48];
	int    loc;	
	int    number_cells(0);
	int    number_segment_cells;
	double spacing(0.0);
	double gate_range = doubleValues_["meters_to_first_cell"];

	for(int aa = 0; aa < num_segments; aa++){
	    loc = 16 + (aa * 4);
	    spacing  = buffer.get_four_byte_float(loc);
	    set_double("spacing",aa,spacing);


	    loc = 48 + (aa * 2);
	    number_segment_cells = buffer.get_two_byte_integer(loc);
	    set_integer("segment_cell_count",aa,number_segment_cells);

	    number_cells +=   number_segment_cells;

	    for(int bb = 0; bb < number_segment_cells; bb++){
		doubleVector_.push_back(gate_range);
		gate_range += spacing;
	    }
	}

	validate();

	integerValues_["number_of_cells"] = number_cells;

    }catch(Fault re){
	re.add_msg("DoradeBlockCsfd::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockCsfd::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockCsfd::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	validate();

	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_four_byte_integer(  8,get_integer("number_of_cell_segments"));
	buffer.set_four_byte_float  ( 12,get_double ("meters_to_first_cell"));

	// Set all values to zero
	for(int aa = 0; aa < maxSegments_; aa++){
	    int loc1 = 16 + (aa * 4);
	    buffer.set_four_byte_float(loc1,0.0);

	    int loc2 = 48 + (aa * 2);
	    buffer.set_two_byte_integer(loc2,0);
	}

	int numberSegments = get_integer("number_of_cell_segments");
	for(int aa = 0; aa < numberSegments; aa++){
	    int loc1 = 16 + (aa * 4);
	    buffer.set_four_byte_float(loc1,get_double("spacing",aa));

	    int loc2 = 48 + (aa * 2);
	    buffer.set_two_byte_integer(loc2,get_integer("segment_cell_count",aa));
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockCsfd::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockCsfd::listEntry(){

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
    
    int numberSegments = get_integer("number_of_cell_segments");
    sprintf(lineChar,"\tnumber_of_cell_segments: %d\n",
	    numberSegments);
    returnString += lineChar;

    for(int segment = 0; segment < numberSegments; segment++){
	sprintf(lineChar,"\t\tSegment %d: cell count: %d\n",segment,get_integer("segment_cell_count",segment));
	returnString += lineChar;
	
	sprintf(lineChar,"\t\tSegment %d: cell spacing %6.2f\n",segment,get_double("spacing",segment));
	returnString += lineChar;

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
DoradeBlockCsfd * DoradeBlockCsfd::castToDoradeBlockCsfd(){
    return this;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockCsfd::validate() throw(Fault){

    validate_integer("DoradeBlockCsfd","number_of_cell_segments");
    validate_double ("DoradeBlockCsfd","meters_to_first_cell");

    int numberSegments = get_integer("number_of_cell_segments");

    if ((numberSegments > maxSegments_) || (numberSegments < 0)){
	char msg[2048];
	sprintf(msg,"DoradeBlockCsfd::validate: num_segments value of %d is out of range \n",
		numberSegments);
	throw Fault(msg);
    }

    for(int index = 0; index < numberSegments; index++){
	validate_double ("DoradeBlockCsfd","spacing"  ,index);
	validate_integer("DoradeBlockCsfd","segment_cell_count",index);
    }

}
