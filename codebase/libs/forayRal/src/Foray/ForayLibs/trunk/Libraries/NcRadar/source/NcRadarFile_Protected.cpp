//
//
//
//
//

#include "NcRadarFile.h"
#include "NcRadarFile_inline.h"
using namespace std;
using namespace ForayUtility;

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
