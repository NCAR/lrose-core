//
//
//
//

#include <stdio.h>
#include "Dorade.h"
#include "DoradeBlockParm.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockParm::id_("PARM");
int     DoradeBlockParm::lengthA_(216);
int     DoradeBlockParm::lengthB_(104);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockParm::DoradeBlockParm(){

    integerValues_["block_size"] = lengthA_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockParm::~DoradeBlockParm(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockParm::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if((lengthA_ != buffer.get_four_byte_integer(4)) &&
	   (lengthB_ != buffer.get_four_byte_integer(4))){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockParm::test : caught Fault.\n");
	throw re;
    }
    
    return true;
}
//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockParm::write_size() throw(Fault){

    return lengthA_;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockParm::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}

	stringValues_     ["id"]                   = buffer.get_string_from_char (  0,4);
	int blockSize                              = buffer.get_four_byte_integer(  4);
	integerValues_    ["block_size"]           = blockSize;
	string fieldName                           = buffer.get_string_from_char (  8,8);
	int    space = fieldName.find(" ");
	if(space == string::npos ){
	    stringValues_ ["field_name"]           = fieldName;
	}else{
	    stringValues_ ["field_name"]           = fieldName.substr(0,space);
	}
	stringValues_     ["param_description"]    = buffer.get_string_from_char ( 16,40);
	stringValues_     ["param_units"]          = buffer.get_string_from_char ( 56,8);
	integerValues_    ["pulse_width_meters"]   = buffer.get_two_byte_integer ( 72);
	integerValues_    ["polarization"]         = buffer.get_two_byte_integer ( 74);
	integerValues_    ["number_of_samples"]    = buffer.get_two_byte_integer ( 76);
	integerValues_    ["binary_format"]        = buffer.get_two_byte_integer ( 78);
	doubleValues_     ["parameter_scale"]      = buffer.get_four_byte_float  ( 92);
	doubleValues_     ["parameter_bias"]       = buffer.get_four_byte_float  ( 96);
	integerValues_    ["bad_data"]             = buffer.get_four_byte_integer(100);
	if(blockSize == lengthA_){
	    integerValues_["offset_to_data"]       = buffer.get_four_byte_integer(120);
	    integerValues_["number_of_cells"]      = buffer.get_four_byte_integer(200);
	    doubleValues_ ["meters_to_first_cell"] = buffer.get_four_byte_float  (204);
	    doubleValues_ ["meters_between_cells"] = buffer.get_four_byte_float  (208);
	}

	validate();
	
	
    }catch(Fault &re){
	re.add_msg("DoradeBlockParm::decode : caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockParm::decode : caught exception \n");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockParm::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	
	integerValues_["block_size"] = lengthA_;

	validate();

	int blockSize = lengthA_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_string           (  8,get_string ("field_name"),8);
	buffer.set_string           ( 16,get_string ("param_description"),40);
	buffer.set_string           ( 56,get_string ("param_units"),8);
	buffer.set_two_byte_integer ( 76,get_integer("number_of_samples"));
	buffer.set_two_byte_integer ( 78,get_integer("binary_format"));
	buffer.set_four_byte_float  ( 92,get_double ("parameter_scale"));
	buffer.set_four_byte_float  ( 96,get_double ("parameter_bias"));
	buffer.set_four_byte_integer(100,get_integer("bad_data"));

	// lengthA values
	buffer.set_four_byte_integer(120,get_integer("offset_to_data"));
	buffer.set_four_byte_integer(200,get_integer("number_of_cells"));
	buffer.set_four_byte_float  (204,get_double ("meters_to_first_cell"));
	buffer.set_four_byte_float  (208,get_double ("meters_between_cells"));

    }catch(Fault &re){
	re.add_msg("DoradeBlockParm::encode : caught Fault \n");
	throw re;
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockParm::listEntry(){

    string returnString("");
    char lineChar[1024];

    sprintf(lineChar,"%4s %5d \n", stringValues_["id"].c_str(),integerValues_["block_size"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tField Name: %s \n", get_string("field_name").c_str());
    returnString += string(lineChar);

    sprintf(lineChar,"\tPulse Width: %d (meters)\n", get_integer("pulse_width_meters"));
    returnString += string(lineChar);

    int polarizationValue = get_integer("polarization");
    sprintf(lineChar,"\tPolarization: %d (%s)\n", polarizationValue,Dorade::polarization[polarizationValue]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tNumber of Samples: %d\n",get_integer("number_of_samples"));
    returnString += string(lineChar);

    int binaryFormat = get_integer("binary_format");
    sprintf(lineChar,"\tBinary Format: %d (%s)\n",binaryFormat,Dorade::binaryFormats[binaryFormat]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tParameter Scale: %6.2f\n",get_double("parameter_scale"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tParameter Bias: %6.2f\n",get_double("parameter_bias"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tBad Data: %d\n",get_integer("bad_data"));
    returnString += string(lineChar);

    if(get_integer("block_size") == lengthA_){

	sprintf(lineChar,"\tOffset to Data: %d \n",get_integer("offset_to_data"));
	returnString += string(lineChar);

	sprintf(lineChar,"\tNumber of Cells: %d \n",get_integer("number_of_cells"));
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
DoradeBlockParm * DoradeBlockParm::castToDoradeBlockParm(){
    return this;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockParm::validate() throw (Fault){

    validate_integer("DoradeBlockParm","block_size");
    validate_string ("DoradeBlockParm","field_name");
    validate_string ("DoradeBlockParm","param_description");
    validate_string ("DoradeBlockParm","param_units");
    validate_integer("DoradeBlockParm","binary_format");
    validate_double ("DoradeBlockParm","parameter_scale");
    validate_double ("DoradeBlockParm","parameter_bias");
    validate_integer("DoradeBlockParm","bad_data");
    
    if(integerValues_["block_size"] == lengthA_){
	validate_integer("DoradeBlockParm","offset_to_data");
	validate_integer("DoradeBlockParm","number_of_cells");
	validate_double ("DoradeBlockParm","meters_to_first_cell");
	validate_double ("DoradeBlockParm","meters_between_cells");
    }

    int binaryFormat = get_integer("binary_format");
    if((binaryFormat < 1) || (binaryFormat > 5)){
	char msg[2048];
	sprintf(msg,"DoradeBlockParm::validate : binary_format value of %d is undefined.\n",binaryFormat);
	throw Fault(msg);
    }
    

}

