//
//
//
//

#include <stdio.h>

#include "Dorade.h"
#include "DoradeBlockRadd.h"
using namespace std;
using namespace ForayUtility;


// Static values
const string  DoradeBlockRadd::id_("RADD");
const int     DoradeBlockRadd::lengthA_(300);
const int     DoradeBlockRadd::lengthB_(144);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRadd::DoradeBlockRadd(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRadd::~DoradeBlockRadd(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRadd::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

	if((lengthA_ != buffer.get_four_byte_integer(4)) &&
	   (lengthB_ != buffer.get_four_byte_integer(4))){
	    return false;
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockRadd::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockRadd::write_size() throw(Fault){

    return lengthA_;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRadd::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]                    = buffer.get_string_from_char (  0,4);
	int blocksize                           = buffer.get_four_byte_integer(  4);
	integerValues_["block_size"]            = blocksize;
	stringValues_ ["radar_name"]            = buffer.get_string_from_char (  8,8);
	doubleValues_ ["radar_constant"]        = buffer.get_four_byte_float  ( 16);
	doubleValues_ ["peak_power"]            = buffer.get_four_byte_float  ( 20);
	doubleValues_ ["noise_power"]           = buffer.get_four_byte_float  ( 24);
	doubleValues_ ["receiver_gain"]         = buffer.get_four_byte_float  ( 28);
	doubleValues_ ["antenna_gain"]          = buffer.get_four_byte_float  ( 32);
	doubleValues_ ["system_gain"]           = buffer.get_four_byte_float  ( 36);
	doubleValues_ ["horizontal_beam_width"] = buffer.get_four_byte_float  ( 40);
	doubleValues_ ["vertical_beam_width"]   = buffer.get_four_byte_float  ( 44);
	integerValues_["radar_type"]            = buffer.get_two_byte_integer ( 48);
	integerValues_["scan_mode"]             = buffer.get_two_byte_integer ( 50);
	integerValues_["number_of_fields"]      = buffer.get_two_byte_integer ( 64);
	integerValues_["total_num_des"]         = buffer.get_two_byte_integer ( 66);
	integerValues_["data_compress"]         = buffer.get_two_byte_integer ( 68);
	integerValues_["data_reduction"]        = buffer.get_two_byte_integer ( 70);
	doubleValues_ ["data_reduction_param0"] = buffer.get_four_byte_float  ( 72);
	doubleValues_ ["data_reduction_param1"] = buffer.get_four_byte_float  ( 76);
	doubleValues_ ["radar_longitude"]       = buffer.get_four_byte_float  ( 80);
	doubleValues_ ["radar_latitude"]        = buffer.get_four_byte_float  ( 84);
	doubleValues_ ["radar_altitude"]        = buffer.get_four_byte_float  ( 88);
	doubleValues_ ["eff_unamb_vel"]         = buffer.get_four_byte_float  ( 92);
	doubleValues_ ["eff_unamb_range_km"]    = buffer.get_four_byte_float  ( 96);
	
	int numberOfFrequencies                 = buffer.get_two_byte_integer (100);
	integerValues_["number_of_frequencies"] = numberOfFrequencies;

	for(int index = 0; (index < numberOfFrequencies) && (index < 5); index++){
	    int location = 104 + (index * 4);
	    set_double("frequency",index,buffer.get_four_byte_float(location));
	}

	int numberOfIpps                        = buffer.get_two_byte_integer (102);
	integerValues_["number_of_ipps"]        = numberOfIpps;
	for(int index = 0; (index < numberOfIpps) && (index < 5); index++){
	    int location = 124 + (index * 4);
	    set_double("interpulse_period",index,buffer.get_four_byte_float(location));
	}
	
	if(blocksize == lengthA_){
	    doubleValues_["pulse_width"]        = buffer.get_four_byte_float  (256);
	}

	// remove any trailing spaces in 
	string radarName = stringValues_["radar_name"];
	int    space = radarName.find(" ");
	if(space != string::npos ){
	    stringValues_["radar_name"] = radarName.substr(0,space);
	}
	

	validate();

    }catch(Fault &re){
	re.add_msg("DoradeBlockRadd::decode : caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockRadd::decode: caught exception \n");
    }

    return true;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRadd::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {
	
	validate();

	int blockSize = lengthA_;
	bufferData = buffer.new_data(blockSize);

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_string           (  8,get_string ("radar_name"),8);
	buffer.set_four_byte_float  ( 16,get_double ("radar_constant"));
	buffer.set_four_byte_float  ( 20,get_double ("peak_power"));
	buffer.set_four_byte_float  ( 24,get_double ("noise_power"));
	buffer.set_four_byte_float  ( 28,get_double ("receiver_gain"));
	buffer.set_four_byte_float  ( 32,get_double ("antenna_gain"));
	buffer.set_four_byte_float  ( 36,get_double ("system_gain"));
	buffer.set_four_byte_float  ( 40,get_double ("horizontal_beam_width"));
	buffer.set_four_byte_float  ( 44,get_double ("vertical_beam_width"));
	buffer.set_two_byte_integer ( 48,get_integer("radar_type"));
	buffer.set_two_byte_integer ( 50,get_integer("scan_mode"));
	buffer.set_two_byte_integer ( 64,get_integer("number_of_fields"));
	buffer.set_two_byte_integer ( 66,get_integer("total_num_des"));
	buffer.set_two_byte_integer ( 68,get_integer("data_compress"));
	buffer.set_two_byte_integer ( 70,get_integer("data_reduction"));
	buffer.set_four_byte_float  ( 72,get_double ("data_reduction_param0"));
	buffer.set_four_byte_float  ( 76,get_double ("data_reduction_param1"));
	buffer.set_four_byte_float  ( 80,get_double ("radar_longitude"));
	buffer.set_four_byte_float  ( 84,get_double ("radar_latitude"));
	buffer.set_four_byte_float  ( 88,get_double ("radar_altitude"));
	buffer.set_four_byte_float  ( 92,get_double ("eff_unamb_vel"));
	buffer.set_four_byte_float  ( 96,get_double ("eff_unamb_range_km"));

	int numberOfFrequencies =        get_integer("number_of_frequencies");
	buffer.set_two_byte_integer (100,numberOfFrequencies);
	
	for(int index = 0; (index < numberOfFrequencies) && (index < 5); index++){
	    int location = 104 + (index * 4);
	    buffer.set_four_byte_float(location,get_double("frequency",index));
	}

	int numberOfIpps = get_integer("number_of_ipps");
	buffer.set_two_byte_integer (102,numberOfIpps);

	for(int index = 0; (index < numberOfIpps) && (index < 5); index++){
	    int location = 124 + (index * 4);
	    buffer.set_four_byte_float(location,get_double("interpulse_period",index));
	}

	buffer.set_four_byte_float  (256,get_double ("pulse_width"));

    }catch(Fault &re){
	re.add_msg("DoradeBlockRadd::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRadd::validate() throw(Fault){

    validate_string ("DoradeBlockRadd","radar_name");
    validate_integer("DoradeBlockRadd","radar_type");
    validate_integer("DoradeBlockRadd","scan_mode");
    validate_integer("DoradeBlockRadd","number_of_fields");
    validate_integer("DoradeBlockRadd","total_num_des");
    validate_integer("DoradeBlockRadd","data_compress");
    validate_integer("DoradeBlockRadd","data_reduction");
    validate_double ("DoradeBlockRadd","data_reduction_param0");
    validate_double ("DoradeBlockRadd","data_reduction_param1");
    validate_double ("DoradeBlockRadd","radar_longitude");
    validate_double ("DoradeBlockRadd","radar_latitude");
    validate_double ("DoradeBlockRadd","radar_altitude");

    try {
	int radarType = get_integer("radar_type");
	if((radarType < 0) || (radarType > 5)){
	    char msg[2048];
	    sprintf(msg,"DoradeBlockRadd::validate : radar type value of %d is invalid. \n",
		    radarType);
	    throw Fault(msg);
	}

	int scanMode = get_integer("scan_mode");
	if((scanMode < 0) || (scanMode > 10)){
	    char msg[2048];
	    sprintf(msg,"DoradeBlockRadd::validate : scan mode value of %d is invalid. \n",
		    scanMode);
	    throw Fault(msg);
	}
    }catch(Fault &re){
	re.add_msg("DoradeBlockSswb::validate : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockRadd::listEntry(){

    string returnString("");
    char lineChar[1024];

    sprintf(lineChar,"%4s %5d \n", stringValues_["id"].c_str(),integerValues_["block_size"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tRadar Name: \"%s\"\n",get_string("radar_name").c_str());
    returnString += string(lineChar);

    int radarType = get_integer("radar_type");
    sprintf(lineChar,"\tRadar Type: %d (%s)\n",radarType,Dorade::radarTypes[radarType]);
    returnString += string(lineChar);

    int scanMode = get_integer("scan_mode");
    sprintf(lineChar,"\tScan Type: %d (%s)\n",scanMode,Dorade::scanModes[scanMode]);
    returnString += string(lineChar);

    int numberOfFields = get_integer("number_of_fields");
    sprintf(lineChar,"\tNumber of Fields: %d\n",numberOfFields);
    returnString += string(lineChar);

    sprintf(lineChar,"\tRadar Longitude: %9.4f\n",get_double("radar_longitude"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tRadar Latitude: %9.4f\n",get_double("radar_latitude"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tRadar Altitude: %9.4f\n",get_double("radar_altitude"));
    returnString += string(lineChar);

    int numberOfFrequencies = get_integer("number_of_frequencies");
    sprintf(lineChar,"\tNumber of Frequencies: %d\n",numberOfFrequencies);
    returnString += string(lineChar);

    for(int index = 0; index < numberOfFrequencies; index++){
	sprintf(lineChar,"\t\tFrequency %2d: %9.4f\n",index,get_double("frequency",index));
	returnString += string(lineChar);

    }

    int numberOfIpps = get_integer("number_of_ipps");
    sprintf(lineChar,"\tNumber of Interpulse periods: %d\n",numberOfIpps);
    returnString += string(lineChar);

    for(int index = 0; index < numberOfIpps; index++){
	sprintf(lineChar,"\t\tInterpulse Period %2d: %9.4f\n",index,get_double("interpulse_period",index));
	returnString += string(lineChar);
    }

    sprintf(lineChar,"\n");
    returnString += string(lineChar);

    sprintf(lineChar,"\tNoise Power: %10.4f\n", get_double("noise_power"));
    returnString += string(lineChar);

    return returnString;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRadd * DoradeBlockRadd::castToDoradeBlockRadd(){
    return this;
}
