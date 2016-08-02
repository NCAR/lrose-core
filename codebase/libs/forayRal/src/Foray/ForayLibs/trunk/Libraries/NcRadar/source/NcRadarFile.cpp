//
//
//
//

#include <cmath>
#include "FilePath.h"

#include "NcRadarFile.h"
#include "NcRadarFile_inline.h"
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

        if(numberOfCells > ri.size()){
            char message[2048];
            sprintf(message,"NcRadarFile::set_ray_data(%d,RayIntegers): Number of Cells (%d) is greater then RayIntegers size (%d).\n",
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

        if(numberOfCells > rd.size()){
            char message[2048];
            sprintf(message,"NcRadarFile::set_ray_data(%d,RayDoubles): Number of Cells (%d) is greater then RayDoubles size (%d).\n",
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
