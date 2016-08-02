//
//
//
//

#include <stdio.h>

#include "DoradeBlockVold.h"
using namespace std;
using namespace ForayUtility;

// Static values
string  DoradeBlockVold::id_("VOLD");
int     DoradeBlockVold::length_(72);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockVold::DoradeBlockVold(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockVold::~DoradeBlockVold(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockVold::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if(length_ != buffer.get_four_byte_integer(4)){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockVold::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockVold::write_size() throw(Fault){

    return length_;
}




//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockVold::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]                = buffer.get_string_from_char (0,4);
	integerValues_["block_size"]        = buffer.get_four_byte_integer(4);
	integerValues_["volume_number"]     = buffer.get_two_byte_integer (10);
	integerValues_["maximum_bytes"]     = buffer.get_four_byte_integer(12);
	stringValues_ ["project_name"]      = buffer.get_string_from_char (16,20);
	integerValues_["year"]              = buffer.get_two_byte_integer (36);
	integerValues_["month"]             = buffer.get_two_byte_integer (38);
	integerValues_["day"]               = buffer.get_two_byte_integer (40);
	integerValues_["data_set_hour"]     = buffer.get_two_byte_integer (42);
	integerValues_["data_set_minute"]   = buffer.get_two_byte_integer (44);
	integerValues_["data_set_second"]   = buffer.get_two_byte_integer (46);
	integerValues_["number_sensor_des"] = buffer.get_two_byte_integer (70);
	
    }catch(Fault re){
	re.add_msg("DoradeBlockVold::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockVold::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockVold::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {

	validate();

	int blockSize = length_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_two_byte_integer ( 10,get_integer("volume_number"));
	buffer.set_four_byte_integer( 12,65500);
	buffer.set_string           ( 16,get_string ("project_name"),20);
	buffer.set_two_byte_integer ( 36,get_integer("year"));
	buffer.set_two_byte_integer ( 38,get_integer("month"));
	buffer.set_two_byte_integer ( 40,get_integer("day"));
	buffer.set_two_byte_integer ( 42,get_integer("data_set_hour"));
	buffer.set_two_byte_integer ( 44,get_integer("data_set_minute"));
	buffer.set_two_byte_integer ( 46,get_integer("data_set_second"));
	buffer.set_two_byte_integer ( 70,get_integer("number_sensor_des"));

    }catch(Fault &re){
	re.add_msg("DoradeBlockVold::encode : caught Fault \n");
	throw re;
    }


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockVold::listEntry(){

    string returnString("");
    char   lineChar[4096];

    sprintf(lineChar,
	    "%4s %5d \n",
	    stringValues_["id"].c_str(),
	    integerValues_["block_size"]);

    returnString += string(lineChar);

    sprintf(lineChar,"\tVolume Number: %d\n",get_integer("volume_number"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tProject Name: %s\n",get_string("project_name").c_str());
    returnString += string(lineChar);

    sprintf(lineChar,"\tyear: %d\n",get_integer("year"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tmonth: %d\n",get_integer("month"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tday: %d\n",get_integer("day"));
    returnString += string(lineChar);

    sprintf(lineChar,"\thour: %d\n",get_integer("data_set_hour"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tminute: %d\n",get_integer("data_set_minute"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tsecond: %d\n",get_integer("data_set_second"));
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
void DoradeBlockVold::validate() throw(Fault){

    validate_integer("DoradeBlockVold","volume_number");
    validate_string ("DoradeBlockVold","project_name");
    validate_integer("DoradeBlockVold","year");
    validate_integer("DoradeBlockVold","month");
    validate_integer("DoradeBlockVold","day");
    validate_integer("DoradeBlockVold","data_set_hour");
    validate_integer("DoradeBlockVold","data_set_minute");
    validate_integer("DoradeBlockVold","data_set_second");
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockVold * DoradeBlockVold::castToDoradeBlockVold(){
    return this;
}

