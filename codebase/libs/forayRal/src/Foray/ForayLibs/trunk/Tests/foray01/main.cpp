/*
 *  RayFile test Application 01.
 *
 */

#include <iostream>
using namespace std;

#include <stdio.h>

#include "RayConst.h"
#include "RayFile.h"
#include "DoradeFile.h"
#include "DoradeFileName.h"
#include "NcRadarFile.h"
using namespace ForayUtility;

const string   radarName("post");
const int      scanType(RayConst::scanModePPI);
const double   fixedAngle(1.5);
const int      volumeNumber(56);
const int      sweepNumber(4);
const int      numberOfRays(355);
const double   startAngle(15.0);

const double   timeIncreament(0.75);


void create_file (RayFile &file, RaycTime &startTime, const string name ) throw (Fault);
void build_header(RayFile &file, RaycTime &startTime)                     throw (Fault);
void build_ray   (RayFile &file, RaycTime &startTime, const int    index) throw (Fault);

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

    cout << "Hi" << endl;


    try {

	RaycTime time;

	time.set_current_time();
	
	DoradeFile  doradeFile;
	DoradeFileName namer;
	string name = namer.generate_swp_name(time,radarName,scanType,fixedAngle,volumeNumber);
	create_file(doradeFile,time,name);
	
	NcRadarFile ncRadarFile;
	create_file(ncRadarFile,time,"test_ncRadar.nc");


    }catch (Fault &fault){
	
	cout << "Caught Fault: \n";
	cout << fault.msg();
    }

    cout << "Bye" << endl;
}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void create_file(RayFile &file, RaycTime &time, string name) throw (Fault){

    try {

	cout << "creating file " << name << endl;
	
	file.open_file(name,true);

	build_header(file,time);
	file.write_ground_headers();

	for(int index=0; index < numberOfRays; index++){
	    build_ray(file,time,index);
	    file.write_ground_ray();
	}

	file.write_ground_tail();
	file.close_file();

    }catch (Fault &fault){
	char message[2048];
	sprintf(message,"Caught Fault while creating %s.\n",name.c_str());
	fault.add_msg(message);
	throw fault;
    }

}


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void build_header(RayFile &file,RaycTime &startTime) throw (Fault){

    try {

	int cellSpacingMethod = RayConst::cellSpacingBySegment;

	file.set_time   ("start_time"                ,startTime);	
	file.set_time   ("stop_time"                 ,startTime + ((numberOfRays - 1) * timeIncreament));
	file.set_angle  ("start_angle"               ,RaycAngle(startAngle));
	file.set_angle  ("stop_angle"                ,RaycAngle(startAngle + 360.0 - (numberOfRays/360.0)));
	file.set_angle  ("fixed_angle"               ,RaycAngle(fixedAngle));   
	file.set_integer("number_of_cells"           ,500);
	file.set_integer("cell_spacing_method"       ,cellSpacingMethod);
	file.set_integer("number_of_cell_segments"   ,1);
	file.set_double ("segment_cell_spacing"    ,0,150.0);
	file.set_integer("segment_cell_count"      ,0,500);
	file.set_double ("meters_to_first_cell"      ,1000.0);
	file.set_double ("nyquist_velocity"          ,26.7);
	file.set_double ("unambiguous_range"         ,145000.0);
	file.set_double ("platform_longitude"        ,-105.18);
	file.set_double ("platform_latitude"         ,39.95);
	file.set_double ("platform_altitude"         ,1743.0);
	file.set_double ("radar_constant"            ,69.0458);
	file.set_double ("receiver_gain"             ,46.95);
	file.set_double ("antenna_gain"              ,45.63);
	file.set_double ("system_gain"               ,RayConst::badDouble);
	file.set_double ("horizontal_beam_width"     ,1.0);
	file.set_double ("vertical_beam_width"       ,1.0);
	file.set_double ("pulse_width"               ,0.000000994021);
	file.set_double ("band_width"                ,1e+12);
	file.set_double ("peak_power"                ,524807.0);
	file.set_double ("transmitter_power"         ,RayConst::badDouble);
	file.set_double ("noise_power"               ,-116.6);
	file.set_double ("test_pulse_power"          ,RayConst::badDouble);
	file.set_double ("test_pulse_start_range"    ,RayConst::badDouble);
	file.set_double ("test_pulse_end_range"      ,RayConst::badDouble);
	file.set_integer("number_of_frequencies"     ,1);
	file.set_double ("frequency"               ,0,2.8090e+9);
	file.set_integer("number_of_prfs"            ,1);
	file.set_double ("pulse_repetition_frequency",0,1024.0);

	file.set_string ("platform_name"         ,"radar05");
	file.set_integer("platform_type"         , (int)RayConst::radarTypeGround);
	file.set_integer("scan_mode"             , (int)RayConst::scanModePPI);
	file.set_integer("volume_number"         ,volumeNumber);
	file.set_integer("sweep_number"          ,sweepNumber);
	file.set_integer("number_of_samples"     ,48);
	file.set_string ("project_name"          ,"example05");
	file.set_string ("producer_name"         ,"NCAR EOL");

	file.set_boolean("calibration_data_present", false);

	file.set_integer("number_of_fields"  ,5);

	file.set_string ("field_name"     ,0,"dbz");
	file.set_string ("field_name"     ,1,"vr");
	file.set_string ("field_name"     ,2,"dbz2");
	file.set_string ("field_name"     ,3,"vr2");
	file.set_string ("field_name"     ,4,"pid");

	file.set_integer("binary_format",  0,(int)RayConst::binaryFormat2ByteInt);
	file.set_integer("binary_format",  1,(int)RayConst::binaryFormat2ByteInt);
	file.set_integer("binary_format",  2,(int)RayConst::binaryFormat2ByteInt);
	file.set_integer("binary_format",  3,(int)RayConst::binaryFormat2ByteInt);
	//	file.set_integer("binary_format",  2,(int)RayConst::binaryFormat4ByteFloat);
	//	file.set_integer("binary_format",  3,(int)RayConst::binaryFormat4ByteFloat);
	file.set_integer("binary_format",  4,(int)RayConst::binaryFormat2ByteInt);

	file.set_integer("bad_data"     ,  0,(int)RayConst::badShort);
	file.set_integer("bad_data"     ,  1,(int)RayConst::badShort);
	file.set_integer("bad_data"     ,  2,(int)RayConst::badShort);
	file.set_integer("bad_data"     ,  3,(int)RayConst::badShort);
	file.set_integer("bad_data"     ,  4,(int)RayConst::badShort);

	file.set_string ("field_long_name",0,"Reflectivity");
	file.set_string ("field_long_name",1,"radial velocity");
	file.set_string ("field_long_name",2,"Reflectivity");
	file.set_string ("field_long_name",3,"radial velocity");
	file.set_string ("field_long_name",4,"particle identification");

	file.set_integer("field_units"    ,0,(int)RayConst::unitsReflectivity);
	file.set_integer("field_units"    ,1,(int)RayConst::unitsVelocity);
	file.set_integer("field_units"    ,2,(int)RayConst::unitsReflectivity);
	file.set_integer("field_units"    ,3,(int)RayConst::unitsVelocity);
	file.set_integer("field_units"    ,4,(int)RayConst::unitsNone);


	file.compute_2byte_scale_and_bias(0,-55,55);
	//file.set_double ("parameter_scale",0,0.01);
	//file.set_double ("parameter_bias" ,0,0.0);

	file.compute_2byte_scale_and_bias(1,-25,25);
	//file.set_double ("parameter_scale",1,0.01);
	//file.set_double ("parameter_bias" ,1,0.0);

	file.set_double ("parameter_scale",2,0.01);
	file.set_double ("parameter_bias" ,2,-20.0);

	file.set_double ("parameter_scale",3,0.01);
	file.set_double ("parameter_bias" ,3,-20.0);

	file.set_double ("parameter_scale",4,1.0);
	file.set_double ("parameter_bias" ,4,0.0);

	file.set_integer("field_polarization" ,0,(int)RayConst::horizontalPolarization);
	file.set_integer("field_polarization" ,1,(int)RayConst::horizontalPolarization);
	file.set_integer("field_polarization" ,2,(int)RayConst::horizontalPolarization);
	file.set_integer("field_polarization" ,3,(int)RayConst::horizontalPolarization);
	file.set_integer("field_polarization" ,4,(int)RayConst::noPolarization);

	// Not used by NcRadar
	file.set_integer("number_of_rays"        ,numberOfRays);

	
    }catch (Fault &fault){
	
	fault.add_msg("build_header: caught Fault.\n");
	throw fault;
    }
    
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
void build_ray(RayFile &file,RaycTime &startTime,const int index) throw (Fault){


    try {

	RaycTime rayTime;
	rayTime = startTime + (index * timeIncreament);

	RaycAngle rayAzimuth;
	rayAzimuth = startAngle - (index * (360.0/numberOfRays));

	file.set_time   ("ray_time"          ,rayTime);
	file.set_angle  ("ray_azimuth"       ,rayAzimuth);
	file.set_angle  ("ray_elevation"     ,RaycAngle(fixedAngle));
	file.set_double ("ray_peak_power"    ,-999);
	file.set_double ("ray_true_scan_rate",timeIncreament);
	file.set_integer("ray_status"        ,(int)RayConst::rayStatusNormal);

	int angleSection = (int)((rayAzimuth.value() / 360.0 ) * 8.0);

	bool angleSectionEven(true);
	if((angleSection & 0x1) == 1){
	    angleSectionEven = false;
	}

	int numberOfCells = file.get_integer("number_of_cells");

	RayDoubles rayData0;
	RayDoubles rayData1;
	RayIntegers pidData;

	rayData0.reserve(numberOfCells);
	rayData1.reserve(numberOfCells);
	pidData.reserve(numberOfCells);
    
	for(int cellIndex = 0; cellIndex < numberOfCells; cellIndex++){

	    int cellSection = (int)(((double)cellIndex / numberOfCells) * 8.0);
	
	    bool cellSectionEven(true);
	    if((cellSection & 0x1) == 1){
		cellSectionEven = false;
	    }
	
	    double  value0(RayConst::badShort);
	    double  value1(RayConst::badShort);
	    int     pidValue(0);

	    if((cellSectionEven && angleSectionEven) ||
	       (!cellSectionEven && !angleSectionEven)){

		value0 = (angleSection * 10.0) - 20.0;
		value1 = (angleSection * 5.0)  - 20.0;

		pidValue = angleSection + cellSection + 1;
	    }

	    rayData0.push_back(value0);
	    rayData1.push_back(value1);
	    pidData.push_back(pidValue);
	}

	file.set_ray_data(0,rayData0);
	file.set_ray_data(1,rayData1);
	file.set_ray_data(2,rayData0);
	file.set_ray_data(3,rayData1);
	file.set_ray_data(4,pidData);

    }catch (Fault &fault){
	fault.add_msg("build_rays: cault Fault \n");
	throw fault;
    }

}
