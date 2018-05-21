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
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

#include "FilePath.h"
using namespace ForayUtility;

#include "RayConst.h"

#include "Dorade.h"
#include "DoradeBlock.h"
#include "DoradeBlockUnknown.h"
#include "DoradeBlockSswb.h"
#include "DoradeBlockVold.h"
#include "DoradeBlockRadd.h"
#include "DoradeBlockParm.h"
#include "DoradeBlockCelv.h"
#include "DoradeBlockCsfd.h"
#include "DoradeBlockFrib.h"
#include "DoradeBlockCfac.h"
#include "DoradeBlockSwib.h"
#include "DoradeBlockRyib.h"
#include "DoradeBlockAsib.h"
#include "DoradeBlockXstf.h"
#include "DoradeBlockRdat.h"
#include "DoradeBlockNull.h"
#include "DoradeBlockRktb.h"

#include "DoradeFile.h"



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeFile::DoradeFile(){

    file_     = NULL;
    decoder_  = NULL;

    sswb_     = NULL;
    vold_     = NULL;
    radd_     = NULL;
    celv_     = NULL;
    csfd_     = NULL;
    swib_     = NULL;
    xstf_     = NULL;

    headers_read_ = false;
    newFile_      = false;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeFile::~DoradeFile(){

    close_file();

    if(decoder_ != NULL){
	delete decoder_;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::open_file(const string &filename, const bool newFile) throw(Fault) {

    char msg[2048];
    FilePath filePath;
    useBigEndian_ = false;

    if(file_ != NULL){
	close_file();
    }

    if(newFile){
	// bWrite new File.
	if((file_ = fopen(filename.c_str(),"w")) == NULL){
	    sprintf(msg,"DoradeFile::open_file failed: %s \n",strerror(errno));
	    throw Fault(msg);
	}
	newFile_ = true;
    }else{
	// Read existing file.
	if((file_ = fopen(filename.c_str(),"r")) == NULL){
	    sprintf(msg,"DoradeFile::open_file failed: %s \n",strerror(errno));
	    throw Fault(msg);
	}
	newFile_ = false;
	useBigEndian_ = test_big_endian();
    }

    filePath.file(filename);

    stringValues_["file_name"]      = filePath.get_name();
    stringValues_["directory_name"] = filePath.get_directory();

    if(decoder_ == NULL){
	decoder_ = new Decoder();
	decoder_->buffers_big_endian(useBigEndian_);
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::close_file(){

    if(file_ != NULL){
	fclose(file_);
    }

    file_ = NULL;

    if(sswb_ != NULL){
	delete sswb_;
	sswb_ = NULL;
    }

    if(vold_ != NULL){
	delete vold_;
	vold_ = NULL;
    }

    if(radd_ != NULL){
	delete radd_;
	radd_ = NULL;
    }

    while(parmVector_.size() != 0){
	DoradeBlockParm *parm = parmVector_.back();
	parmVector_.pop_back();
	delete parm;
    }

    if(celv_ != NULL){
	delete celv_;
	celv_ = NULL;
    }

    if(csfd_ != NULL){
	delete csfd_;
	csfd_ = NULL;
    }

    if(swib_ != NULL){
	delete swib_;
	swib_ = NULL;
    }

    if(xstf_ != NULL){
	delete xstf_;
	xstf_ = NULL;
    }

    clear_rdatVector();

    integerValues_.clear();
    doubleValues_.clear();
    stringValues_.clear();

    raycTimeValues_.clear();
    raycAngleValues_.clear();

    fieldIndex_.clear();

    cellVector_.clear();

    headers_read_ = false;

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::reset_file(){

    if(file_ != NULL){
	fseek(file_,0,SEEK_SET);
    }

}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeFile::read_block_(Buffer &buffer) throw(Fault){

    char msg[2048];
    unsigned char blockHead[8];

    if(file_ == NULL){
	throw Fault("DoradeFile::read_block: file not opened.\n");
    }

    buffer.is_big_endian(useBigEndian_);

    if(fread(blockHead,8,1,file_) != 1){
	// End of file ?
	if(feof(file_) || (errno == 0)){
	    return false;
	}
	// Error
	sprintf(msg,
		"DoradeFile::read_block: failed to read block head: %s, %d\n",
		strerror(errno),errno);
	throw Fault(msg);
    }

    int            blockSize = decoder_->four_byte(&blockHead[4]);
    if (blockSize == 0) {
	sprintf(msg,
	    "DoradeFile::read_block : unexpected block size of 0\n");
	throw Fault(msg);
    }
    string         blockName = decoder_->char_string(blockHead,4);
    unsigned char *blockData = buffer.new_data(blockSize);

    memcpy(blockData,blockHead,8);

    if(blockSize > 8) {
	if(fread(&blockData[8],blockSize - 8, 1,file_) != 1){
	    sprintf(msg,
		    "DoradeFile::read_block: failed to read block: %s (%s,%d)\n",
		    strerror(errno),
		    blockName.c_str(),
		    blockSize);
	    throw Fault(msg);
	}
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeBlock *DoradeFile::read_next_block() throw(Fault){

    Buffer buffer;
    DoradeBlock *returnBlock(NULL);

    if(file_ == NULL){
	throw Fault("DoradeFile::read_next_block: file not opened \n");
    }

    
    try {

	if(read_block_(buffer)){

	    if(DoradeBlockSswb::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockSswb();
	    }else if(DoradeBlockVold::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockVold();
	    }else if(DoradeBlockRadd::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockRadd();
	    }else if(DoradeBlockParm::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockParm();
	    }else if(DoradeBlockCelv::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockCelv();
	    }else if(DoradeBlockCsfd::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockCsfd();
	    }else if(DoradeBlockFrib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockFrib();
	    }else if(DoradeBlockCfac::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockCfac();
	    }else if(DoradeBlockSwib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockSwib();
	    }else if(DoradeBlockRyib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockRyib();
	    }else if(DoradeBlockAsib::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockAsib();
	    }else if(DoradeBlockXstf::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockXstf();
	    }else if(DoradeBlockRdat::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockRdat();
	    }else if(DoradeBlockNull::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockNull();
	    }else if(DoradeBlockRktb::test(buffer)){
		returnBlock = (DoradeBlock *) new DoradeBlockRktb();
	    }else{
		returnBlock = (DoradeBlock *) new DoradeBlockUnknown();
	    }

	    returnBlock->decode(buffer);
	}

    }catch(Fault &re){
	re.add_msg("DoradeFile::read_next_block: caught Fault\n");
	throw re;
    }catch(...){
	throw Fault("DoradeFile::read_next_block: caught exception\n" );
    }


    return returnBlock;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_headers() throw (Fault){

    if(file_ == NULL){
	throw Fault("DoradeFile::read_headers: file not opened. \n");
    }

    if(newFile_ == true){
	throw Fault("DoradeFile::read_headers: file was opened for writing. \n");
    }

    // read start_time

    DoradeBlock      *block;
    DoradeBlockSswb  *local_sswb(NULL);
    DoradeBlockVold  *local_vold(NULL);
    DoradeBlockRadd  *local_radd(NULL);
    DoradeBlockParm  *local_parm(NULL);
    DoradeBlockCelv  *local_celv(NULL);
    DoradeBlockCsfd  *local_csfd(NULL);
    DoradeBlockSwib  *local_swib(NULL);
    DoradeBlockRyib  *local_ryib(NULL);

    int blockCount(0);

    try {

	reset_file();

	while(((block = read_next_block()) != NULL) && (blockCount < 30)){

          if((local_sswb = block->castToDoradeBlockSswb())){
		sswb_ = local_sswb;
	    }else if((local_vold = block->castToDoradeBlockVold())){
		vold_ = local_vold;
	    }else if((local_radd = block->castToDoradeBlockRadd())){
		radd_ = local_radd;
	    }else if((local_parm = block->castToDoradeBlockParm())){
		parmVector_.push_back(local_parm);
	    }else if((local_celv = block->castToDoradeBlockCelv())){
		celv_ = local_celv;
	    }else if((local_csfd = block->castToDoradeBlockCsfd())){
		csfd_ = local_csfd;
	    }else if((local_swib = block->castToDoradeBlockSwib())){
		swib_ = local_swib;
	    }else if((local_ryib = block->castToDoradeBlockRyib())){
		// End of header blocks
		delete local_ryib;
		break;
	    }else{
		delete block;
	    }

	    blockCount++;
	}

    }catch (Fault &re){
	re.add_msg("DoradeFile::read_headers : Caught Fault.\n");
	throw(re);
    }

    // Make sure that all of the header blocks were found.

    if (sswb_ == NULL){
	throw Fault("DoradeFile::read_headers : File is missing SSWB block. \n");
    }

    if (vold_  == NULL) {
	throw Fault("DoradeFile::read_headers : File is missing VOLD block. \n");
    }

    if(radd_ == NULL){
	throw Fault("DoradeFile::read_headers : File is missing RADD block. \n");
    }

    if(parmVector_.size() == 0){
	throw Fault("DoradeFile::read_headers : File has no PARM blocks.\n");
    }

    if((celv_ == NULL) && (csfd_ == NULL)){
	throw Fault("DoradeFile::read_headers : File is missing either a CELV or a CSFD block. \n");
    }

    if(swib_ == NULL){
	throw Fault("DoradeFile::read_headers : File is missing SWIB block. \n");
    }

    // Decode Header values
    try {
	read_sswb(*sswb_);

	read_vold(*vold_);

	read_radd(*radd_);

	int numberOfParms = parmVector_.size();     
	integerValues_["number_of_fields"] = numberOfParms;  // Many files don't have this set in sswb.
	for(int index=0; index < numberOfParms; index++){
	    read_parm(*(parmVector_[index]),index);
	}

	// cell spacing
	if(celv_ != NULL){
	    integerValues_["cell_spacing_method"] = RayConst::cellSpacingByVector;
	    read_celv(*celv_);
	}else if(csfd_ != NULL){
	    integerValues_["cell_spacing_method"] = RayConst::cellSpacingBySegment;
	    read_csfd(*csfd_);
	}else{
	    throw Fault("DoradeFile::read_headers: no cell spacing information.\n");
	}

	read_swib(*swib_);

	set_missing_values();

    }catch(Fault &re){
	re.add_msg("DoradeFile::read_headers: Caught Fault \n");
	throw(re);
    }

    headers_read_ = true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_sswb(DoradeBlockSswb &sswb)   throw(Fault){

    try {
	integerValues_ ["number_of_fields"] = sswb.get_integer("number_of_fields");
	integerValues_ ["size_of_file"]     = sswb.get_integer("size_of_file");

	// start time set in vold block
	raycTimeValues_["stop_time"]        = RaycTime(sswb.get_integer("stop_time"));
	
	
    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_sswb: Caught Fault.");
	throw fault;
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_vold(DoradeBlockVold &vold)   throw(Fault){

    try {
	
	int year,mon,day,hour,min,sec;
	RaycTime rt;

	year = vold.get_integer("year");
	mon  = vold.get_integer("month");
	day  = vold.get_integer("day");
	hour = vold.get_integer("data_set_hour");
	min  = vold.get_integer("data_set_minute");
	sec  = vold.get_integer("data_set_second");

	startYear_  = year;
	startMonth_ = mon;
	startDay_   = day;

	rt.set(year,mon,day,hour,min,sec,0);

	raycTimeValues_["start_time"]    = rt;
	integerValues_ ["volume_number"] = vold.get_integer("volume_number");
	stringValues_  ["project_name"]  = vold.get_string ("project_name");

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_vold: Caught Fault.");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_radd(DoradeBlockRadd &radd)   throw(Fault){

    try {
	stringValues_ ["platform_name"]         = radd.get_string ("radar_name");
	doubleValues_ ["radar_constant"]        = radd.get_double ("radar_constant");
	doubleValues_ ["peak_power"]            = radd.get_double ("peak_power");
	doubleValues_ ["noise_power"]           = radd.get_double ("noise_power");
	doubleValues_ ["receiver_gain"]         = radd.get_double ("receiver_gain");
	doubleValues_ ["antenna_gain"]          = radd.get_double ("antenna_gain");
	doubleValues_ ["system_gain"]           = radd.get_double ("system_gain");
	doubleValues_ ["horizontal_beam_width"] = radd.get_double ("horizontal_beam_width");
	doubleValues_ ["vertical_beam_width"]   = radd.get_double ("vertical_beam_width");
	integerValues_["platform_type"]         = radd.get_integer("radar_type");
	integerValues_["scan_mode"]             = radd.get_integer("scan_mode");	
	doubleValues_ ["platform_longitude"]    = radd.get_double ("radar_longitude");
	doubleValues_ ["platform_latitude"]     = radd.get_double ("radar_latitude");
	doubleValues_ ["platform_altitude"]     = radd.get_double ("radar_altitude") * 1000.0;
	doubleValues_ ["nyquist_velocity"]      = radd.get_double ("eff_unamb_vel");
	doubleValues_ ["unambiguous_range"]     = radd.get_double ("eff_unamb_range_km") * 1000.0;
	
	int numberOfFrequencies                 = radd.get_integer("number_of_frequencies");
	integerValues_["number_of_frequencies"] = numberOfFrequencies;
	for(int index = 0; index < numberOfFrequencies; index++){
	    set_double("frequency",index,radd.get_double("frequency",index));
	}

	int numberOfIpps                        = radd.get_integer("number_of_ipps");
	integerValues_["number_of_prfs"]        = numberOfIpps;
	for(int index = 0; index < numberOfIpps; index++){
	    set_double("pulse_repetition_frequency",index,1000.0/radd.get_double("interpulse_period",index));
	}

	if(radd.get_integer("block_size") == 300){
	    doubleValues_ ["pulse_width"]           = radd.get_double ("pulse_width") / 1000000.0;
	    doubleValues_ ["band_width"]            = 1.0/doubleValues_["pulse_width"];
	}
	

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_radd: Caught Fault.");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_parm(DoradeBlockParm &parm, const int index)  throw(Fault){

    try {

	string units   = parm.get_string("param_units");
	int    unitsId = (int)RayConst::unitsUnknown;
	if(units == "none"){
	    unitsId = (int)RayConst::unitsNone;
	}else if(units == "dBz"){
	    unitsId = (int)RayConst::unitsReflectivity;
	}else if(units == "dBm"){
	    unitsId = (int)RayConst::unitsPower;
	}else if(units == "m/s"){
	    unitsId = (int)RayConst::unitsVelocity;
	}

	set_string ("field_name"        ,index, parm.get_string ("field_name"));
	set_string ("field_long_name"   ,index, parm.get_string ("param_description"));
	set_integer("field_units"       ,index, unitsId);
	set_integer("field_polarization",index, parm.get_integer("polarization"));
	set_integer("number_of_samples" ,       parm.get_integer("number_of_samples"));  
	set_integer("binary_format"     ,index, parm.get_integer("binary_format"));

	double doradeScale = parm.get_double("parameter_scale");
	double doradeBias  = parm.get_double("parameter_bias");
	double forayScale  = 1.0/doradeScale;
	double forayBias   = -1.0 * doradeBias / doradeScale;
	set_double ("parameter_scale"   ,index, forayScale);
	set_double ("parameter_bias"    ,index, forayBias);

	set_integer("bad_data"          ,index, parm.get_integer("bad_data"));        
	set_integer("field_polarization",index, (int)RayConst::unknownPolarization);

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_parm: Caught Fault.");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_csfd(DoradeBlockCsfd &csfd)  throw(Fault){

    try {

	int numberOfSegments                        = csfd.get_integer("number_of_cell_segments");
	integerValues_["number_of_cell_segments"]   = numberOfSegments;
	doubleValues_ ["meters_to_first_cell"]      = csfd.get_double ("meters_to_first_cell");
	
	for(int index=0; index < numberOfSegments; index++){
	    set_double ("segment_cell_spacing" ,index, csfd.get_double("spacing",index));	    
	    set_integer("segment_cell_count"   ,index, csfd.get_integer("segment_cell_count",index));	    
	}

	integerValues_["number_of_cells"]     = csfd.get_integer("number_of_cells");

	cellVector_                           = csfd.getDoubleVector();

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_csfd: Caught Fault.");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_celv(DoradeBlockCelv &celv)  throw(Fault){

    try {

	// number_of_cells
	integerValues_["number_of_cells"     ] = celv.get_integer("number_of_cells");
	doubleValues_ ["meters_to_first_cell"] = celv.get_double ("meters_to_first_cell");
	// Cell spacing vector
	cellVector_ = celv.getDoubleVector();

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_celv: Caught Fault.");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::read_swib(DoradeBlockSwib &swib)  throw(Fault){

    try {
	
	integerValues_["number_of_rays"] = swib.get_integer("num_rays");
	integerValues_["sweep_number"]   = swib.get_integer("sweep_num");

	double fixed_angle = swib.get_double("fixed_angle");
	raycAngleValues_["fixed_angle"]  = RaycAngle(fixed_angle);
	
	double start_angle = swib.get_double("start_angle");
	raycAngleValues_["start_angle"]  = RaycAngle(start_angle);

	double stop_angle = swib.get_double("stop_angle");
	raycAngleValues_["stop_angle"]   = RaycAngle(stop_angle);

    }catch(Fault &fault){
	fault.add_msg("DoradeFile::read_swib: Caught Fault.");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::set_missing_values(){

    booleanValues_["test_pulse_present"]       = false;
    doubleValues_ ["transmitter_power"]        = RayConst::badDouble;
    doubleValues_ ["test_pulse_power" ]        = RayConst::badDouble;
    doubleValues_ ["test_pulse_start_range"]   = RayConst::badDouble;
    doubleValues_ ["test_pulse_end_range"]     = RayConst::badDouble;

    booleanValues_["calibration_data_present"] = false;

    // Need to add missing values to caliabration variables.
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeFile::find_first_ray() throw (Fault){

    reset_file();
    
    bool return_value;

    try{
        return_value = find_next_ray();
    }catch(Fault &re){
	re.add_msg("DoradeFile::find_first_ray : Caught Fault.\n");
	throw re;
    }

    return return_value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeFile::find_next_ray() throw (Fault){
    
    DoradeBlockRyib *ryib(NULL);
    DoradeBlock     *block(NULL);

    if(headers_read_ == false){
	char msg[2048];
	sprintf(msg,"DoradeFile::find_next_ray : headers not read, need to call read_headers\n");
		throw Fault(msg);
    }

    const int oneDay(86400);  // One day worth of seconds.

    try {
	while((block = read_next_block()) != NULL){

          if((ryib = block->castToDoradeBlockRyib())){
		// Found RYIB
		break;
	    }else{
		// Did not find RYIB
		delete block;
	    }
	}

	if(block == NULL){
	    // Must have reached the end of file.
	    return false;
	}

	// Azimuth and Elevation angles and othere ray specific values

	raycAngleValues_["ray_azimuth"]        = RaycAngle(ryib->get_double("azimuth"));
	raycAngleValues_["ray_elevation"]      = RaycAngle(ryib->get_double("elevation"));
	doubleValues_   ["ray_peak_power"]     = ryib->get_double          ("peak_power");
	doubleValues_   ["ray_true_scan_rate"] = ryib->get_double          ("true_scan_rate");
	integerValues_  ["ray_status"]         = ryib->get_integer         ("ray_status");


	// Time
	int hour        = ryib->get_integer("hour");
	int minute      = ryib->get_integer("minute");
	int second      = ryib->get_integer("second");
	int millisecond = ryib->get_integer("millisecond");

	RaycTime thisRayTime;
	thisRayTime.set(startYear_,startMonth_,startDay_,
			hour,minute,second,millisecond * 1000000);

	if(raycTimeValues_.count("ray_time") != 0){
	    RaycTime lastRayTime = raycTimeValues_["ray_time"];
	    if (thisRayTime < lastRayTime){
		// Times of rays crossed midnight.
		thisRayTime += oneDay;
	    }
	}
	raycTimeValues_["ray_time"] = thisRayTime;

	// Data
	int numberOfFields = integerValues_["number_of_fields"];
	int fieldCount(0);

	// clear out data from last ray.
	clear_rdatVector();
	if(xstf_ != NULL){
	    delete xstf_;
	    xstf_ = NULL;
	}

	DoradeBlockRyib *ryib2(NULL);
	DoradeBlockRdat *rdat(NULL);
	DoradeBlockXstf *xstf(NULL);
	while((fieldCount < numberOfFields) && ((block = read_next_block()) != NULL)){

          if((ryib2 = block->castToDoradeBlockRyib())){
		// Found an RYIB block; this is an error.
		delete ryib2;
		char msg[2048];
		sprintf(msg,"DoradeFile::find_next_ray : Found only %d RDAT blocks, should be %d RDAT blocks.\n",
			fieldCount + 1,
			numberOfFields);
		throw Fault(msg);
	    }

          if((rdat = block->castToDoradeBlockRdat())){
		rdatVector_.push_back(rdat);
		fieldCount++;
	    }else if((xstf = block->castToDoradeBlockXstf()) != NULL){
		// Found xstf block
		xstf_ = xstf;
	    }else{
		delete block;
	    }
	}

    }catch(Fault &re){
	re.add_msg("DoradeFile::find_next_ray : caught Fault.\n");
	throw(re);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//

//
//////////////////////////////////////////////////////////////////////
vector<double> DoradeFile::get_cell_vector() throw (Fault){

    if(cellVector_.size() == 0){
	throw Fault("DoradeFile::get_cell_vector : cellVector is of size 0.\n");
    }

    return cellVector_;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::set_cell_spacing_vector(vector<double> &vector) throw (Fault){

    validate_integer("DoradeFile::set_cell_spacing_vector","number_of_cells");
    
    if(vector.size() != get_integer("number_of_cells")){
	char msg[2048];
	sprintf(msg,"DoradeFile::set_cell_spacing_vector : number_of_cells not equal to vector size: %d != %d \n",
		get_integer("number_of_cells"),
		(int) vector.size());
	throw Fault(msg);
    }

    cellVector_ = vector;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int DoradeFile::get_field_index(string fieldName) throw (Fault){
    
    if(headers_read_ == false){
	char msg[2048];
	sprintf(msg,"DoradeFile::get_field_index : headers not read, need to call read_headers\n");
	throw Fault(msg);
    }
    
    try {
	int numberOfFields = get_integer("number_of_fields");

	for(int field = 0; field < numberOfFields; field++){
	    string indexedFieldName = get_string("field_name",field);
	    if(indexedFieldName ==  fieldName){
		return field;
	    }
	}

    }catch (Fault &re){
	char msg[4096];
	sprintf(msg,"DoradeFile::get_field_index : Caught Fault. \n");
	re.add_msg(msg);
	throw re;
    }

    // If we get here throw Fault.
    char msg[4096];
    sprintf(msg,"DoradeFile::get_field_index : No field named %s. \n",
	    fieldName.c_str());
    throw Fault(msg);
    
    return -1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeFile::test_for_field(string fieldName) throw (Fault){
    
    if(headers_read_ == false){
	char msg[2048];
	sprintf(msg,"DoradeFile::test_for_field : headers not read, need to call read_headers\n");
	throw Fault(msg);
    }
    
    try {
	int numberOfFields = get_integer("number_of_fields");

	for(int field = 0; field < numberOfFields; field++){
	    string indexedFieldName = get_string("field_name",field);
	    if(indexedFieldName ==  fieldName){
		return true;
	    }
	}

    }catch (Fault &re){
	char msg[4096];
	sprintf(msg,"DoradeFile::test_for_field : Caught Fault. \n");
	re.add_msg(msg);
	throw re;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::clear_rdatVector(){

    int numberOfRdats = rdatVector_.size();
    
    for(int aa = 0; aa < numberOfRdats; aa++){
	delete rdatVector_[aa];
    }

    rdatVector_.clear();
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::get_ray_data(int fieldIndex, RayIntegers *ri) throw (Fault){

    
    int numberOfFields = rdatVector_.size();

    if((fieldIndex < 0) || (fieldIndex >= numberOfFields)){
	char msg[2048];
	sprintf(msg,"DoradeFile::get_ray_data (RayIntegers) : field index of %d is invalid \n",
		fieldIndex);
	throw Fault(msg);
    }


    try {
	int binaryFormat = get_integer("binary_format",fieldIndex);

	DoradeBlockRdat *rdat = rdatVector_[fieldIndex];

	if(rdat->dataIsInteger(binaryFormat)){
	    rdat->decodeIntegerData(binaryFormat,ri);
	}else{
	    char msg[2048];
	    sprintf(msg,"DoradeFile::get_ray_data (RayIntegers) : field index of %d contains float data.\n",
		    fieldIndex);
	    throw Fault(msg);
	}
    }catch (Fault &re){
	re.add_msg("DoradeFile::get_ray_data (RayIntegers) : Caught Fault.\n");
	throw(re);
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void DoradeFile::get_ray_data(int fieldIndex, RayDoubles *rd) throw (Fault){

    int numberOfFields = rdatVector_.size();

    RayIntegers ri;

    if((fieldIndex <0) || (fieldIndex >= numberOfFields)){
	char msg[2048];
	sprintf(msg,"DoradeFile::get_ray_data (RayDoubles) : field index of %d is invalid \n",
		fieldIndex);
	throw Fault(msg);
    }

    try {
	int binaryFormat = get_integer("binary_format",fieldIndex);
	
	if(rdatVector_[fieldIndex]->dataIsInteger(binaryFormat)){
	    rdatVector_[fieldIndex]->decodeIntegerData(binaryFormat,&ri);

	    double scale = get_double("parameter_scale",fieldIndex);
	    double bias  = get_double("parameter_bias",fieldIndex);

	    int numberOfIndexes = ri.size();
	    rd->clear();

	    for(int index = 0;index < numberOfIndexes; index++){
		rd->push_back(((double)ri[index] * scale) + bias);
	    }

	}else{
	    throw Fault("DoradeFile::get_ray_data (RayDoubles) : Float decoding not implemented yet.");
	}

    }catch (Fault &re){
	re.add_msg("DoradeFile::get_ray_data (RayDoubles) : Caught Fault.\n");
	throw(re);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
const unsigned char * DoradeFile::get_xstf_data(){

    if(xstf_ == NULL){
	return NULL;
    }

    return xstf_->content();
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool DoradeFile::test_big_endian() throw (Fault){

    if(file_ == NULL){
	char msg[1024];
	sprintf(msg,"DoradeFile::test_big_endian: file not opened.\n");
		throw Fault(msg);
    }

    unsigned char buffer[6];
    
    if(fread(buffer,6, 1,file_) != 1){
	char msg[1024];
	sprintf(msg,"DoradeFile::test_big_endian: read failure: %s\n",
		strerror(errno));
		
	    throw Fault(msg);
    }
    fseek(file_,0,SEEK_SET);

    if((buffer[4] == 0) && (buffer[5] == 0)){
	return true;
    }

    return false;
}


