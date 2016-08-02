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

#include <errno.h>
extern int errno;
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "RaycTime.h"

#include "Dorade.h"
#include "DoradeBlockSswb.h"
#include "DoradeBlockVold.h"
#include "DoradeBlockRadd.h"
#include "DoradeBlockParm.h"
#include "DoradeBlockCsfd.h"
#include "DoradeBlockCelv.h"
#include "DoradeBlockCfac.h"
#include "DoradeBlockSwib.h"
#include "DoradeBlockRyib.h"
#include "DoradeBlockRdat.h"
#include "DoradeBlockNull.h"
#include "DoradeBlockRktb.h"

#include "DoradeFile.h"
using namespace std;
using namespace ForayUtility;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::write_ground_headers() throw(Fault){

    if(file_ == NULL){
	throw Fault("DoradeFile::write_ground_headers: file not opened.\n");
    }

    if(newFile_ == false){
	throw Fault("DoradeFile::write_ground_headers: file was open for reading.\n");
    }

    Buffer blockBuffer;
    blockBuffer.is_big_endian(false);

    try {

	int numberOfFields  = get_integer("number_of_fields");
	int numberOfCells   = get_integer("number_of_cells");

	calculate_file_offsets_and_size();

	build_sswb (blockBuffer);
	write_block(blockBuffer);
	build_vold (blockBuffer);
	write_block(blockBuffer);
	build_cfac(blockBuffer);
	write_block(blockBuffer);

	integerValues_["number_of_radar_blocks"] = numberOfFields + 1;
	
	build_radd (blockBuffer);
	write_block(blockBuffer);
	
	for(int index = 0; index < numberOfFields; index++){
	    build_parm (blockBuffer,index);
	    write_block(blockBuffer);
	}


	int cellSpacingMethod = get_integer("cell_spacing_method");
	if(cellSpacingMethod == (int)RayConst::cellSpacingByVector){
	    build_celv (blockBuffer);
	    write_block(blockBuffer);
	}else if(cellSpacingMethod == (int)RayConst::cellSpacingBySegment) {
	    build_csfd (blockBuffer);
	    write_block(blockBuffer);
	}else{
	    char message[2048];
	    sprintf(message,"DoradeFile::write_ground_headers: cell spacing method of %d not recognized.\n",
		    cellSpacingMethod);
	    throw Fault(message);
	}

	build_swib (blockBuffer);
	write_block(blockBuffer);

	RayIntegers blankInt;
	rayIntegerData_.reserve(numberOfFields);
	rayIntegerData_.assign(numberOfFields,blankInt);

	RayDoubles  blankDouble;
	rayDoubleData_.reserve(numberOfFields);
	rayDoubleData_.assign(numberOfFields,blankDouble);

	for(int index = 0; index < numberOfFields; index++){
	    rayIntegerData_[index].reserve(numberOfCells);
	    rayDoubleData_[index].reserve(numberOfCells);
	}

	integerValues_["ray_index"] = 0;
	
    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::write_ground_headers: Caught Fault.\n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::write_ground_ray() throw(Fault){

    if(file_ == NULL){
	throw Fault("DoradeFile::write_ground_headers: file not opened.\n");
    }

    if(newFile_ == false){
	throw Fault("DoradeFile::write_ground_headers: file was open for reading.\n");
    }

    Buffer blockBuffer;
    blockBuffer.is_big_endian(false);
    
    try{
	// For RKTB block, we test for scan type.  (RHI vs. PPI/SUR)
	int scan_mode = get_integer("scan_mode");
	switch (scan_mode) {
	case RayConst::scanModePPI: // FALLTHRU
	case RayConst::scanModeSUR:
	    set_double("ray_rotation_angle",get_integer("ray_index"),
		get_RaycAngle("ray_azimuth").value());
	    break;

	case RayConst::scanModeRHI:
	    set_double("ray_rotation_angle",get_integer("ray_index"),
		90.0 - get_RaycAngle("ray_elevation").value() );
	    break;

	default:
	    // what do we do for CAL, COP, VER, TAR, MAN, IDL, AIR, HOR???
	    set_double("ray_rotation_angle",get_integer("ray_index"),
		get_RaycAngle("ray_azimuth").value());
	}

	build_ryib (blockBuffer);
	write_block(blockBuffer);

	int numberOfFields = get_integer("number_of_fields");
	for(int fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){
	    build_rdat (blockBuffer,fieldIndex);
	    write_block(blockBuffer);
	}

	integerValues_["ray_index"]++;

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::write_ground_ray: Caught Fault.\n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::write_ground_tail() throw(Fault){

    if(file_ == NULL){
	throw Fault("DoradeFile::write_ground_tail: file not opened.\n");
    }

    if(newFile_ == false){
	throw Fault("DoradeFile::write_ground_tail: file was open for reading.\n");
    }

    Buffer blockBuffer;
    blockBuffer.is_big_endian(false);
    
    try{

	build_null (blockBuffer);
	write_block(blockBuffer);

	build_rktb (blockBuffer);
	write_block(blockBuffer);


    }catch(Fault &fault){
	fault.add_msg("DoradeFile::write_ground_tail: Caught Fault.\n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::write_block(Buffer &buffer)  throw(Fault){

    if(file_ == NULL){
	throw Fault("DoradeFile::write_ground_headers: file not opened.\n");
    }

    if(newFile_ == false){
	throw Fault("DoradeFile::write_ground_headers: file was open for reading.\n");
    }

    int blockSize             = buffer.current_size();
    const unsigned char *data = buffer.data(0);

    if(fwrite(data,blockSize,1,file_) != 1){
	char msg[2048];
	sprintf(msg,
		"DoradeFile::write_block: failed to write block: %s, %d\n",
		strerror(errno),errno);
	throw Fault(msg);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_sswb(Buffer &buffer)  throw(Fault){

    DoradeBlockSswb sswb;

    try {

	// Validate DoradeFile values.
	validate_RaycTime("DoradeFile","start_time");
	validate_RaycTime("DoradeFile","stop_time");
	validate_integer ("DoradeFile","size_of_file");
	validate_integer ("DoradeFile","number_of_fields");
	
	// set values
	RaycTime startTime = get_RaycTime("start_time");
	RaycTime stopTime  = get_RaycTime("stop_time");
	sswb.set_double ("start_time"      ,(double) startTime.seconds());
	sswb.set_double ("stop_time"       ,(double) stopTime.seconds());
	sswb.set_integer("size_of_file"    ,get_integer("size_of_file"));
	sswb.set_integer("compression_flag",0);
	sswb.set_integer("number_of_fields",get_integer("number_of_fields"));
	sswb.set_string ("radar_name"      ,get_string ("platform_name"));

	sswb.set_integer("num_key_tables"  ,1);
	sswb.set_integer("key_offset"      ,0,get_integer("rktb_offset"));
	sswb.set_integer("key_size"        ,0,DoradeBlockRktb::write_size(get_integer("number_of_rays")));
	sswb.set_integer("key_type"        ,0,2);  // Need to make constant in Dorade.h

	sswb.encode(buffer);
			
    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_sswb: Caught Fault \n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_vold(Buffer &buffer)  throw(Fault){

    DoradeBlockVold vold;

    try {

	// Validate DoradeFile values.
	validate_integer ("DoradeFile","volume_number");
	validate_string  ("DoradeFile","project_name");
	validate_RaycTime("DoradeFile","start_time");
	
	// set values
	RaycTime startTime = get_RaycTime("start_time");

	vold.set_integer("volume_number"    ,get_integer("volume_number"));
	vold.set_string ("project_name"     ,get_string ("project_name"));
	vold.set_integer("year"             ,startTime.get_year());
	vold.set_integer("month"            ,startTime.get_month());
	vold.set_integer("day"              ,startTime.get_day());
	vold.set_integer("data_set_hour"    ,startTime.get_hour());
	vold.set_integer("data_set_minute"  ,startTime.get_minute());
	vold.set_integer("data_set_second"  ,startTime.get_second());
	vold.set_integer("number_sensor_des",1);

	vold.encode(buffer);
			
    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_vold: Caught Fault \n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_radd(Buffer &buffer)  throw(Fault){

    DoradeBlockRadd radd;

    try {

	// Validate DoradeFile values.
	validate_string ("DoradeFile","platform_name");
	validate_integer("DoradeFile","platform_type");
	validate_integer("DoradeFile","scan_mode");
	validate_integer("DoradeFile","number_of_fields");
	validate_double ("DoradeFile","platform_longitude");
	validate_double ("DoradeFile","platform_latitude");
	validate_double ("DoradeFile","platform_altitude");
	
	radd.set_string ("radar_name"             ,get_string ("platform_name"));
	radd.set_double ("radar_constant"         ,get_double ("radar_constant"));
	radd.set_double ("peak_power"             ,get_double ("peak_power"));
	radd.set_double ("noise_power"            ,get_double ("noise_power"));
	radd.set_double ("receiver_gain"          ,get_double ("receiver_gain"));
	radd.set_double ("antenna_gain"           ,get_double ("antenna_gain"));
	radd.set_double ("system_gain"            ,get_double ("system_gain"));
	radd.set_double ("horizontal_beam_width"  ,get_double ("horizontal_beam_width"));
	radd.set_double ("vertical_beam_width"    ,get_double ("vertical_beam_width"));
	radd.set_integer("radar_type"             ,get_integer("platform_type"));
	radd.set_integer("scan_mode"              ,get_integer("scan_mode"));
	radd.set_integer("number_of_fields"       ,get_integer("number_of_fields"));
	radd.set_integer("total_num_des"          ,get_integer("number_of_radar_blocks"));
	radd.set_integer("data_compress"          ,0);  // None
	radd.set_integer("data_reduction"         ,0);  // None
	radd.set_double ("data_reduction_param0"  ,0.0);
	radd.set_double ("data_reduction_param1"  ,0.0);
	radd.set_double ("radar_longitude"        ,get_double("platform_longitude"));
	radd.set_double ("radar_latitude"         ,get_double("platform_latitude"));
	radd.set_double ("radar_altitude"         ,get_double("platform_altitude") / 1000.0);
	radd.set_double ("eff_unamb_vel"          ,get_double("nyquist_velocity"));
        radd.set_double ("eff_unamb_range_km"     ,get_double("unambiguous_range") /1000.0);
	radd.set_double ("pulse_width"            ,get_double("pulse_width") * 1000000.0);

	int numberOfFrequencies =                  get_integer("number_of_frequencies");
	radd.set_integer("number_of_frequencies"  ,numberOfFrequencies);
	for(int index = 0; index < numberOfFrequencies; index++){
	    radd.set_double("frequency",index, get_double("frequency",index));
	}

	int numberOfPrfs =                         get_integer("number_of_prfs");
	radd.set_integer("number_of_ipps"  ,numberOfPrfs);
	for(int index = 0; index < numberOfPrfs; index++){
	    radd.set_double("interpulse_period",index, 1000.0/get_double("pulse_repetition_frequency",index));
	}

	radd.encode(buffer);
			
    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_radd: Caught Fault \n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_parm(Buffer &buffer,const int index)  throw(Fault){


    DoradeBlockParm parm;

    try {

	validate_string ("DoradeFile","field_name"     ,index);
	validate_integer("DoradeFile","binary_format"  ,index);
	validate_double ("DoradeFile","parameter_scale",index);
	validate_double ("DoradeFile","parameter_bias" ,index);
	validate_integer("DoradeFile","bad_data"       ,index);
	validate_integer("DoradeFile","number_of_cells");
	validate_double ("DoradeFile","meters_to_first_cell");

	string units("unknown");
	int    unitsId = get_integer("field_units",index);
	if(unitsId == (int)RayConst::unitsNone){
	    units = "none";
	}else if(unitsId == (int)RayConst::unitsReflectivity){
	    units = "dBz";
	}else if(unitsId == (int)RayConst::unitsPower){
	    units = "dBm";
	}else if(unitsId == (int)RayConst::unitsVelocity){
	    units = "m/s";
	}

	parm.set_string ("field_name"          ,get_string ("field_name"     ,index));
	parm.set_string ("param_description"   ,get_string ("field_long_name",index));
	parm.set_string ("param_units"         ,units);
	parm.set_integer("number_of_samples"   ,get_integer("number_of_samples"));
	parm.set_integer("binary_format"       ,get_integer("binary_format"  ,index));

	double forayScale  = get_double("parameter_scale",index);
	double forayBias   = get_double("parameter_bias",index);
	double doradeScale = 1.0/forayScale; 
	double doradeBias  = -1.0 * forayBias / forayScale;
 
	parm.set_double ("parameter_scale"     ,doradeScale);
	parm.set_double ("parameter_bias"      ,doradeBias);
	parm.set_integer("bad_data"            ,get_integer("bad_data"       ,index));
	parm.set_integer("offset_to_data"      ,16);    // Assume RDAT.
	parm.set_integer("number_of_cells"     ,get_integer("number_of_cells"));
	parm.set_double ("meters_to_first_cell",-1.0);  // Not used.
	parm.set_double ("meters_between_cells",-1.0);  // Not Used.

	parm.encode(buffer);

    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_parm: Caught Fault \n");
	throw(fault);

    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_csfd(Buffer &buffer)  throw(Fault){

    DoradeBlockCsfd csfd;

    try {
	
	validate_double ("DoradeFile","meters_to_first_cell");
	validate_integer("DoradeFile","number_of_cell_segments");
	int numberOfSegments = get_integer("number_of_cell_segments");
	int computedNumberOfCells(0);
	for(int index = 0; index < numberOfSegments; index++){
	    validate_double ("DoradeFile","segment_cell_spacing",index);
	    validate_integer("DoradeFile","segment_cell_count"  ,index);
	    
	    computedNumberOfCells +=  get_integer("segment_cell_count",index);
	}
	
	validate_integer("DoradeFile","number_of_cells");
	int numberOfCells = get_integer("number_of_cells");
	if(numberOfCells != computedNumberOfCells){
	    char msg[2048];
	    sprintf(msg,"DoradeFile::build_csfd: computed cell count (%d) not equal to set cell count(%d)\n",
		    computedNumberOfCells,numberOfCells);
	    throw Fault(msg);
	}

	csfd.set_double ("meters_to_first_cell"    ,get_double("meters_to_first_cell"));
	csfd.set_integer("number_of_cell_segments" ,numberOfSegments);

	for(int index = 0; index < numberOfSegments; index++){
	    csfd.set_integer("segment_cell_count",index,get_integer("segment_cell_count"  ,index));
	    csfd.set_double ("spacing"           ,index,get_double ("segment_cell_spacing",index));
	}

	csfd.encode(buffer);

    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_csfd: Caught Fault \n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_celv(Buffer &buffer)  throw(Fault){

    DoradeBlockCelv celv;

    try {
	validate_integer("DoradeFile","number_of_cells");

	celv.set_integer("number_of_cells",get_integer("number_of_cells"));
	celv.set_double_vector(cellVector_);

	celv.encode(buffer);

    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_celv: Caught Fault \n");
	throw(fault);
    }
}
//////////////////////////////////////////////////////////////////////
//
// Build an empty correction factor record, since the vintage dorade library
// expects one
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_cfac(Buffer &buffer)  throw(Fault){

    DoradeBlockCfac cfac;

    try {

	cfac.encode(buffer);

    }catch(Fault &fault){
	char msg[2048];
	fault.add_msg("DoradeFile::build_celv: Caught Fault \n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_swib(Buffer &buffer)  throw(Fault){

    DoradeBlockSwib swib;
    
    try{

	validate_string   ("DoradeFile","platform_name"    );
	validate_integer  ("DoradeFile","sweep_number"  );
	validate_integer  ("DoradeFile","number_of_rays");
	validate_RaycAngle("DoradeFile","fixed_angle"   );
	validate_RaycAngle("DoradeFile","start_angle"   );
	validate_RaycAngle("DoradeFile","stop_angle"    );

	swib.set_string ("radar_name"  ,get_string   ("platform_name"));
	swib.set_integer("sweep_num"   ,get_integer  ("sweep_number"));
	swib.set_integer("num_rays"    ,get_integer  ("number_of_rays"));
	swib.set_double ("fixed_angle" ,get_RaycAngle("fixed_angle").value());
	swib.set_double ("start_angle" ,get_RaycAngle("start_angle").value());
	swib.set_double ("stop_angle"  ,get_RaycAngle("stop_angle").value());
	swib.set_integer("filter_flag" ,0);

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::build_swib: Caught Fault \n");
	throw(fault);
    }

    swib.encode(buffer);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_ryib(Buffer &buffer)  throw(Fault){

    DoradeBlockRyib ryib;

    try{
	validate_integer  ("DoradeFile","sweep_number");
	validate_RaycTime ("DoradeFile","ray_time");
	validate_RaycAngle("DoradeFile","ray_azimuth");
	validate_RaycAngle("DoradeFile","ray_elevation");
	validate_double   ("DoradeFile","ray_peak_power");
	validate_double   ("DoradeFile","ray_true_scan_rate");
	validate_integer  ("DoradeFile","ray_status");

	RaycTime time(get_RaycTime("ray_time"));

	ryib.set_integer("sweep_num"     ,get_integer("sweep_number"));
	ryib.set_integer("julian_day"    ,time.get_julian_day());
	ryib.set_integer("hour"          ,time.get_hour());
	ryib.set_integer("minute"        ,time.get_minute());
	ryib.set_integer("second"        ,time.get_second());
	ryib.set_integer("millisecond"   ,time.get_nanosecond()/1000000);
	ryib.set_double ("azimuth"       ,get_RaycAngle("ray_azimuth").value());
	ryib.set_double ("elevation"     ,get_RaycAngle("ray_elevation").value());
	ryib.set_double ("peak_power"    ,get_double   ("ray_peak_power"));
	ryib.set_double ("true_scan_rate",get_double   ("ray_true_scan_rate"));
	ryib.set_integer("ray_status"    ,get_integer  ("ray_status"));
	
    }catch(Fault &fault){
	fault.add_msg("DoradeFile::build_ryib: Caught Fault \n");
	throw(fault);
    }

    ryib.encode(buffer);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_rdat(Buffer &buffer,const int fieldIndex)  throw(Fault){

    DoradeBlockRdat rdat;

    try{
	validate_string ("DoradeFile","field_name"     ,fieldIndex);
	validate_integer("DoradeFile","number_of_cells");
	validate_integer("DoradeFile","binary_format"  ,fieldIndex);

	int binaryFormat = get_integer("binary_format",fieldIndex);

	rdat.set_string("pdata_name"    ,get_string("field_name",fieldIndex));

	if((binaryFormat == Dorade::binaryFormat1ByteInt) ||
	   (binaryFormat == Dorade::binaryFormat2ByteInt)){

	    rdat.encode(buffer,binaryFormat,rayIntegerData_[fieldIndex]);

	}else if(binaryFormat == Dorade::binaryFormat4ByteFloat){

	    rdat.encode(buffer,binaryFormat,rayDoubleData_[fieldIndex]);

	}else{
	    char msg[2048];
	    sprintf(msg,"DoradeFile::build_rdat: Cannot encode data with binary format value of %d.\n",
		    binaryFormat);
	    throw Fault(msg);
	}

	
    }catch(Fault &fault){
	fault.add_msg("DoradeFile::build_rdat: Caught Fault \n");
	throw(fault);
    }


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_null(Buffer &buffer)  throw(Fault){

    DoradeBlockNull null;

    try{

	null.encode(buffer);

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::build_null: Caught Fault \n");
	throw(fault);
    }


}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::build_rktb(Buffer &buffer)  throw(Fault){

    DoradeBlockRktb rktb;

    try{

	int numberOfRays = get_integer("number_of_rays");

	//	rktb.set_integer("index_queue_size",540);

	rktb.set_integer("number_of_rays",numberOfRays);
	
	// Key Table data.
	for(int index = 0; index < numberOfRays; index++){
	    rktb.set_double ("rotation_angle",index,get_double ("ray_rotation_angle",index));
	    rktb.set_integer("offset"        ,index,get_integer("ray_offset"        ,index));
	    rktb.set_integer("size"          ,index,get_integer("ray_size")                );
	}

	rktb.encode(buffer);

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::build_rktb: Caught Fault \n");
	throw(fault);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::set_ray_data(int fieldIndex, RayIntegers &ri) throw(Fault){

    validate_integer("DoradeFile::set_ray_data","number_of_fields");

    if((fieldIndex < 0) || (fieldIndex >= get_integer("number_of_fields"))){
	char msg[2048];
	sprintf(msg,"DoradeFile::set_ray_data: Field Index of %d is out of bounds. (Number of fields is %d.\n ",
		fieldIndex,get_integer("number_of_fields"));
	throw Fault(msg);
    }

    validate_integer("DoradeFile::set_ray_data","number_of_cells");
    validate_integer("DoradeFile::set_ray_data","binary_format"  ,fieldIndex);
    validate_double ("DoradeFile::set_ray_data","parameter_scale",fieldIndex);
    validate_double ("DoradeFile::set_ray_data","parameter_bias" ,fieldIndex);

    int    numberOfCells = get_integer("number_of_cells");
    int    binaryFormat  = get_integer("binary_format"   ,fieldIndex);
    double forayBias     = get_double ("parameter_bias"  ,fieldIndex);
    double forayScale    = get_double ("parameter_scale" ,fieldIndex);

    if(ri.size() < numberOfCells){
	char msg[2048];
	sprintf(msg,"DoradeFile::set_ray_data: RayInteger size of %d is less then number_of_cells value of %d.\n",
		ri.size(),numberOfCells);
	throw Fault(msg);
    }
    
    if((binaryFormat == Dorade::binaryFormat1ByteInt) ||
       (binaryFormat == Dorade::binaryFormat2ByteInt) ||
       (binaryFormat == Dorade::binaryFormat3ByteInt)){

	rayIntegerData_[fieldIndex].clear();

	for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
	    rayIntegerData_[fieldIndex].push_back(ri[cellIndex]);
	}
    }else if((binaryFormat == Dorade::binaryFormat4ByteFloat) ||
	     (binaryFormat == Dorade::binaryFormat2ByteFloat)){

	rayDoubleData_[fieldIndex].clear();

	for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){

	    double value = (double)ri[cellIndex];

	    rayDoubleData_[fieldIndex].push_back((value * forayScale) + forayBias);
	}
    }else{
	char msg[2048];
	sprintf(msg,"DoradeFile::set_ray_data: bianry_format value of %d is undefined.",
		binaryFormat);
	throw Fault(msg);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::set_ray_data(int fieldIndex, RayDoubles  &rd) throw(Fault){

    validate_integer("DoradeFile::set_ray_data","number_of_fields");

    if((fieldIndex < 0) || (fieldIndex >= get_integer("number_of_fields"))){
	char msg[2048];
	sprintf(msg,"DoradeFile::set_ray_data: Field Index of %d is out of bounds. (Number of fields is %d.\n ",
		fieldIndex,get_integer("number_of_fields"));
	throw Fault(msg);
    }

    validate_integer("DoradeFile::set_ray_data","number_of_cells");
    validate_integer("DoradeFile::set_ray_data","binary_format"  ,fieldIndex);
    validate_double ("DoradeFile::set_ray_data","parameter_scale",fieldIndex);
    validate_double ("DoradeFile::set_ray_data","parameter_bias" ,fieldIndex);

    int    numberOfCells = get_integer("number_of_cells");
    int    binaryFormat  = get_integer("binary_format"   ,fieldIndex);
    double forayBias     = get_double ("parameter_bias"  ,fieldIndex);
    double forayScale    = get_double ("parameter_scale" ,fieldIndex);
    int    badData       = get_integer("bad_data"        ,fieldIndex);

    if(rd.size() < numberOfCells){
	char msg[2048];
	sprintf(msg,"DoradeFile::set_ray_data: RayDoubles size of %d is less then number_of_cells value of %d.\n",
		rd.size(),numberOfCells);
	throw Fault(msg);
    }
    
    if((binaryFormat == Dorade::binaryFormat1ByteInt) ||
       (binaryFormat == Dorade::binaryFormat2ByteInt) ||
       (binaryFormat == Dorade::binaryFormat3ByteInt)){

	rayIntegerData_[fieldIndex].clear();

	for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
	    double value = rd[cellIndex];
	    
	    if((int)value == badData){
		rayIntegerData_[fieldIndex].push_back(badData);

	    }else{

		double scaled = (value - forayBias) / forayScale;

		if(scaled >= 0.0){
		    rayIntegerData_[fieldIndex].push_back((int)(scaled + 0.5));
		}else{
		    rayIntegerData_[fieldIndex].push_back((int)(scaled - 0.5));
		}
	    }
	}
    }else if((binaryFormat == Dorade::binaryFormat4ByteFloat) ||
	     (binaryFormat == Dorade::binaryFormat2ByteFloat)){

	rayDoubleData_[fieldIndex].clear();

	for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
	    rayDoubleData_[fieldIndex].push_back(rd[cellIndex]);
	}

    }else{
	char msg[2048];
	sprintf(msg,"DoradeFile::set_ray_data: bianry_format value of %d is undefined.",
		binaryFormat);
	throw Fault(msg);
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::calculate_file_offsets_and_size() throw (Fault){

    try {

	int numberOfFields    = get_integer("number_of_fields");
	int numberOfCells     = get_integer("number_of_cells");
	int numberOfRays      = get_integer("number_of_rays");
	int cellSpacingMethod = get_integer("cell_spacing_method");

	int headerSize(0);
	headerSize = 
	    DoradeBlockSswb::write_size() +
	    DoradeBlockVold::write_size() +
	    DoradeBlockRadd::write_size() +
	   (DoradeBlockParm::write_size() * numberOfFields) +
	    DoradeBlockCfac::write_size() +
	    DoradeBlockSwib::write_size();

	if(cellSpacingMethod == (int)RayConst::cellSpacingByVector){
	    headerSize += DoradeBlockCelv::write_size(numberOfCells);
	}else if(cellSpacingMethod == (int)RayConst::cellSpacingBySegment){
	    headerSize += DoradeBlockCsfd::write_size();
	}else{
	    throw Fault("DoradeFile::calculate_file_offsets_and_size: Unknown cell spacing method.\n");
	}


	// Calculate ray_size
	int raySize = DoradeBlockRyib::write_size();
	
	for(int index = 0; index < numberOfFields; index++){

	    // For RDAT's
	    raySize += 16;
	    
	    int binary_format = get_integer("binary_format",index);
	    if(binary_format == Dorade::binaryFormat1ByteInt){
		raySize += numberOfCells;
	    }else if(binary_format == Dorade::binaryFormat2ByteInt) {
		raySize += (numberOfCells * 2);
	    }else if(binary_format == Dorade::binaryFormat4ByteFloat){
		raySize += (numberOfCells * 4);
	    }else{
		char msg[2048];
		sprintf(msg,"DoradeFile::calculate_file_size: Can not encode values using binary format type %d \n",
			binary_format);
		throw Fault(msg);
	    }
	}

	set_integer("ray_size",raySize);

	// Calculate ray_offsets
	for(int index = 0; index < numberOfRays; index++){
	    int rayOffset = headerSize + (index * raySize);
	    set_integer("ray_offset",index,rayOffset);
	}


	int nullSize = DoradeBlockNull::write_size();
	
	int rktbSize = DoradeBlockRktb::write_size(numberOfRays);
	
	integerValues_["rktb_size"]    = rktbSize;
	integerValues_["rktb_offset"]  = headerSize + (raySize * numberOfRays) + nullSize;
	integerValues_["size_of_file"] = headerSize + (raySize * numberOfRays) + nullSize + rktbSize;

    }catch (Fault &fault){
	fault.add_msg("DoradeFile::calculate_file_size : caught Fault \n");
	throw(fault);
    }
}
