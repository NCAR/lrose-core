

#include "RayFile.h"
using namespace std;
using namespace ForayUtility;

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RayFile::RayFile(){

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
RayFile::~RayFile(){

}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void RayFile::copy_headers(RayFile &source) throw (Fault){

    try {

	set_integer("number_of_cells"      , source.get_integer("number_of_cells"));
	set_integer("cell_spacing_method"  , source.get_integer("cell_spacing_method"));

	if( get_integer("cell_spacing_method") == RayConst::cellSpacingBySegment){
	    int numberOfCellSegments = source.get_integer("number_of_cell_segments");
	    set_double ("meters_to_first_cell"    ,source.get_double("meters_to_first_cell"));
	    set_integer("number_of_cell_segments" ,numberOfCellSegments);

	    for(int segment = 0; segment < numberOfCellSegments; segment++){
		set_double ("segment_cell_spacing",segment, source.get_double ("segment_cell_spacing",segment));
		set_integer("segment_cell_count"  ,segment, source.get_integer("segment_cell_count"  ,segment));
	    }
	}
	
	int numberOfFields = source.get_integer("number_of_fields");
	set_integer("number_of_fields",numberOfFields);
	for(int fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){

	    set_string ("field_name"        ,fieldIndex,source.get_string ("field_name"        ,fieldIndex));
	    set_string ("field_long_name"   ,fieldIndex,source.get_string ("field_name"        ,fieldIndex));
	    set_integer("field_units"       ,fieldIndex,source.get_integer("field_units"       ,fieldIndex));
	    set_integer("field_polarization",fieldIndex,source.get_integer("field_polarization",fieldIndex));
	    set_integer("binary_format"     ,fieldIndex,source.get_integer("binary_format"     ,fieldIndex));
	    set_double ("parameter_scale"   ,fieldIndex,source.get_double ("parameter_scale"   ,fieldIndex));
	    set_double ("parameter_bias"    ,fieldIndex,source.get_double ("parameter_bias"    ,fieldIndex));
	    set_integer("bad_data"          ,fieldIndex,source.get_integer("bad_data"          ,fieldIndex));
	}

	set_string ("platform_name"        , source.get_string ("platform_name"));
	set_double ("platform_longitude"   , source.get_double ("platform_longitude"));
	set_double ("platform_latitude"    , source.get_double ("platform_latitude"));
	set_double ("platform_altitude"    , source.get_double ("platform_altitude"));
	set_integer("platform_type"        , source.get_integer("platform_type"));

	set_integer("number_of_samples"    , source.get_integer("number_of_samples"));
	set_double ("radar_constant"       , source.get_double ("radar_constant"));
	set_double ("peak_power"           , source.get_double ("peak_power"));
	set_double ("noise_power"          , source.get_double ("noise_power"));
	set_double ("transmitter_power"    , source.get_double ("transmitter_power"));
	set_double ("receiver_gain"        , source.get_double ("receiver_gain"));
	set_double ("antenna_gain"         , source.get_double ("antenna_gain"));
	set_double ("system_gain"          , source.get_double ("system_gain"));
	set_double ("horizontal_beam_width", source.get_double ("horizontal_beam_width"));
	set_double ("vertical_beam_width"  , source.get_double ("vertical_beam_width"));
	set_integer("scan_mode"            , source.get_integer("scan_mode"));
	set_double ("nyquist_velocity"     , source.get_double ("nyquist_velocity"));
	set_double ("unambiguous_range"    , source.get_double ("unambiguous_range"));
	set_double ("pulse_width"          , source.get_double ("pulse_width"));
	set_double ("band_width"           , source.get_double ("band_width"));

	int numberOfFrequencies = source.get_integer("number_of_frequencies");
	set_integer("number_of_frequencies",numberOfFrequencies);
	for(int frequencyIndex = 0; frequencyIndex < numberOfFrequencies; frequencyIndex++){
	    set_double("frequency",frequencyIndex, source.get_double("frequency",frequencyIndex));
	}

	int numberOfPrfs = source.get_integer("number_of_prfs");
	set_integer("number_of_prfs",numberOfPrfs);
	for(int prfIndex = 0; prfIndex < numberOfPrfs; prfIndex++){
	    set_double("pulse_repetition_frequency",prfIndex,source.get_double("pulse_repetition_frequency",prfIndex));
	}

	set_integer("sweep_number"         , source.get_integer("sweep_number"));
	set_integer("volume_number"        , source.get_integer("volume_number"));
	set_integer("number_of_rays"       , source.get_integer("number_of_rays"));
	set_angle  ("fixed_angle"          , source.get_angle  ("fixed_angle"));
	set_angle  ("start_angle"          , source.get_angle  ("start_angle"));
	set_angle  ("stop_angle"           , source.get_angle  ("stop_angle"));

	set_time   ("start_time"           , source.get_time   ("start_time"));
	set_time   ("stop_time"            , source.get_time   ("stop_time"));
	
	set_string ("project_name"         , source.get_string ("project_name"));
	set_string ("file_name"            , source.get_string ("file_name"));
	set_string ("directory_name"       , source.get_string ("directory_name"));

	bool testPulsePresent = source.get_boolean("test_pulse_present");
	set_double ("test_pulse_power"      , source.get_double ("test_pulse_power"));
	if(testPulsePresent){
	    set_double ("test_pulse_start_range", source.get_double ("test_pulse_start_range"));
	    set_double ("test_pulse_end_range"  , source.get_double ("test_pulse_end_range"));
	    set_boolean("test_pulse_present"    , source.get_boolean("test_pulse_present"));
	}


	bool calibrationDataPresent = source.get_boolean("calibration_data_present");
	set_boolean("calibration_data_present",calibrationDataPresent);

	if(calibrationDataPresent){
	    set_double("horizontal_antenna_gain_db"           ,source.get_double("horizontal_antenna_gain_db"));
	    set_double("vertical_antenna_gain_db"             ,source.get_double("vertical_antenna_gain_db"));

	    set_double("horizontal_transmitter_power_dbm"     ,source.get_double("horizontal_transmitter_power_dbm"));
	    set_double("vertical_transmitter_power_dbm"       ,source.get_double("vertical_transmitter_power_dbm"));

	    set_double("horizontal_two_way_waveguide_loss_db" ,source.get_double("horizontal_two_way_waveguide_loss_db"));
	    set_double("vertical_two_way_waveguide_loss_db"   ,source.get_double("vertical_two_way_waveguide_loss_db"));

	    set_double("horizontal_two_way_radome_loss_db"    ,source.get_double("horizontal_two_way_radome_loss_db"));
	    set_double("vertical_two_way_radome_loss_db"      ,source.get_double("vertical_two_way_radome_loss_db"));

	    set_double("receiver_mismatch_loss_db"            ,source.get_double("receiver_mismatch_loss_db"));

	    set_double("horizontal_radar_constant"            ,source.get_double("horizontal_radar_constant"));
	    set_double("vertical_radar_constant"              ,source.get_double("vertical_radar_constant"));

	    set_double("horizontal_co_polar_noise_dbm"        ,source.get_double("horizontal_co_polar_noise_dbm"));
	    set_double("vertical_co_polar_noise_dbm"          ,source.get_double("vertical_co_polar_noise_dbm"));
	    set_double("horizontal_cross_polar_noise_dbm"     ,source.get_double("horizontal_cross_polar_noise_dbm"));
	    set_double("vertical_cross_polar_noise_dbm"       ,source.get_double("vertical_cross_polar_noise_dbm"));


	    set_double("horizontal_co_polar_receiver_gain_db"       ,source.get_double("horizontal_co_polar_receiver_gain_db"));
	    set_double("vertical_co_polar_receiver_gain_db"         ,source.get_double("vertical_co_polar_receiver_gain_db"));
	    set_double("horizontal_cross_polar_receiver_gain_db"    ,source.get_double("horizontal_cross_polar_receiver_gain_db"));
	    set_double("vertical_cross_polar_receiver_gain_db"      ,source.get_double("vertical_cross_polar_receiver_gain_db"));

	    set_double("horizontal_co_polar_base_dbz_at_1km"        ,source.get_double("horizontal_co_polar_base_dbz_at_1km"));
	    set_double("vertical_co_polar_base_dbz_at_1km"          ,source.get_double("vertical_co_polar_base_dbz_at_1km"));
	    set_double("horizontal_cross_polar_base_dbz_at_1km"     ,source.get_double("horizontal_cross_polar_base_dbz_at_1km"));
	    set_double("vertical_cross_polar_base_dbz_at_1km"       ,source.get_double("vertical_cross_polar_base_dbz_at_1km"));

	    set_double("horizontal_co_polar_sun_power_dbm"          ,source.get_double("horizontal_co_polar_sun_power_dbm"));
	    set_double("vertical_co_polar_sun_power_dbm"            ,source.get_double("vertical_co_polar_sun_power_dbm"));
	    set_double("horizontal_cross_polar_sun_power_dbm"       ,source.get_double("horizontal_cross_polar_sun_power_dbm"));
	    set_double("vertical_cross_polar_sun_power_dbm"         ,source.get_double("vertical_cross_polar_sun_power_dbm"));
	    
	    set_double("horizontal_noise_source_power_dbm"    ,source.get_double("horizontal_noise_source_power_dbm"));
	    set_double("vertical_noise_source_power_dbm"      ,source.get_double("vertical_noise_source_power_dbm"));

	    set_double("horizontal_power_measurement_loss_db" ,source.get_double("horizontal_power_measurement_loss_db"));
	    set_double("vertical_power_measurement_loss_db"   ,source.get_double("vertical_power_measurement_loss_db"));

	    set_double("horizontal_coupler_forward_loss_db"   ,source.get_double("horizontal_coupler_forward_loss_db"));
	    set_double("vertical_coupler_forward_loss_db"     ,source.get_double("vertical_coupler_forward_loss_db"));

	    set_double("zdr_correction_db"                    ,source.get_double("zdr_correction_db"));
	    
	    set_double("horizontal_ldr_correction_db"         ,source.get_double("horizontal_ldr_correction_db"));
	    set_double("vertical_ldr_correction_db"           ,source.get_double("vertical_ldr_correction_db"));

	    set_double("system_phidp_degrees"                 ,source.get_double("system_phidp_degrees"));
	}

    }catch(Fault &fault){
	fault.add_msg("RayFile::copy_headers caught Fault.\n");
	throw fault;
    }
}


void RayFile::compute_2byte_scale_and_bias(int index, double minValue,double maxValue) throw(ForayUtility::Fault){

    try{

	double minShort = RayConst::minShort;
	double maxShort = RayConst::maxShort; 
	
	double scale = (maxValue - minValue)/(maxShort - minShort);
	double bias  = maxValue - (scale * maxShort);

	set_double("parameter_scale",index,scale);
	set_double("parameter_bias",index,bias);

    }catch(Fault &fault){

	char message[2048];
	sprintf(message,"Fault in compute_scale_and_bais(%d,%f,%f)",index,minValue,maxValue);
	fault.add_msg(message);
	throw(fault);
    }

}


