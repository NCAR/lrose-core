//
//
//
//

#include <stdio.h>

#include <map>

#include "DoradeBlockRktb.h"
using namespace std;
using namespace ForayUtility;


// Static values
string  DoradeBlockRktb::id_("RKTB");

#define ANGLE2INDEX          1.0
#define INDEX_QUEUE_SIZE     540
#define INDEX_QUEUE_OFFSET   32


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRktb::DoradeBlockRktb(){

    doubleValues_ ["angle_to_index"]     = ANGLE2INDEX;
    integerValues_["index_queue_size"]   = INDEX_QUEUE_SIZE;
    integerValues_["index_queue_offset"] = INDEX_QUEUE_OFFSET;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRktb::~DoradeBlockRktb(){


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRktb::test(Buffer &buffer) throw(Fault){

    try {
	if(id_ != buffer.get_string_from_char(0,4)){
	    return false;
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockRktb::test: caught Fault.\n");
	throw re;
    }
    
    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockRktb::write_size(const int numberOfRays) throw(Fault){

    return calculate_block_size(numberOfRays);
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeBlockRktb::decode(Buffer &buffer) throw(Fault){

    try{
	if(!test(buffer)){
	    return false;
	}
	stringValues_ ["id"]                 = buffer.get_string_from_char ( 0,4);
	integerValues_["block_size"]         = buffer.get_four_byte_integer( 4);
	doubleValues_ ["angle_to_index"]     = buffer.get_four_byte_float  ( 8);
	integerValues_["index_queue_size"]   = buffer.get_four_byte_integer(12);
	integerValues_["first_key_offset"]   = buffer.get_four_byte_integer(16);
	integerValues_["index_queue_offset"] = buffer.get_four_byte_integer(20);
	integerValues_["number_of_rays"]     = buffer.get_four_byte_integer(24);

	int indexQueueSize   = get_integer("index_queue_size");
	int indexQueueOffset = get_integer("index_queue_offset");

	for(int index = 0; index < indexQueueSize; index++){
	    int loc = indexQueueOffset + (4 * index);
	    set_integer("index_queue",index,buffer.get_four_byte_integer(loc));
	}

	int numberOfRays   = get_integer("number_of_rays");
	int keyTableOffset = get_integer("first_key_offset");

	for(int index = 0; index < numberOfRays; index++){
	    int loc = keyTableOffset + (index * 12);
	    set_double ("rotation_angle",index,buffer.get_four_byte_float  (loc));
	    set_integer("offset"        ,index,buffer.get_four_byte_integer(loc + 4));
	    set_integer("size"          ,index,buffer.get_four_byte_integer(loc + 8));
	}

	validate();

    }catch(Fault re){
	re.add_msg("DoradeBlockRktb::decode:: caught Fault \n");
	throw re;
    }catch(...){
	throw Fault("DoradeBlockRktb::decode: caught exception \n");
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRktb::encode(Buffer &buffer) throw(Fault){

    unsigned char *bufferData;

    try {

	int indexQueueSize   = get_integer("index_queue_size");
	int indexQueueOffset = get_integer("index_queue_offset");

	int firstKeyOffset = indexQueueOffset + (4 * indexQueueSize);

	int numberOfRays   = get_integer("number_of_rays");

	int blockSize = calculate_block_size(numberOfRays);
	bufferData = buffer.new_data(blockSize);

	calculate_index_queue();

	buffer.set_string           (  0,id_,4);
	buffer.set_four_byte_integer(  4,blockSize);
	buffer.set_four_byte_float  (  8,get_double ("angle_to_index"));
	buffer.set_four_byte_integer( 12,             indexQueueSize);
	buffer.set_four_byte_integer( 16,             firstKeyOffset);
	buffer.set_four_byte_integer( 20,             indexQueueOffset);
	buffer.set_four_byte_integer( 24,             numberOfRays);
	


	for(int index = 0; index < indexQueueSize; index++){
	    int loc = indexQueueOffset + (index * 4);
	    buffer.set_four_byte_integer(loc,get_integer("index_queue",index));
	}

	for(int index = 0; index < numberOfRays; index++){
	    int loc = firstKeyOffset + (12 * index);
	    buffer.set_four_byte_float  (loc     ,get_double ("rotation_angle",index));
	    buffer.set_four_byte_integer(loc + 4,get_integer("offset"        ,index));
	    buffer.set_four_byte_integer(loc + 8,get_integer("size"          ,index));
	}

    }catch(Fault &re){
	re.add_msg("DoradeBlockRktb::encode : caught Fault \n");
	throw re;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeBlockRktb::listEntry(){

    string returnString("");
    char lineChar[1024];

    sprintf(lineChar,"%4s %5d \n", stringValues_["id"].c_str(),integerValues_["block_size"]);
    returnString += string(lineChar);

    sprintf(lineChar,"\tangle_to_index: %f \n",get_double("angle_to_index"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tindex_queue_size: %d \n",get_integer("index_queue_size"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tfirst_key_offset: %d \n",get_integer("first_key_offset"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tindex_queue_offset: %d \n",get_integer("index_queue_offset"));
    returnString += string(lineChar);

    sprintf(lineChar,"\tnum_rays: %d \n",get_integer("number_of_rays"));
    returnString += string(lineChar);

    return returnString;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRktb::validate() throw(Fault){

    validate_double ("DoradeBlockRktb","angle_to_index");
    validate_integer("DoradeBlockRktb","index_queue_size");
    validate_integer("DoradeBlockRktb","index_queue_offset");
    validate_integer("DoradeBlockRktb","number_of_rays");

    int numberOfIndexes = get_integer("index_queue_size");
    for(int index = 0; index < numberOfIndexes; index++){
	validate_integer("DoradeBlockRktb","index_queue",index);
    }

    int numberOfRays = get_integer("number_of_rays");
    for(int index = 0; index < numberOfRays; index++){
	validate_double ("DoradeBlockRktb","rotation_angle",index);
	validate_integer("DoradeBlockRktb","offset"        ,index);
	validate_integer("DoradeBlockRktb","size"          ,index);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeBlockRktb::calculate_index_queue() throw(Fault){

    int indexQueueOffset = get_integer("index_queue_offset");
    int numberOfRays     = get_integer("number_of_rays");

    map<double,int>angleQueue;
    map<double,int>::iterator angleQueueIt;

    for(int index = 0; index < numberOfRays; index++){
	angleQueue[get_double("rotation_angle",index)]  = index;
    }

    // To make sure that the 360 to 0 values
    // are handled correctly insert first value
    // at first_angle+360 and last value at last_angle - 360.
    //  Don't do this if RHI scan.
    angleQueueIt = angleQueue.begin();
    double firstAngle = angleQueueIt->first;
    int    firstIndex = angleQueueIt->second;

    angleQueueIt = angleQueue.end();
    angleQueueIt--;
    double lastAngle = angleQueueIt->first;
    int    lastIndex = angleQueueIt->second;

    angleQueue[firstAngle + 360.0] = firstIndex;
    angleQueue[lastAngle - 360.0]  = lastIndex;

    int numberOfIndexes = get_integer("index_queue_size");
    double indexToAngle = 360.0/(double)numberOfIndexes;

    set_double("angle_to_index",1.0/indexToAngle);

    double angle;

    map<double,int>::iterator findIt; 

    for(int index = 0; index < numberOfIndexes; index++){
	angle = (double)index * indexToAngle;

	findIt = angleQueue.lower_bound((double)angle);
	
	double highAngle = findIt->first;
	int    highIndex = findIt->second;

	--findIt;

	double lowAngle = findIt->first;
	int    lowIndex = findIt->second;

	if((angle - lowAngle) < (highAngle - angle)){
	    set_integer("index_queue",index,lowIndex);
	}else{
	    set_integer("index_queue",index,highIndex);
	}
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlockRktb * DoradeBlockRktb::castToDoradeBlockRktb(){
    return this;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeBlockRktb::calculate_block_size(const int numberOfRays) throw(Fault){

    int blockSize;

    blockSize = INDEX_QUEUE_OFFSET + (INDEX_QUEUE_SIZE * 4) + (numberOfRays * 12);

    return blockSize;
}




