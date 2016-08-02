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

#include <cmath>
#include "FilePath.h"
#include "NcRadarFile.h"
using namespace std;
using namespace ForayUtility;

const double NcRadarFile::LIGHTSPEED_ = 299792458.0;


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
NcRadarFile::NcRadarFile(){

    ncFileId_        = -1;
    currentRayIndex_ = -1;


}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
NcRadarFile::~NcRadarFile(){
    indexToVariableId_.clear();  // don't leak memory
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::open_file(const string &filename, const bool newFile) throw(Fault) {

    int  status;

    if(newFile){
        // Create a new file.
        status = nc_create(filename.c_str(),0,&ncFileId_);
        if(status != NC_NOERR){
            char statusMessage[2048];
            sprintf(statusMessage,"NcRadarFile::open_file(%s,true): nc_create returned error: %s.\n",
                    filename.c_str(),
                    nc_strerror(status));
            throw Fault(statusMessage);
        }
        writeFile_ = true;
    }else{
        // Read an existing file.
        status = nc_open(filename.c_str(),NC_NOWRITE,&ncFileId_);
        if(status != NC_NOERR){
            char statusMessage[2048];
            sprintf(statusMessage,"NcRadarFile::open_file(%s,false): nc_open returned error: %s.\n",
                    filename.c_str(),
                    nc_strerror(status));
            throw Fault(statusMessage);
        }
        writeFile_ = false;
    }


    FilePath filePath;
    filePath.file(filename);
    
    set_string("file_name"     , filePath.get_name());
    set_string("directory_name", filePath.get_directory());

    set_string("producer_name" ,"NCAR EOL");

    currentRayIndex_ = -1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::close_file() {

    if(ncFileId_ >= 0){
        nc_close(ncFileId_);
        indexToVariableId_.clear();  // don't leak memory
    }
    ncFileId_        = -1;
    currentRayIndex_ = -1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::reset_file() {

    currentRayIndex_ = -1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::read_headers() throw(Fault) {

    if(ncFileId_ < 0){
        char statusMessage[2048];
        sprintf(statusMessage,"NcRadarFile::read_headers: no open file.\n");
        throw Fault(statusMessage);
    }

    try {

        int numberOfRays = get_nc_dimension_length("Time");
        set_integer("number_of_rays",numberOfRays);
    
        double startTimeValue  = get_nc_double("volume_start_time");
        double firstTimeOffset = get_nc_double("time_offset",0);
        set_time("start_time",RaycTime(startTimeValue + firstTimeOffset));

	double lastTimeOffset = get_nc_double("time_offset",numberOfRays - 1);
        set_time("stop_time",RaycTime(startTimeValue + lastTimeOffset));
	
        process_field_info();

        double fixedAngle      = get_nc_double("Fixed_Angle");
        set_angle("fixed_angle",RaycAngle(fixedAngle));


        // Assume Cell Spacing by Segment
        int method = RayConst::cellSpacingBySegment;
        set_integer("cell_spacing_method",method );
        set_integer("number_of_cell_segments",1);

        double metersToFirstCell = get_nc_double("Range_to_First_Cell");
        set_double("meters_to_first_cell",metersToFirstCell);

        double cellSpacing = get_nc_double("Cell_Spacing");
        set_double("segment_cell_spacing",0,cellSpacing);

        int maxCells = get_nc_dimension_length("maxCells");
        set_integer("segment_cell_count",0,maxCells);
        set_integer("number_of_cells",maxCells);

        double nyquistVelocity = get_nc_double("Nyquist_Velocity");
        set_double("nyquist_velocity",nyquistVelocity);

        double uRange = get_nc_double("Unambiguous_Range");
        set_double("unambiguous_range",uRange);

        double latitude = get_nc_double("Latitude");
        set_double("platform_latitude",latitude);

        double longitude = get_nc_double("Longitude");
        set_double("platform_longitude",longitude);

        double altitude = get_nc_double("Altitude");
        set_double("platform_altitude",altitude);

        // Assume only one system; Assume that numSystems dimension is set to 1.
        double radarConstant = get_nc_double("Radar_Constant");
        set_double("radar_constant",radarConstant);

        double receiverGain = get_nc_double("rcvr_gain");
        set_double("receiver_gain",receiverGain);

        double antennaGain = get_nc_double("ant_gain");
        set_double("antenna_gain",antennaGain);

        double systemGain = get_nc_double("sys_gain");
        set_double("system_gain",systemGain);

        double beamWidth = get_nc_double("bm_width");
        set_double("horizontal_beam_width",beamWidth);
        set_double("vertical_beam_width"  ,beamWidth);

        double pulseWidth = get_nc_double("pulse_width");
        set_double("pulse_width",pulseWidth);

        double bandWidth = get_nc_double("band_width");
        set_double("band_width",bandWidth);

        double peakPower = get_nc_double("peak_pwr");
        set_double("peak_power",peakPower);

        if(test_for_variable("xmtr_pwr")){
            double transmitterPower = get_nc_double("xmtr_pwr");
            set_double("transmitter_power",transmitterPower);
        }else if(test_for_variable("xmtr_power")){
            double transmitterPower = get_nc_double("xmtr_power");
            set_double("transmitter_power",transmitterPower);
        }else{
            throw Fault("NcRadarFile::read_headers: Can not find a variable for transmitting power.\n");
        }

        double noisePower = get_nc_double("noise_pwr");
        set_double("noise_power",noisePower);

	set_boolean("test_pulse_present",true);  // Always true for NcRadar.

        double testPulsePower = get_nc_double("tst_pls_pwr");
        set_double("test_pulse_power",testPulsePower);

        double testPulseStartRange = get_nc_double("tst_pls_rng0");
        set_double("test_pulse_start_range",testPulseStartRange);

        double testPulseEndRange = get_nc_double("tst_pls_rng1");
        set_double("test_pulse_end_range",testPulseEndRange);

        double wavelength = get_nc_double("Wavelength");
        set_integer("number_of_frequencies",1);
        set_double("frequency",0, LIGHTSPEED_ / wavelength);

        double prf = get_nc_double("PRF");
        set_integer("number_of_prfs",1);
        set_double("pulse_repetition_frequency",0,prf);


	int calibrationDataPresent = get_nc_integer("calibration_data_present");

	if(calibrationDataPresent == 0){
	    set_boolean("calibration_data_present",false);

	}else{

	    set_boolean("calibration_data_present",true);
	    
	    set_double("horizontal_antenna_gain_db"              ,get_nc_double("ant_gain_h_db"));                             
	    set_double("vertical_antenna_gain_db"                ,get_nc_double("ant_gain_v_db"));                               
	    set_double("horizontal_transmitter_power_dbm"        ,get_nc_double("xmit_power_h_dbm"));                    
	    set_double("vertical_transmitter_power_dbm"          ,get_nc_double("xmit_power_v_dbm"));                   
	    set_double("horizontal_two_way_waveguide_loss_db"    ,get_nc_double("two_way_waveguide_loss_h_db"));
	    set_double("vertical_two_way_waveguide_loss_db"      ,get_nc_double("two_way_waveguide_loss_v_db"));    
	    set_double("horizontal_two_way_radome_loss_db"       ,get_nc_double("two_way_radome_loss_h_db"));         
	    set_double("vertical_two_way_radome_loss_db"         ,get_nc_double("two_way_radome_loss_v_db"));          
	    set_double("receiver_mismatch_loss_db"               ,get_nc_double("receiver_mismatch_loss_db"));                
	    set_double("horizontal_radar_constant"               ,get_nc_double("radar_constant_h"));               
	    set_double("vertical_radar_constant"                 ,get_nc_double("radar_constant_v"));                          
	    set_double("horizontal_co_polar_noise_dbm"           ,get_nc_double("noise_hc_dbm"));                    
	    set_double("vertical_co_polar_noise_dbm"             ,get_nc_double("noise_vc_dbm"));                          
	    set_double("horizontal_cross_polar_noise_dbm"        ,get_nc_double("noise_hx_dbm"));                     
	    set_double("vertical_cross_polar_noise_dbm"          ,get_nc_double("noise_vx_dbm"));                       
	    set_double("horizontal_co_polar_receiver_gain_db"    ,get_nc_double("receiver_gain_hc_db"));
	    set_double("vertical_co_polar_receiver_gain_db"      ,get_nc_double("receiver_gain_vc_db"));            
	    set_double("horizontal_cross_polar_receiver_gain_db" ,get_nc_double("receiver_gain_hx_db"));       
	    set_double("vertical_cross_polar_receiver_gain_db"   ,get_nc_double("receiver_gain_vx_db"));         
	    set_double("horizontal_co_polar_base_dbz_at_1km"     ,get_nc_double("base_1km_hc_dbz"));           
	    set_double("vertical_co_polar_base_dbz_at_1km"       ,get_nc_double("base_1km_vc_dbz"));                 
	    set_double("horizontal_cross_polar_base_dbz_at_1km"  ,get_nc_double("base_1km_hx_dbz"));            
	    set_double("vertical_cross_polar_base_dbz_at_1km"    ,get_nc_double("base_1km_vx_dbz"));              
	    set_double("horizontal_co_polar_sun_power_dbm"       ,get_nc_double("sun_power_hc_dbm"));                 
	    set_double("vertical_co_polar_sun_power_dbm"         ,get_nc_double("sun_power_vc_dbm"));                  
	    set_double("horizontal_cross_polar_sun_power_dbm"    ,get_nc_double("sun_power_hx_dbm"));             
	    set_double("vertical_cross_polar_sun_power_dbm"      ,get_nc_double("sun_power_vx_dbm"));               
	    set_double("horizontal_noise_source_power_dbm"       ,get_nc_double("noise_source_power_h_dbm"));
	    set_double("vertical_noise_source_power_dbm"         ,get_nc_double("noise_source_power_v_dbm"));          
	    set_double("horizontal_power_measurement_loss_db"    ,get_nc_double("power_measure_loss_h_db"));     
	    set_double("vertical_power_measurement_loss_db"      ,get_nc_double("power_measure_loss_v_db"));        
	    set_double("horizontal_coupler_forward_loss_db"      ,get_nc_double("coupler_forward_loss_h_db"));        
	    set_double("vertical_coupler_forward_loss_db"        ,get_nc_double("coupler_forward_loss_v_db"));        
	    set_double("zdr_correction_db"                       ,get_nc_double("zdr_correction_db"));                       
	    set_double("horizontal_ldr_correction_db"            ,get_nc_double("ldr_correction_h_db"));                    
	    set_double("vertical_ldr_correction_db"              ,get_nc_double("ldr_correction_v_db"));                    
	    set_double("system_phidp_degrees"                    ,get_nc_double("system_phidp_deg"));                          
	}
	
        string radarName       = get_nc_string_attribute("Instrument_Name");
        set_string("platform_name",radarName);

        string radarType       = get_nc_string_attribute("Instrument_Type");
        set_integer("platform_type",process_instrument_type(radarType));

        string scanMode        = get_nc_string_attribute("Scan_Mode");
        int    scanModeInteger = process_scan_mode(scanMode);
        set_integer("scan_mode",scanModeInteger);

        int numberOfSamples = get_nc_integer_attribute(NC_GLOBAL,"Num_Samples");
        set_integer("number_of_samples",numberOfSamples);
	
        int    sweepNumber = get_nc_integer_attribute(NC_GLOBAL,"Scan_Number");
        set_integer("sweep_number",sweepNumber);

        int    volumeNumber = get_nc_integer_attribute(NC_GLOBAL,"Volume_Number");
        set_integer("volume_number",volumeNumber);
	
        string projectName = get_nc_string_attribute("Project_Name");
        set_string("project_name",projectName);

        double startAngle;
        double stopAngle;
	
        if(scanModeInteger == RayConst::scanModeRHI){
            startAngle = get_nc_double("Elevation",0);
            stopAngle  = get_nc_double("Elevation",numberOfRays - 1);
        }else{
            startAngle = get_nc_double("Elevation",0);
            stopAngle  = get_nc_double("Elevation",numberOfRays - 1);
        }
	
        set_angle("start_angle",RaycAngle(startAngle));
        set_angle("stop_angle" ,RaycAngle(stopAngle));

    }catch (Fault &fault){
        fault.add_msg("NcRadarFile::read_headers: caught Fault. \n");
        throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool NcRadarFile::find_first_ray() throw(Fault) {

    currentRayIndex_ = -1;

    bool return_value;

    try{
        return_value = find_next_ray();
    }catch(Fault &re){
        re.add_msg("NcRadarFile::find_first_ray : Caught Fault.\n");
        throw re;
    }

    return return_value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool NcRadarFile::find_next_ray() throw(Fault) {

    int numberOfRays;

    try {
        numberOfRays = get_integer("number_of_rays");
    }catch (Fault &fault){
        fault.add_msg("NcRadarFile::find_next_ray: caught Fault. \n");
        throw fault;
    }

    currentRayIndex_++;

    if(currentRayIndex_ >= numberOfRays){
        return false;
    }

    try {
        double startTimeValue  = get_nc_double("volume_start_time");
        double timeOffset      = get_nc_double("time_offset",currentRayIndex_);
        double azimuth         = get_nc_double("Azimuth"    ,currentRayIndex_);
        double elevation       = get_nc_double("Elevation"  ,currentRayIndex_);
        int    status          = RayConst::rayStatusNormal;  
        double peakPower       = get_nc_double("peak_pwr");

        set_time   ("ray_time"          ,RaycTime(startTimeValue + timeOffset));
        set_angle  ("ray_azimuth"       ,RaycAngle(azimuth));
        set_angle  ("ray_elevation"     ,RaycAngle(elevation));
        set_integer("ray_status"        ,status);
        set_double ("ray_peak_power"    ,peakPower);
        set_double ("ray_true_scan_rate",RayConst::badDouble);

    }catch(Fault &fault){
        fault.add_msg("NcRadarFile::find_next_ray: caught Fault.\n");
        throw(fault);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
vector<double> NcRadarFile::get_cell_vector() throw(Fault) {

    // Create the cell vector.  Assume segment method and that there
    // is only one segment.
    vector<double> cellVector;
    int            numberOfCells;
    double         currentRange;
    double         cellSpacing;

    try {
        numberOfCells = get_integer("number_of_cells");
        currentRange  = get_double ("meters_to_first_cell");
        cellSpacing   = get_double ("segment_cell_spacing",0);

        cellVector.reserve(numberOfCells);
	
        for(int index = 0; index < numberOfCells; index++){
            cellVector.push_back(currentRange);
            currentRange += cellSpacing;
        }
	
    }catch(Fault &fault){
        fault.add_msg("NcRadarFile::get_cell_vector : cault Fault. \n");
        throw fault;
    }

    return cellVector;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void  NcRadarFile::set_cell_spacing_vector(vector<double> &) throw(Fault) {

    throw Fault("NcRadarFile::set_cell_spacing_vector: Not implemented yet.\n");
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int  NcRadarFile::get_field_index(string fieldName) throw(Fault) {

    try {
        int numberOfFields = get_integer("number_of_fields");

        for(int index = 0; index < numberOfFields; index++){
            string indexedFieldName = get_string("field_name",index);
            if(indexedFieldName ==  fieldName){
                return index;
            }
        }

    }catch (Fault &re){
        char msg[4096];
        sprintf(msg,"DoradeFile::get_field_index : Caught Fault. \n");
        re.add_msg(msg);
        throw re;
    }

    // If we get to here then no field index was found.
    char msg[4096];
    sprintf(msg,"NcRadarFile::get_field_index: No field name of %s.\n",fieldName.c_str());
    throw Fault(msg);
    
    return -1;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool  NcRadarFile::test_for_field(string fieldName) throw(Fault) {

    try {
        int numberOfFields = get_integer("number_of_fields");

        for(int index = 0; index < numberOfFields; index++){
            string indexedFieldName = get_string("field_name",index);
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
void NcRadarFile::get_ray_data(int index, RayIntegers *ri) throw(Fault) {

    int numberOfFields;
    int binaryFormat;
    int numberOfCells;

    try{
        numberOfFields = get_integer("number_of_fields");
        if((index < 0) || (index >= numberOfFields)){
            char message[2048];
            sprintf(message,"NcRadarFile::get_ray_data(RayIntegers): field index of %d is out of bounds.\n",
                    index);
            throw Fault(message);
        }

        binaryFormat = get_integer("binary_format",index);
        numberOfCells = get_integer("number_of_cells");  

        ri->clear();
        ri->reserve(numberOfCells);

        size_t start[2];
        start[0] = currentRayIndex_;
        start[1] = 0;
	
        size_t count[2];
        count[0] = 1;
        count[1] = numberOfCells;

        int variableId = indexToVariableId_[index];

        if((binaryFormat == RayConst::binaryFormat1ByteInt) ||
           (binaryFormat == RayConst::binaryFormat2ByteInt)){

            int *ncData = new int[numberOfCells];

            int status = nc_get_vara_int(ncFileId_,variableId,start,count,ncData);
            if(status != NC_NOERR){
                char statusMessage[2048];
                sprintf(statusMessage,"NcRadarFile::get_ray_data(RayIntegers): nc_inq_vara_int: returned error: %s.\n",
                        nc_strerror(status));
                throw Fault(statusMessage);
            }

            for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                ri->push_back(ncData[cellIndex]);
            }

            delete [] ncData;

        }else if(binaryFormat == RayConst::binaryFormat4ByteFloat){

            double bias  = get_double("parameter_bias" ,index);
            double scale = get_double("parameter_scale",index);

            double *ncData = new double[numberOfCells];

            int status = nc_get_vara_double(ncFileId_,variableId,start,count,ncData);
            if(status != NC_NOERR){
                char statusMessage[2048];
                sprintf(statusMessage,"NcRadarFile::get_ray_data(RayIntegers): nc_inq_vara_double: returned error: %s.\n",
                        nc_strerror(status));
                throw Fault(statusMessage);
            }

            for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){

                double value = ncData[cellIndex];

		double scaled = (value - bias) / scale;

                if(scaled >= 0.0){
                    ri->push_back((int)(scaled + 0.5));
		}else{
                    ri->push_back((int)(scaled - 0.5));
                }
            }
            delete [] ncData;

        }else{
            char message[2048];
            sprintf(message,"NcRadarFile::get_ray_data(RayIntegers): binaryForamt of %d for field index of %d is invalid.\n ",
                    index,
                    binaryFormat);
            throw Fault(message);
        }

    }catch (Fault &fault){
        fault.add_msg("NcRadarFile::get_ray_data(RayIntegers): caught Fault.\n");
        throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::get_ray_data(int index, RayDoubles  *rd) throw(Fault) {

    int numberOfFields;
    int binaryFormat;
    int numberOfCells;

    try{
        numberOfFields = get_integer("number_of_fields");
        if((index < 0) || (index >= numberOfFields)){
            char message[2048];
            sprintf(message,"NcRadarFile::get_ray_data(RayDoubles): field index of %d is out of bounds.\n",
                    index);
            throw Fault(message);
        }

        binaryFormat = get_integer("binary_format",index);
        numberOfCells = get_integer("number_of_cells");  

        rd->clear();
        rd->reserve(numberOfCells);

        size_t start[2];
        start[0] = currentRayIndex_;
        start[1] = 0;
	
        size_t count[2];
        count[0] = 1;
        count[1] = numberOfCells;

        int variableId = indexToVariableId_[index];

        if((binaryFormat == RayConst::binaryFormat1ByteInt) ||
           (binaryFormat == RayConst::binaryFormat2ByteInt)){

            double bias  = get_double("parameter_bias" ,index);
            double scale = get_double("parameter_scale",index);

            int *ncData = new int[numberOfCells];

            int status = nc_get_vara_int(ncFileId_,variableId,start,count,ncData);
            if(status != NC_NOERR){
                char statusMessage[2048];
                sprintf(statusMessage,"NcRadarFile::get_ray_data(RayDoubles): nc_inq_vara_int: returned error: %s.\n",
                        nc_strerror(status));
                throw Fault(statusMessage);
            }

            for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                rd->push_back((double)((ncData[cellIndex] * scale) + bias));
            }

            delete [] ncData;

        }else if(binaryFormat == RayConst::binaryFormat4ByteFloat){

            double *ncData = new double[numberOfCells];

            int status = nc_get_vara_double(ncFileId_,variableId,start,count,ncData);
            if(status != NC_NOERR){
                char statusMessage[2048];
                sprintf(statusMessage,"NcRadarFile::get_ray_data(RayDoubles): nc_inq_vara_double: returned error: %s.\n",
                        nc_strerror(status));
                throw Fault(statusMessage);
            }

            for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                rd->push_back(ncData[cellIndex]);
            }

            delete [] ncData;

        }else{
            char message[2048];
            sprintf(message,"NcRadarFile::get_ray_data(RayDoubles): binaryForamt of %d for field index of %d is invalid.\n ",
                    index,
                    binaryFormat);
            throw Fault(message);
        }

    }catch (Fault &fault){
        fault.add_msg("NcRadarFile::get_ray_data(RayDoulbes): caught Fault.\n");
        throw fault;
    }


}
    
//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::write_ground_headers() throw(Fault) {

    if(!writeFile_){
        throw Fault("NcRadarFile::write_ground_headers: file is not opened for writing.\n");
    }

    try {

        define_nc_file();
	
        write_nc_header_variables();

    }catch (Fault &fault){
        fault.add_msg("NcRadarFile::write_ground_headers: caught Fault.\n");
        throw fault;
    }
    
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::write_ground_ray() throw(Fault) {

    if(!writeFile_){
        throw Fault("NcRadarFile::write_ground_ray: file is not opened for writing.\n");
    }

    try{
        currentRayIndex_++;

        RaycTime baseTime      = get_time("start_time");
        RaycTime rayTime       = get_time("ray_time");
        double   secondsOffset = rayTime.value() - (double)baseTime.seconds();

        RaycAngle rayAzimuth   = get_angle("ray_azimuth");
        RaycAngle rayElevation = get_angle("ray_elevation");
	
        set_nc_double("time_offset",secondsOffset       ,currentRayIndex_);
        set_nc_float ("Azimuth"    ,rayAzimuth.value()  ,currentRayIndex_);
        set_nc_float ("Elevation"  ,rayElevation.value(),currentRayIndex_);
        set_nc_float ("clip_range" ,RayConst::badDouble ,currentRayIndex_);


    }catch (Fault &fault){
        fault.add_msg("NcRadarFile::write_ground_ray: caught Fault.\n");
        throw fault;
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::write_ground_tail() throw(Fault) {

    // Do nothing.
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_ray_data(int fieldIndex, RayIntegers &ri)  throw(Fault) {

    try {
        int rayIndex = currentRayIndex_ + 1;

        int format        = get_integer("binary_format",fieldIndex);
        int numberOfCells = get_integer("number_of_cells");

        if(numberOfCells > (int) ri.size()){
            char message[2048];
            sprintf(message,"NcRadarFile::set_ray_data(%d,RayIntegers): Number of Cells (%d) is greater then RayIntegers size (%ld).\n",
                    fieldIndex,
                    numberOfCells,
                    ri.size());
            throw Fault(message);
        }

        RayIntegers::iterator cellIterator;
        cellIterator = ri.begin();

        size_t start[2];
        size_t count[2];

        start[0] = rayIndex;
        start[1] = 0;
    
        count[0] = 1;
        count[1] = numberOfCells;

        int variableId = indexToVariableId_[fieldIndex];
        int status;
    
        switch(format){
        case RayConst::binaryFormat2ByteInt:

            {
                short *shortData = new short[numberOfCells]; 

                for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                    shortData[cellIndex] = *cellIterator;
                    cellIterator++;
                }
	
                status = nc_put_vara_short(ncFileId_,variableId,start,count,shortData);
                if(status != NC_NOERR){
                    char statusMessage[2048];
                    sprintf(statusMessage,"NcRadarFile::set_ray_data(%d,RayIntegers): nc_put_vara_short returned error: %s.\n",
                            fieldIndex,
                            nc_strerror(status));
                    throw Fault(statusMessage);
                }

                delete [] shortData;
            }

            break;

        case RayConst::binaryFormat4ByteFloat:

            {
                double scale = get_double("parameter_scale",fieldIndex);
                double bias  = get_double("parameter_bias" ,fieldIndex);
	
                float *floatData = new float[numberOfCells]; 

                for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                    double value = *cellIterator;
                    floatData[cellIndex] = (value * scale) + bias;
                    cellIterator++;
                }
	
                status = nc_put_vara_float(ncFileId_,variableId,start,count,floatData);
                if(status != NC_NOERR){
                    char statusMessage[2048];
                    sprintf(statusMessage,"NcRadarFile::set_ray_data(%d,RayDoubles): nc_put_vara_float returned error: %s.\n",
                            fieldIndex,
                            nc_strerror(status));
                    throw Fault(statusMessage);
                }

                delete [] floatData;
            }
	
            break;

        default:
            {
                char message[2048];
                sprintf(message,"NcRadarFile::set_ray_data(%d,RayDoubles): binaryFormat of %d not recognized or implemented.\n",
                        fieldIndex,
                        format);
                throw Fault(message);
            }
        }  

    }catch (Fault &fault){
        char message[2048];
        sprintf(message,"NcRadarFile::set_ray_data(%d,RayDoubles): caught fault.\n",
                fieldIndex);
        fault.add_msg(message);
        throw fault;
    }

    
    
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_ray_data(int fieldIndex, RayDoubles  &rd)  throw(Fault) {


    try {
        int rayIndex = currentRayIndex_ + 1;

        int format        = get_integer("binary_format",fieldIndex);
        int numberOfCells = get_integer("number_of_cells");

        if(numberOfCells > (int) rd.size()){
            char message[2048];
            sprintf(message,"NcRadarFile::set_ray_data(%d,RayDoubles): Number of Cells (%d) is greater then RayDoubles size (%ld).\n",
                    fieldIndex,
                    numberOfCells,
                    rd.size());
            throw Fault(message);
        }

        RayDoubles::iterator cellIterator;
        cellIterator = rd.begin();

        size_t start[2];
        size_t count[2];

        start[0] = rayIndex;
        start[1] = 0;
    
        count[0] = 1;
        count[1] = numberOfCells;

        int variableId = indexToVariableId_[fieldIndex];
        int status;
    
        switch(format){
        case RayConst::binaryFormat2ByteInt:
            {

                double scale = get_double("parameter_scale",fieldIndex);
                double bias  = get_double("parameter_bias" ,fieldIndex);

		int    badData = get_integer("bad_data",fieldIndex);
                short *shortData = new short[numberOfCells]; 

                for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                    double value = *cellIterator;
		    if((int)value == badData){
			shortData[cellIndex] = badData;
		    }else{

			double scaled = (value - bias) / scale;
                        if (scaled < RayConst::minShort) {
                          scaled = RayConst::minShort;
                        } else if (scaled > RayConst::maxShort) {
                          scaled = RayConst::maxShort;
                        }

                        shortData[cellIndex] = (short) floor(scaled + 0.5);

		    }

                    cellIterator++;
                }
	
                status = nc_put_vara_short(ncFileId_,variableId,start,count,shortData);
                if(status != NC_NOERR){
                    char statusMessage[2048];
                    sprintf(statusMessage,"NcRadarFile::set_ray_data(%d,RayDoubles): nc_put_vara_short returned error: %s.\n",
                            fieldIndex,
                            nc_strerror(status));
                    throw Fault(statusMessage);
                }

                delete [] shortData;

                break;
            }
	
        case RayConst::binaryFormat4ByteFloat:
	  
            {
	  
                float *floatData = new float[numberOfCells]; 

                for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){
                    floatData[cellIndex] = *cellIterator;
                    cellIterator++;
                }
	
                status = nc_put_vara_float(ncFileId_,variableId,start,count,floatData);
                if(status != NC_NOERR){
                    char statusMessage[2048];
                    sprintf(statusMessage,"NcRadarFile::set_ray_data(%d,RayDoubles): nc_put_vara_float returned error: %s.\n",
                            fieldIndex,
                            nc_strerror(status));
                    throw Fault(statusMessage);
                }

                delete [] floatData;
            }
	
            break;

        default:
            {
	  
                char message[2048];
                sprintf(message,"NcRadarFile::set_ray_data(%d,RayDoubles): binaryFormat of %d not recognized or implemented.\n",
                        fieldIndex,
                        format);
                throw Fault(message);
            }
	
        }  

    }catch (Fault &fault){
        char message[2048];
        sprintf(message,"NcRadarFile::set_ray_data(%d,RayDoubles): caught fault.\n",
                fieldIndex);
        fault.add_msg(message);
        throw fault;
    }
}

// Local Variables: 
// compile-command: "scons -u"
// End:

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool NcRadarFile::test_for_variable(const std::string &name)  throw(ForayUtility::Fault){

    int status;
    int variableId;

    status = nc_inq_varid(ncFileId_,name.c_str(),&variableId);

    if(status == NC_NOERR){
	return true;
    }

    // NC_ENOTVAR is error code for variable not found.  
    if(status != NC_ENOTVAR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_variable_id(%s): nc_inq_varid returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return false;
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::get_nc_variable_id(const std::string &name)  throw(ForayUtility::Fault){

    int status;
    int variableId;

    status = nc_inq_varid(ncFileId_,name.c_str(),&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_variable_id(%s): nc_inq_varid returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::get_nc_dimension_id(const std::string &name)  throw(ForayUtility::Fault){

    int status;
    int dimensionId;

    status = nc_inq_dimid(ncFileId_,name.c_str(),&dimensionId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_dimension_id(%s): nc_inq_demid returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return dimensionId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::get_nc_integer(const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    int    value;
    int    variableId;
    size_t index[] = {0};

    variableId = get_nc_variable_id(name);

    status = nc_get_var1_int(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_int(%s): nc_get_var1_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double NcRadarFile::get_nc_double(const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    double value;
    int    variableId;
    size_t index[] = {0};

    variableId = get_nc_variable_id(name);

    status = nc_get_var1_double(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_double(%s): nc_get_var1_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double NcRadarFile::get_nc_double(const std::string &name,const int index)  throw(ForayUtility::Fault){

    int    status;
    double value;
    int    variableId;
    size_t indexArray[1];

    variableId = get_nc_variable_id(name);

    indexArray[0] = index;

    status = nc_get_var1_double(ncFileId_,variableId,indexArray,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_double(%s,%d): nc_get_var1_double returned error: %s.\n",
		name.c_str(),
		index,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
std::string NcRadarFile::get_nc_short_string(const std::string &name,const int index)  throw(ForayUtility::Fault){

    int     status;
    char   *value;
    size_t  startArray[2];
    size_t  countArray[2];

    int shortStringSize = get_nc_dimension_length("short_string");
    int variableId      = get_nc_variable_id(name);

    startArray[0] = index;
    startArray[1] = 0;

    countArray[0] = 1;
    countArray[1] = shortStringSize;

    value = new char[shortStringSize + 1];
    memset(value,0,shortStringSize + 1);

    status = nc_get_vara_text(ncFileId_,variableId,startArray,countArray,value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_short_string(%s,%d): nc_get_vara_text returned error: %s.\n",
		name.c_str(),
		index,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    std::string originalValue(value);
    std::string returnValue;
    size_t space = originalValue.find(" ");
    if(space == std::string::npos ){
	returnValue = originalValue;
    }else{
	returnValue = originalValue.substr(0,space);
    }
		
    delete [] value;

    return returnValue;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::get_nc_dimension_length(const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    size_t dimensionLength;
    int    dimensionId;

    dimensionId = get_nc_dimension_id(name);

    status = nc_inq_dimlen(ncFileId_,dimensionId,&dimensionLength);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_dimension_length(%s): nc_get_dimlen returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return (int)dimensionLength;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
std::string NcRadarFile::get_nc_string_attribute(const std::string &name)  throw(ForayUtility::Fault){
    
    return get_nc_string_attribute(NC_GLOBAL,name);
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
std::string NcRadarFile::get_nc_string_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int    status;
    char   *value;
    size_t  length;
    
    status = nc_inq_attlen(ncFileId_,variableId,name.c_str(),&length);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_string_attribute(%s): nc_inq_attlen returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    value = new char[length + 1];
    memset(value,0,length + 1);

    status = nc_get_att_text(ncFileId_,variableId,name.c_str(),value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_string_attribute(%s): nc_get_att_text returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    std::string returnString(value);

    delete [] value;

    return returnString;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
bool NcRadarFile::test_nc_double_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int     status;
    double  value;

    status = nc_get_att_double(ncFileId_,variableId,name.c_str(),&value);

    if(status == NC_NOERR){
	return true;
    }

    // NC_ENOTVAR is error code for variable not found.  
    if(status != NC_ENOTATT){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::test_nc_double_attribute(%s): nc_get_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return false;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
double NcRadarFile::get_nc_double_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int     status;
    double  value;

    status = nc_get_att_double(ncFileId_,variableId,name.c_str(),&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_double_attribute(%s): nc_get_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::get_nc_integer_attribute(const int variableId, const std::string &name)  throw(ForayUtility::Fault){

    int  status;
    int  value;

    status = nc_get_att_int(ncFileId_,variableId,name.c_str(),&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::get_nc_integer_attribute(%s): nc_get_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_dimension(const std::string &name,const int length, int &dimensionId) throw(ForayUtility::Fault){

    int     status;

    status = nc_def_dim(ncFileId_,name.c_str(),(size_t)length,&dimensionId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_dimension(%s): nc_def_dim returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::create_nc_scalar(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  

    status = nc_def_var(ncFileId_,name.c_str(),type,0,NULL,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_scalar(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::create_nc_system_variable(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  
    int     dimensionIds[1];

    dimensionIds[0] = numSystemsDimensionId_;

    status = nc_def_var(ncFileId_,name.c_str(),type,1,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_system_variable(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::create_nc_time_variable(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  
    int     dimensionIds[1];

    dimensionIds[0] = timeDimensionId_;

    status = nc_def_var(ncFileId_,name.c_str(),type,1,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_time_variable(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::create_nc_data_variable(const std::string &name,const nc_type type) throw(ForayUtility::Fault){

    int     status;
    int     variableId;  
    int     dimensionIds[2];

    dimensionIds[0] = timeDimensionId_;
    dimensionIds[1] = maxCellsDimensionId_;

    status = nc_def_var(ncFileId_,name.c_str(),type,2,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_nc_data_variable(%s): nc_def_var returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    return variableId;
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_text_attribute(const int varid,const std::string &name,const std::string &value) throw(ForayUtility::Fault){

    int status;

    const char *cvalue = value.c_str();

    status = nc_put_att_text(ncFileId_,varid,name.c_str(),strlen(cvalue),cvalue);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_text_attribute(%s): nc_put_att_text returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_integer_attribute(const int varid,const std::string &name,const int &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_int(ncFileId_,varid,name.c_str(),NC_INT,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_integer_attribute(%s): nc_put_att_int returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_float_attribute(const int varid,const std::string &name,const double &value) throw(ForayUtility::Fault){

    int   status;
    float fvalue = value;

    status = nc_put_att_float(ncFileId_,varid,name.c_str(),NC_FLOAT,1,&fvalue);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_float_attribute(%s): nc_put_att_float returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_double_attribute(const int varid,const std::string &name,const double &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_double(ncFileId_,varid,name.c_str(),NC_DOUBLE,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_double_attribute(%s): nc_put_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_double_attribute(const int varid,const std::string &name,
						 const double &value1,const double &value2)  throw(ForayUtility::Fault){

    int    status;
    double dvalue[2];

    dvalue[0] = value1;
    dvalue[1] = value2;

    status = nc_put_att_double(ncFileId_,varid,name.c_str(),NC_DOUBLE,2,dvalue);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_double_attribute(%s): nc_put_att_double returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_short_attribute(const int varid,const std::string &name,const short &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_short(ncFileId_,varid,name.c_str(),NC_SHORT,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_short_attribute(%s): nc_put_att_short returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_int_attribute(const int varid,const std::string &name,const int &value) throw(ForayUtility::Fault){

    int   status;

    status = nc_put_att_int(ncFileId_,varid,name.c_str(),NC_INT,1,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_int_attribute(%s): nc_put_att_int returned error: %s.\n",
		name.c_str(),
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_missing_and_fill_attributes(const int varid) throw(ForayUtility::Fault){

    nc_type type;

    int status = nc_inq_vartype(ncFileId_,varid,&type);
    if(status != NC_NOERR){
        char varName[1024];
        strcpy(varName, "Unknown");
        nc_inq_varname(ncFileId_,varid,varName);
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_missing_and_fill_attributes: id=%d, name=%s, nc_inq_vartype returned error: %s.\n",
                varid, varName,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }

    
    try {

	if(type == NC_SHORT){
          set_missing_and_fill_attributes(varid,RayConst::badShort);
	}else{
          set_missing_and_fill_attributes(varid,RayConst::badDouble);
	}

    }catch(ForayUtility::Fault &fault){
	fault.add_msg("NcRadarFile::set_missing_and_fill_attributes: caught fault.\n");
	throw(fault);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_missing_and_fill_attributes(const int varid,const double value) throw(ForayUtility::Fault){
    
    try {

	nc_type type;

	int status = nc_inq_vartype(ncFileId_,varid,&type);
        char varName[1024];
        strcpy(varName, "Unknown");
        nc_inq_varname(ncFileId_,varid,varName);
	if(status != NC_NOERR){
          char statusMessage[2048];
          sprintf(statusMessage,"NcRadarFile::set_missing_and_fill_attributes: id=%d, name=%s, nc_inq_vartype returned error: %s.\n",
                  varid, varName,
                  nc_strerror(status));
	    throw ForayUtility::Fault(statusMessage);
	}

	if(type == NC_FLOAT){
	    set_nc_float_attribute(varid,"_FillValue"   ,value);
	    set_nc_float_attribute(varid,"missing_value",value);
	}else if(type == NC_DOUBLE){
	    set_nc_double_attribute(varid,"_FillValue"   ,value);
	    set_nc_double_attribute(varid,"missing_value",value);
	}else if(type == NC_SHORT){
	    set_nc_short_attribute(varid,"_FillValue"   ,(short)value);
	    set_nc_short_attribute(varid,"missing_value",(short)value);
	}else if(type == NC_INT){
	    set_nc_int_attribute(varid,"_FillValue"   ,(int)value);
	    set_nc_int_attribute(varid,"missing_value",(int)value);
	}else{
	  char statusMessage[2048];
	  sprintf(statusMessage,"NcRadarFile::set_missing_and_fill_attributes: id=%d, name=%s, type (%d) not supported. Value is %g\n",
                  varid, varName,
                  type,value);
	    throw ForayUtility::Fault(statusMessage);
	}

    }catch(ForayUtility::Fault &fault){
          char faultMessage[2048];
          sprintf(faultMessage,"NcRadarFile::set_missing_and_fill_attributes: caught Fault, id=%d, value=%g\n", varid, value);
	  fault.add_msg(faultMessage);
	throw(fault);
    }

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::create_fields_variable() throw(ForayUtility::Fault){

    int status;
    int dimensionIds[2];
    int variableId;

    dimensionIds[0] = numFieldsDimensionId_;
    dimensionIds[1] = shortStringDimensionId_;

    status = nc_def_var(ncFileId_,"field_names",NC_CHAR,2,dimensionIds,&variableId);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::create_fields_variable: nc_def_var returned error: %s.\n",
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_integer(const std::string &name, const int value) throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    size_t index[1];

    variableId = get_nc_variable_id(name);
    index[0]   = 0;

    status = nc_put_var1_int(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_integer(%s,%d): nc_put_var1_int returned error: %s.\n",
		name.c_str(),
		value,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_float(const std::string &name, const float value) throw(ForayUtility::Fault){

    set_nc_float(name,value,0);

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_float(const std::string &name, const float value,const int indexValue) throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    size_t index[1];

    variableId = get_nc_variable_id(name);
    index[0]   = indexValue;

    status = nc_put_var1_float(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_float(%s,%g,%d): nc_put_var1_float returned error: %s.\n",
		name.c_str(),
		value,
		indexValue,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_double(const std::string &name, const double value) throw(ForayUtility::Fault){

    set_nc_double(name,value,0);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_double(const std::string &name, const double value, 
				       const int indexValue) throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    size_t index[1];

    variableId = get_nc_variable_id(name);
    index[0]   = indexValue;

    status = nc_put_var1_double(ncFileId_,variableId,index,&value);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::set_nc_double(%s,%f,%d): nc_put_var1_double returned error: %s.\n",
		name.c_str(),
		value,
		indexValue,
		nc_strerror(status));
	throw ForayUtility::Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_fields_variable() throw(ForayUtility::Fault){

    int    status;
    int    variableId;
    int    numberOfFields;
    size_t start[2];
    size_t count[2];

    try {
	variableId     = get_nc_variable_id("field_names");
	numberOfFields = get_integer("number_of_fields");

	for(int fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){

	    std::string fieldName = get_string("field_name",fieldIndex);

	    start[0] = fieldIndex;
	    start[1] = 0;
	    count[0] = 1;
	    count[1] = fieldName.size();

	    status = nc_put_vara_text(ncFileId_,variableId,start,count,fieldName.c_str());

	    if(status != NC_NOERR){
		char statusMessage[2048];
		sprintf(statusMessage,"NcRadarFile::set_fields_variable: nc_put_vara_text returned error: %s.\n",
			nc_strerror(status));
		throw ForayUtility::Fault(statusMessage);
	    }
	}
	
    }catch(ForayUtility::Fault &fault){
	fault.add_msg("NcRadarFile::set_fields_variable: caught fault.\n");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::process_instrument_type(const string &type)  throw(Fault){

    if(type == "GROUND"){
	return RayConst::radarTypeGround;
    }else if(type == "Ground"){
	return RayConst::radarTypeGround;
    }else{
	char message[1024];
	sprintf(message,"NcRadarFile::process_instrument_type: Undefined Instrument_Type of %s\n",
		type.c_str());
	throw Fault(message);
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int NcRadarFile::process_scan_mode(const string &mode)  throw(Fault){

    if(mode == "PPI"){
	return RayConst::scanModePPI;
    }else if(mode == "RHI"){
	return RayConst::scanModeRHI;
    }else if(mode == "SUR"){
	return RayConst::scanModeSUR;
    }else if(mode == "TAR"){
	return RayConst::scanModeTAR;
    }else{
	char message[1024];
	sprintf(message,"NcRadarFile::process_scan_mode: Undefined Scan_Mode of %s\n",
		mode.c_str());
	throw Fault(message);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::process_field_info()  throw(Fault){

    
    try{

	int numberOfFields     = get_nc_dimension_length("fields");
	set_integer("number_of_fields",numberOfFields);

	indexToVariableId_.clear();


	string fieldNameKey;
	if(test_for_variable("field_names")){
	    fieldNameKey = "field_names";
	}else if(test_for_variable("fields")){
	    fieldNameKey = "fields";
	}else{
	    throw Fault("NcRadarFile::process_field_info: Can not find variable with field names.\n");
	}

	for(int index = 0; index < numberOfFields; index ++){

	    string fieldName  = get_nc_short_string(fieldNameKey,index);
	    int    variableId = get_nc_variable_id(fieldName);
	    indexToVariableId_.push_back(variableId);

	    string longName   = get_nc_string_attribute(variableId,"long_name");

	    string units   = get_nc_string_attribute(variableId,"units");
	    int    unitsId = (int)RayConst::unitsUnknown;
	    if(units == "none"){
		unitsId = (int)RayConst::unitsNone;
	    }else if(units == "dBz"){
		unitsId = (int)RayConst::unitsReflectivity;
	    }else if(units == "dBm"){
		unitsId = (int)RayConst::unitsPower;
	    }else if((units == "m/s") || (units == "meters/second")){
		unitsId = (int)RayConst::unitsVelocity;
	    }

	    int  badData  = get_nc_integer_attribute(variableId,"missing_value");

	    nc_type variableType;
	    int status = nc_inq_vartype(ncFileId_,variableId,&variableType);
	    if(status != NC_NOERR){
		char statusMessage[2048];
		sprintf(statusMessage,"NcRadarFile::process_field_info: nc_inq_vartype: returned error: %s.\n",
			nc_strerror(status));
		throw ForayUtility::Fault(statusMessage);
	    }

	    int    binaryFormat;
	    double scale;
	    double bias;

	    if(variableType == NC_BYTE){
		binaryFormat = RayConst::binaryFormat1ByteInt;
		scale        = get_nc_double_attribute (variableId,"scale_factor");
		if(test_nc_double_attribute (variableId,"add_offset")){
		    //cout << "Should be add_offset (BYTE)" << endl;
		    bias         = get_nc_double_attribute (variableId,"add_offset");
		}else{
		    bias = 0.0;
		}

	    }else if(variableType == NC_SHORT){
		binaryFormat = RayConst::binaryFormat2ByteInt;
		scale        = get_nc_double_attribute (variableId,"scale_factor");
		if(test_nc_double_attribute (variableId,"add_offset")){
		    //cout << "Should be add_offset (SHORT)" << endl;
		    bias         = get_nc_double_attribute (variableId,"add_offset");
		}else{
		    bias = 0.0;
		}

	    }else if(variableType == NC_FLOAT){
		binaryFormat = RayConst::binaryFormat4ByteFloat;
		scale        = 100.0;
		bias         = 0.0;
	    }else{
		char message[2048];
		sprintf(message,"NcRadarFile::process_field_info: can not decode variable type used by %s \n.",
			fieldName.c_str());
		throw Fault(message);
	    }

	    int fieldPolarization;
	    string polarization = get_nc_string_attribute(variableId,"polarization");
	    if(polarization == "None"){
		fieldPolarization = RayConst::noPolarization;
	    }else if((polarization == "Horizontal") || (polarization == "H")){
		fieldPolarization = RayConst::horizontalPolarization;
	    }else if((polarization == "Vertical") || (polarization == "V")){
		fieldPolarization = RayConst::verticalPolarization;
	    }else{
		fieldPolarization = RayConst::unknownPolarization;
	    }

	    set_string ("field_name"        ,index,fieldName);	
	    set_string ("field_long_name"   ,index,longName);
	    set_integer("field_units"       ,index,unitsId);
	    set_integer("bad_data"          ,index,badData);
	    set_double ("parameter_scale"   ,index,scale);
	    set_double ("parameter_bias"    ,index,bias);
	    set_integer("binary_format"     ,index,binaryFormat);
	    set_integer("field_polarization",index,fieldPolarization);

	}
    }catch (Fault &fault){
	fault.add_msg("NcRadarFile::process_field_info: cault Fault.\n");
	throw fault;
    }

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::define_nc_file() throw (Fault){

    try{
	define_nc_dimensions();
	define_nc_variables();

    }catch(Fault &fault){
	fault.add_msg("NcRadarFile::define_nc_file: caught Fault.\n");
	throw fault;
    }

    int status  = nc_enddef(ncFileId_);
    if(status != NC_NOERR){
	char statusMessage[2048];
	sprintf(statusMessage,"NcRadarFile::define_nc_file: nc_enddef returned error: %s.\n",
		nc_strerror(status));
	throw Fault(statusMessage);
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::define_nc_dimensions() throw (Fault){

    try{
	set_nc_dimension("Time"        ,NC_UNLIMITED                   ,timeDimensionId_       );
	set_nc_dimension("maxCells"    ,get_integer("number_of_cells") ,maxCellsDimensionId_   );
	set_nc_dimension("numSystems"  ,1                              ,numSystemsDimensionId_ );
	set_nc_dimension("fields"      ,get_integer("number_of_fields"),numFieldsDimensionId_  );
	set_nc_dimension("short_string",32                             ,shortStringDimensionId_);
	set_nc_dimension("long_string" ,80                             ,longStringDimensionId_);
    }catch(Fault &fault){
	fault.add_msg("NcRadarFile::define_nc_dimesions: caught Fault.\n");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::define_nc_variables() throw (Fault){


    try{
	
	int variableId;
        int fieldIndex;

	variableId = create_nc_scalar("volume_start_time",NC_INT);
	set_nc_text_attribute(variableId,"long_name","Unix Date/Time value for volume start time");
	set_nc_text_attribute(variableId,"units"    ,"seconds since 1970-01-01 00:00 UTC"        );

	variableId = create_nc_scalar("base_time",NC_INT);
	set_nc_text_attribute(variableId,"long_name","Unix Date/Time value for first record");
	set_nc_text_attribute(variableId,"units"    ,"seconds since 1970-01-01 00:00 UTC"   );

	create_fields_variable();

	variableId = create_nc_scalar("Fixed_Angle",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Targeted fixed angle for this scan");
	set_nc_text_attribute(variableId,"units"    ,"degrees");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_scalar("Range_to_First_Cell",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Range to the center of the first cell");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_scalar("Cell_Spacing",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Distance between cells");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_scalar("Nyquist_Velocity",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Effective unambigous velocity");
	set_nc_text_attribute(variableId,"units"    ,"meters/second");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_scalar("Unambiguous_Range",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Effective unambigous range");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_scalar("Latitude",NC_DOUBLE);
	set_nc_text_attribute(variableId,"long_name","Latitude of the instrument");
	set_nc_text_attribute(variableId,"units"    ,"degrees");
	set_missing_and_fill_attributes(variableId);
	set_nc_double_attribute(variableId,"valid_range",-90.0,90.0);

	variableId = create_nc_scalar("Longitude",NC_DOUBLE);
	set_nc_text_attribute(variableId,"long_name","Longitude of the instrument");
	set_nc_text_attribute(variableId,"units"    ,"degrees");
	set_missing_and_fill_attributes(variableId);
	set_nc_double_attribute(variableId,"valid_range",-360.0,360.0);

	variableId = create_nc_scalar("Altitude",NC_DOUBLE);
	set_nc_text_attribute(variableId,"long_name","Altitude in meters (asl) of the instrument");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);
	set_nc_double_attribute(variableId,"valid_range",-10000.0,90000.0);

	variableId = create_nc_system_variable("Radar_Constant",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Radar Constant");
	set_nc_text_attribute(variableId,"units"    ,"mm6/(m3.mW.km-2)");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("rcvr_gain",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Receiver Gain");
	set_nc_text_attribute(variableId,"units"    ,"dB");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("ant_gain",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Antenna Gain");
	set_nc_text_attribute(variableId,"units"    ,"dB");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("sys_gain",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","System Gain");
	set_nc_text_attribute(variableId,"units"    ,"dB");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("bm_width",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Beam Width");
	set_nc_text_attribute(variableId,"units"    ,"degrees");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("pulse_width",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Pulse Width");
	set_nc_text_attribute(variableId,"units"    ,"seconds");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("band_width",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Band Width");
	set_nc_text_attribute(variableId,"units"    ,"hertz");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("peak_pwr",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Peak Power");
	set_nc_text_attribute(variableId,"units"    ,"watts");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("xmtr_pwr",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Transmitter Power");
	set_nc_text_attribute(variableId,"units"    ,"dBM");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("noise_pwr",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Noise Power");
	set_nc_text_attribute(variableId,"units"    ,"dBM");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("tst_pls_pwr",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Test Pulse Power");
	set_nc_text_attribute(variableId,"units"    ,"dBM");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("tst_pls_rng0",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Range to start of test pulse");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("tst_pls_rng1",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","Range to end of test pulse");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("Wavelength",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","System wavelength");
	set_nc_text_attribute(variableId,"units"    ,"meters");
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_system_variable("PRF",NC_FLOAT);
	set_nc_text_attribute(variableId,"long_name","System pulse repetition frequency");
	set_nc_text_attribute(variableId,"units"    ,"pulses/sec");
	set_missing_and_fill_attributes(variableId);

	// 
	// Calibration Values
	//

	bool calibrationDataPresent = get_boolean("calibration_data_present");

	variableId = create_nc_system_variable("calibration_data_present",NC_INT);
	set_nc_text_attribute(variableId,"long_name","Used at bool; 0 = calibration variables used, 1 = calibration variables not used.");
	set_missing_and_fill_attributes(variableId,RayConst::badShort);

	if(calibrationDataPresent){

	    variableId = create_nc_system_variable("ant_gain_h_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Antenna gain H in db");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("ant_gain_v_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Antenna gain V in db");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("xmit_power_h_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Peak transmit H power in dBm");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("xmit_power_v_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Peak transmit V power in dBm");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("two_way_waveguide_loss_h_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","two way H waveguide loss in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("two_way_waveguide_loss_v_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","two way V waveguide loss in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("two_way_radome_loss_h_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","two way H radome loss in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("two_way_radome_loss_v_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","two way V radome loss in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("receiver_mismatch_loss_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Receiver mismatch loss in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("radar_constant_h",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Radar constant H");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("radar_constant_v",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Radar constant V");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("noise_hc_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","calibrated moise value H co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("noise_vc_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","calibrated moise value V co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("noise_hx_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","calibrated moise value H cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("noise_vx_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","calibrated moise value V cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("receiver_gain_hc_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Receiver gain H co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("receiver_gain_vc_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Receiver gain V co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("receiver_gain_hx_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Receiver gain H cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("receiver_gain_vx_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Receiver gain V cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("base_1km_hc_dbz",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Base reflectivity at 1km, H co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBz");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("base_1km_vc_dbz",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Base reflectivity at 1km, V co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBz");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("base_1km_hx_dbz",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Base reflectivity at 1km, H cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBz");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("base_1km_vx_dbz",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Base reflectivity at 1km, V cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBz");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("sun_power_hc_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Sun power H co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("sun_power_vc_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Sun power V co-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("sun_power_hx_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Sun power H cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("sun_power_vx_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Sun power V cross-polar");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("noise_source_power_h_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Noise source power H in dBm");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("noise_source_power_v_dbm",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Noise source power V in dBm");
	    set_nc_text_attribute(variableId,"units"    ,"dBm");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("power_measure_loss_h_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","power measurement loss H in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("power_measure_loss_v_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","power measurement loss V in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("coupler_forward_loss_h_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Directional coupler forward loss H in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("coupler_forward_loss_v_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","Directional coupler forward loss V in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("zdr_correction_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","zdr correction in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("ldr_correction_h_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","ldr correction H in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("ldr_correction_v_db",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","ldr correction V in dB");
	    set_nc_text_attribute(variableId,"units"    ,"dB");
	    set_missing_and_fill_attributes(variableId);

	    variableId = create_nc_system_variable("system_phidp_deg",NC_FLOAT);
	    set_nc_text_attribute(variableId,"long_name","system phidp, degrees");
	    set_nc_text_attribute(variableId,"units"    ,"degrees");
	    set_missing_and_fill_attributes(variableId);
	}

	// 
	// time variables	
	//

	variableId = create_nc_time_variable("time_offset",NC_DOUBLE);
	set_nc_text_attribute(variableId,"long_name","time offset of the current record from base_time");
	set_nc_text_attribute(variableId,"units"    ,"seconds");
	set_missing_and_fill_attributes(variableId,0.0);

	variableId = create_nc_time_variable("Azimuth",NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"Earth relative azimuth of the ray");
	set_nc_text_attribute  (variableId,"Comment"    ,"Degrees clockwise from true North");
	set_nc_text_attribute  (variableId,"units"      ,"degrees");
	set_nc_double_attribute(variableId,"valid_range",-360.0,360.0);
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_time_variable("Elevation",NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"Earth relative elevation of the ray");
	set_nc_text_attribute  (variableId,"Comment"    ,"Degrees from earth tangent towards zenith");
	set_nc_text_attribute  (variableId,"units"      ,"degrees");
	set_nc_double_attribute(variableId,"valid_range",-360.0,360.0);
	set_missing_and_fill_attributes(variableId);

	variableId = create_nc_time_variable("clip_range",NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"Range of last useful cell");
	set_nc_text_attribute  (variableId,"units"      ,"meters");
	set_missing_and_fill_attributes(variableId);

	indexToVariableId_.clear();

	int numberOfFields = get_integer("number_of_fields");
	for(fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){

	    string fieldName = get_string ("field_name"   ,fieldIndex);
	    int    format    = get_integer("binary_format",fieldIndex);
	    
	    if(format == RayConst::binaryFormat4ByteFloat){
		variableId = create_nc_data_variable(fieldName.c_str(),NC_FLOAT);
	    }else if(format == RayConst::binaryFormat2ByteInt){
		variableId = create_nc_data_variable(fieldName.c_str(),NC_SHORT);
	    }else{
		char message[2048];
		sprintf(message,"NcRadarFile::define_nc_varialbes: binaryFormat of %d not supported by NcRadarFile.",
			format);
		throw Fault(message);
	    }

	    indexToVariableId_.push_back(variableId);

	    set_nc_field_attributes(variableId,fieldIndex);
	}

	RaycTime time = get_time("start_time");
	int year   = time.get_year();
	int month  = time.get_month();
	int day    = time.get_day();
	int hour   = time.get_hour();
	int minute = time.get_minute();
	int second = time.get_second();
	char volumeStartTime[2048];
	sprintf(volumeStartTime,"%4d/%02d/%02d %02d:%02d:%02d",
		year,month,day,hour,minute,second);

	RaycTime productionTime;
	productionTime.set_current_time();

	char productionTimeString[2048];
	sprintf(productionTimeString,"%4d/%02d/%02d %02d:%02d:%02d",
		productionTime.get_year(),
		productionTime.get_month(),
		productionTime.get_day(),
		productionTime.get_hour(),
		productionTime.get_minute(),
		productionTime.get_second());
	
	set_nc_text_attribute   (NC_GLOBAL,"Conventions"       ,"NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor");
	set_nc_text_attribute   (NC_GLOBAL,"Instrument_Name"  ,get_string("platform_name"));
	set_nc_text_attribute   (NC_GLOBAL,"Instrument_Type"  ,RayConst::radarTypes[get_integer("platform_type")]);
	set_nc_text_attribute   (NC_GLOBAL,"Scan_Mode"        ,RayConst::scanModes[get_integer("scan_mode")]);
	set_nc_text_attribute   (NC_GLOBAL,"Volume_Start_Time",volumeStartTime);
	set_nc_integer_attribute(NC_GLOBAL,"Year"             ,year);
	set_nc_integer_attribute(NC_GLOBAL,"Month"            ,month);
	set_nc_integer_attribute(NC_GLOBAL,"Day"              ,day);
	set_nc_integer_attribute(NC_GLOBAL,"Hour"             ,hour);
	set_nc_integer_attribute(NC_GLOBAL,"Minute"           ,minute);
	set_nc_integer_attribute(NC_GLOBAL,"Second"           ,second);
	set_nc_integer_attribute(NC_GLOBAL,"Volume_Number"    ,get_integer("volume_number"));
	set_nc_integer_attribute(NC_GLOBAL,"Scan_Number"      ,get_integer("sweep_number"));
	set_nc_integer_attribute(NC_GLOBAL,"Num_Samples"      ,get_integer("number_of_samples"));
	set_nc_text_attribute   (NC_GLOBAL,"Project_Name"     ,get_string ("project_name"));
	set_nc_text_attribute   (NC_GLOBAL,"Production_Date"  ,productionTimeString);
	set_nc_text_attribute   (NC_GLOBAL,"Producer_Name"    ,get_string ("producer_name"));
	set_nc_text_attribute   (NC_GLOBAL,"Software"         ,"Foray NcRadarFile Class");

    }catch(Fault &fault){
	fault.add_msg("NcRadarFile::define_nc_variables: caught Fault.\n");
	throw fault;
    }
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::write_nc_header_variables() throw (Fault){

    try{

	int startTime = get_time("start_time").seconds();
	set_nc_integer("volume_start_time",startTime);
	set_nc_integer("base_time"        ,startTime);

	set_fields_variable();
	
	set_nc_float  ("Fixed_Angle"         ,get_angle("fixed_angle").value());
	set_nc_float  ("Range_to_First_Cell" ,get_double("meters_to_first_cell"));

	if(get_integer("cell_spacing_method") != RayConst::cellSpacingBySegment){
	    char message[1024];
	    sprintf(message,"NcRadarFile::write_nc_header_variables: cell spacing description must be by segment. Set values is %d.\n",
		    get_integer("cell_spacing_method"));
	    throw Fault(message);
	}
	if(get_integer("number_of_cell_segments") != 1){
	    char message[1024];
	    sprintf(message,"NcRadarFile::write_nc_header_variables: number of cell segments must be 1. Set values is %d.\n",
		    get_integer("number_of_cell_segments"));
	    throw Fault(message);
	}
	
	set_nc_float ("Cell_Spacing"     ,get_double("segment_cell_spacing",0));
	set_nc_float ("Nyquist_Velocity" ,get_double("nyquist_velocity"));
	set_nc_float ("Unambiguous_Range",get_double("unambiguous_range"));
	set_nc_double("Latitude"         ,get_double("platform_latitude"));
	set_nc_double("Longitude"        ,get_double("platform_longitude"));
	set_nc_double("Altitude"         ,get_double("platform_altitude"));
	set_nc_float ("Radar_Constant"   ,get_double("radar_constant"));
	set_nc_float ("rcvr_gain"        ,get_double("receiver_gain"));
	set_nc_float ("ant_gain"         ,get_double("antenna_gain"));
	set_nc_float ("sys_gain"         ,get_double("system_gain"));
	set_nc_float ("bm_width"         ,get_double("horizontal_beam_width"));
	set_nc_float ("pulse_width"      ,get_double("pulse_width"));
	set_nc_float ("band_width"       ,get_double("band_width"));
	set_nc_float ("peak_pwr"         ,get_double("peak_power"));
	set_nc_float ("xmtr_pwr"         ,get_double("transmitter_power"));
	set_nc_float ("noise_pwr"        ,get_double("noise_power"));
	set_nc_float ("tst_pls_pwr"      ,get_double("test_pulse_power"));
	set_nc_float ("tst_pls_rng0"     ,get_double("test_pulse_start_range"));
	set_nc_float ("tst_pls_rng1"     ,get_double("test_pulse_end_range"));
	set_nc_float ("Wavelength"       ,LIGHTSPEED_ /get_double("frequency",0));
        set_nc_float ("PRF"              ,get_double("pulse_repetition_frequency",0));

	bool calibrationDataPresent = get_boolean("calibration_data_present");
	if(!calibrationDataPresent){
	    set_nc_integer("calibration_data_present",0);
	}else{
	    set_nc_integer("calibration_data_present",1);
	    
	    set_nc_float("ant_gain_h_db"                     ,get_double("horizontal_antenna_gain_db"));
	    set_nc_float("ant_gain_v_db"                     ,get_double("vertical_antenna_gain_db"));	    
	    set_nc_float("xmit_power_h_dbm"                  ,get_double("horizontal_transmitter_power_dbm"));
	    set_nc_float("xmit_power_v_dbm"                  ,get_double("vertical_transmitter_power_dbm"));
	    set_nc_float("two_way_waveguide_loss_h_db"       ,get_double("horizontal_two_way_waveguide_loss_db"));
	    set_nc_float("two_way_waveguide_loss_v_db"       ,get_double("vertical_two_way_waveguide_loss_db"));
	    set_nc_float("two_way_radome_loss_h_db"          ,get_double("horizontal_two_way_radome_loss_db"));
	    set_nc_float("two_way_radome_loss_v_db"          ,get_double("vertical_two_way_radome_loss_db"));
	    set_nc_float("receiver_mismatch_loss_db"         ,get_double("receiver_mismatch_loss_db"));
	    set_nc_float("radar_constant_h"                  ,get_double("horizontal_radar_constant"));
	    set_nc_float("radar_constant_v"                  ,get_double("vertical_radar_constant"));
	    set_nc_float("noise_hc_dbm"                      ,get_double("horizontal_co_polar_noise_dbm"));
	    set_nc_float("noise_vc_dbm"                      ,get_double("vertical_co_polar_noise_dbm"));
	    set_nc_float("noise_hx_dbm"                      ,get_double("horizontal_cross_polar_noise_dbm"));
	    set_nc_float("noise_vx_dbm"                      ,get_double("vertical_cross_polar_noise_dbm"));
	    set_nc_float("receiver_gain_hc_db"               ,get_double("horizontal_co_polar_receiver_gain_db"));
	    set_nc_float("receiver_gain_vc_db"               ,get_double("vertical_co_polar_receiver_gain_db"));
	    set_nc_float("receiver_gain_hx_db"               ,get_double("horizontal_cross_polar_receiver_gain_db"));
	    set_nc_float("receiver_gain_vx_db"               ,get_double("vertical_cross_polar_receiver_gain_db"));
	    set_nc_float("base_1km_hc_dbz"                   ,get_double("horizontal_co_polar_base_dbz_at_1km"));
	    set_nc_float("base_1km_vc_dbz"                   ,get_double("vertical_co_polar_base_dbz_at_1km"));
	    set_nc_float("base_1km_hx_dbz"                   ,get_double("horizontal_cross_polar_base_dbz_at_1km"));
	    set_nc_float("base_1km_vx_dbz"                   ,get_double("vertical_cross_polar_base_dbz_at_1km"));
	    set_nc_float("sun_power_hc_dbm"                  ,get_double("horizontal_co_polar_sun_power_dbm"));
	    set_nc_float("sun_power_vc_dbm"                  ,get_double("vertical_co_polar_sun_power_dbm"));
	    set_nc_float("sun_power_hx_dbm"                  ,get_double("horizontal_cross_polar_sun_power_dbm"));
	    set_nc_float("sun_power_vx_dbm"                  ,get_double("vertical_cross_polar_sun_power_dbm"));
	    set_nc_float("noise_source_power_h_dbm"          ,get_double("horizontal_noise_source_power_dbm"));
	    set_nc_float("noise_source_power_v_dbm"          ,get_double("vertical_noise_source_power_dbm"));
	    set_nc_float("power_measure_loss_h_db"           ,get_double("horizontal_power_measurement_loss_db"));
	    set_nc_float("power_measure_loss_v_db"           ,get_double("vertical_power_measurement_loss_db"));
	    set_nc_float("coupler_forward_loss_h_db"         ,get_double("horizontal_coupler_forward_loss_db"));
	    set_nc_float("coupler_forward_loss_v_db"         ,get_double("vertical_coupler_forward_loss_db"));
	    set_nc_float("zdr_correction_db"                 ,get_double("zdr_correction_db"));
	    set_nc_float("ldr_correction_h_db"               ,get_double("horizontal_ldr_correction_db"));
	    set_nc_float("ldr_correction_v_db"               ,get_double("vertical_ldr_correction_db"));
	    set_nc_float("system_phidp_deg"                  ,get_double("system_phidp_degrees"));                   

	}

    }catch(Fault &fault){
	fault.add_msg("NcRadarFile::write_nc_header_variables: caught Fault.\n");
	throw fault;
    }
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void NcRadarFile::set_nc_field_attributes(const int varid,const int fieldIndex) throw(Fault){
    
    try {

	string longName = get_string("field_long_name",fieldIndex);
	set_nc_text_attribute(varid,"long_name",longName);

	int units = get_integer("field_units",fieldIndex);
	if(units == RayConst::unitsNone){
	    set_nc_text_attribute(varid,"units","none");
	}else if(units == RayConst::unitsReflectivity){
	    set_nc_text_attribute(varid,"units","dBz");
	}else if(units == RayConst::unitsPower){
	    set_nc_text_attribute(varid,"units","dBm");
	}else if(units == RayConst::unitsVelocity){
	    set_nc_text_attribute(varid,"units","m/s");
	}else if(units == RayConst::unitsUnknown){
	    set_nc_text_attribute(varid,"units","unknown");
	}else if(units == RayConst::unitsLogCounts){
	    set_nc_text_attribute(varid,"units","dB");
	}else if(units == RayConst::unitsDigitizerCounts){
	    set_nc_text_attribute(varid,"units","digitizercounts");
	}else{
	    char message[2048];
	    sprintf(message,"NcRadarFile::set_nc_field_attributes: units id of %d for field index of %d not recognized.\n",
		    units,
		    fieldIndex);
	    throw ForayUtility::Fault(message);
	}

	nc_type type;

	int status = nc_inq_vartype(ncFileId_,varid,&type);
	if(status != NC_NOERR){
	    char statusMessage[2048];
	    sprintf(statusMessage,"NcRadarFile::set_nc_field_attributes: nc_inq_vartype returned error: %s.\n",
		    nc_strerror(status));
	    throw ForayUtility::Fault(statusMessage);
	}

	if(type == NC_SHORT){
	    set_nc_float_attribute(varid,"scale_factor",get_double("parameter_scale",fieldIndex));
	    set_nc_float_attribute(varid,"add_offset",  get_double("parameter_bias" ,fieldIndex));
	}
        // use specified scale/offset, otherwise default to 1.0/0.0
        if ( type == NC_FLOAT) {
            if (key_is_used("parameter_scale")) {
                set_nc_float_attribute(varid,"scale_factor",get_double("parameter_scale",fieldIndex));
            } else {
                set_nc_float_attribute(varid,"scale_factor",1.0);
            }
            if (key_is_used("parameter_bias")) {
                set_nc_float_attribute(varid,"add_offset",  get_double("parameter_bias" ,fieldIndex));
            } else {
                set_nc_float_attribute(varid,"add_offset",  0.0);
            }
        }
        

	set_missing_and_fill_attributes(varid);

	int polarization = get_integer("field_polarization",fieldIndex);
	switch(polarization){
	case RayConst::noPolarization:
	    set_nc_text_attribute(varid,"polarization","None");
	    break;
	case RayConst::horizontalPolarization:
	    set_nc_text_attribute(varid,"polarization","Horizontal");
	    break;
	case RayConst::verticalPolarization:
	    set_nc_text_attribute(varid,"polarization","Vertical");
	    break;
	case RayConst::circularPolarization:
	    set_nc_text_attribute(varid,"polarization","Circular");
	    break;
	case RayConst::ellipticalPolarization:
	    set_nc_text_attribute(varid,"polarization","Elliptical");
	    break;
	case RayConst::unknownPolarization:
	    set_nc_text_attribute(varid,"polarization","Unknown");
	    break;

        // added for REAL lidar
	case RayConst::parallelPolarization:
	    set_nc_text_attribute(varid,"polarization","parallel");
	    break;
	case RayConst::perpendicularPolarization:
	    set_nc_text_attribute(varid,"polarization","perpendicular");
	    break;

	default:
	    char message[2048];
	    sprintf(message,"NcRadarFile::set_nc_field_attributes: polarization id of %d for field index of %d not recognized.\n",
		    polarization,
		    fieldIndex);
	    throw ForayUtility::Fault(message);
	}

	double frequency = get_double("frequency",0);
	set_nc_double_attribute(varid,"Frequencies_GHz",frequency / 1e+9);

	double prf = get_double("pulse_repetition_frequency",0);
	set_nc_double_attribute(varid,"InterPulsePeriods_secs",1.0/prf);

	// Right now, NcRadar can only encode cell spacing by segement with
	// number_of_segments == 1.
	int spacingMethod = get_integer("cell_spacing_method");
	if(spacingMethod != (int)RayConst::cellSpacingBySegment){
	    throw Fault("NcRadarFile::set_nc_field_attribute: Cell Spacing must be by segment.\n");
	}
	
	int numberOfSegments = get_integer("number_of_cell_segments");
	if(numberOfSegments != 1){
	    char message[2048];
	    sprintf(message,"NcRadarFile::set_nc_field_attribute: number_of_cell_segments must equal 1, current value is %d.",
		    numberOfSegments);
	    throw Fault(message);
	}
		
	set_nc_integer_attribute(varid,"num_segments",1);

	int numberOfCells = get_integer("segment_cell_count",0);
	set_nc_integer_attribute(varid,"cells_in_segment",numberOfCells);
	
	double metersToFirstCell = get_double("meters_to_first_cell");
	set_nc_double_attribute(varid,"meters_to_first_cell",metersToFirstCell);
	
	double segmentCellSpacing = get_double("segment_cell_spacing",0);
	set_nc_double_attribute(varid,"meters_between_cells",segmentCellSpacing);

    }catch(ForayUtility::Fault &fault){
	fault.add_msg("NcRadarFile::set_nc_field_attributes: caught fault.\n");
	throw(fault);
    }
}
