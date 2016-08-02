
#include "NcREAL_File.h"
using namespace std;
using namespace ForayUtility;

void    NcREAL_File::define_nc_variables()                 throw(Fault) 
{
    NcRadarFile::define_nc_variables();
    try{
	
	int variableId;
	variableId = create_nc_time_variable("background_counts_a",NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"average background counts for channel a");
	set_nc_text_attribute  (variableId,"Comment"    ,"average value for middle 80% of pre xmit cells");
	set_nc_text_attribute  (variableId,"units"      ,"digitizercounts");
	set_missing_and_fill_attributes(variableId);

        variableId = create_nc_time_variable("background_counts_b",NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"average background counts for channel b");
	set_nc_text_attribute  (variableId,"Comment"    ,"average value for middle 80% of pre xmit cells");
	set_nc_text_attribute  (variableId,"units"      ,"digitizercounts");
	set_missing_and_fill_attributes(variableId);

        variableId = create_nc_time_variable("pulse_energy",NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"Energy transmitted for this pulse");
	set_nc_text_attribute  (variableId,"Comment"    ,"in milli-joules");
	set_nc_text_attribute  (variableId,"units"      ,"mJ");
	set_missing_and_fill_attributes(variableId);

        variableId = create_nc_scalar("energy_normalization", NC_FLOAT);
	set_nc_text_attribute  (variableId,"long_name"  ,"Expected energy");
	set_nc_text_attribute  (variableId,"units"      ,"mJ");
        

    }catch(Fault &fault){
	fault.add_msg("NcREAL_File::define_nc_variables: caught Fault.\n");
	throw fault;
    }

}

void NcREAL_File::write_ground_ray()                           throw(Fault)
{
    NcRadarFile::write_ground_ray();  // call this first, since it sets currentRayIndex_
    try{
        double bg_counts_a = get_double("bg_counts_a");
        double bg_counts_b = get_double("bg_counts_b");
        double pulse_energy = get_double("pulse_energy");
        
        set_nc_float ("background_counts_a"    ,bg_counts_a ,currentRayIndex_);
        set_nc_float ("background_counts_b"    ,bg_counts_b ,currentRayIndex_);
        set_nc_float ("pulse_energy"  ,pulse_energy,currentRayIndex_);


    } catch (Fault &fault){
        fault.add_msg("NcREAL_File::write_ground_ray: caught Fault.\n");
        throw fault;
    }   
        
}

void NcREAL_File::write_nc_header_variables() throw (Fault) {
    NcRadarFile::write_nc_header_variables();
    try {
        set_nc_float("energy_normalization", get_double("energy_normalization"));
    } catch(Fault &fault){
	fault.add_msg("NcREAL_File::write_nc_header_variables: caught Fault.\n");
	throw fault;
    }
}

    
    

