/*
 *  RayFile test Application 01.
 *
 */

#include <iostream>
using namespace std;

#include <stdio.h>

#include "FilePath.h"
#include "RayFile.h"
#include "DoradeFile.h"
#include "DoradeFileName.h"

using namespace ForayUtility;

const string   radarName("test");
const int      scanType(RayConst::scanModePPI);
const double   fixedAngle(1.5);
const int      volumeNumber(56);
const int      sweepNumber(4);
const int      numberOfRays(355);
const double   startAngle(15.0);

const double   timeIncreament(0.75);


void process_file(string inputName, string outputDir) throw (Fault &);


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

    cout << "Hi" << endl;


    try {

	cout << "input directory:" << argv[1] << endl;
	cout << "output direcory:" << argv[2] << endl;

	FilePath filePath;

	filePath.directory(argv[1],"swp.*");

	filePath.sort_files();

	filePath.first_file();

	do{
	    process_file(filePath.get_full_name(),argv[2]);
	}while(filePath.next_file());

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
void process_file(string inputName,string outputDir) throw (Fault &){

    try {

	DoradeFile originalFile;
	DoradeFile newFile;

	originalFile.open_file(inputName,false);
	originalFile.read_headers();

	RaycTime  startTime  = originalFile.get_time   ("start_time");
	string    radarName  = originalFile.get_string ("platform_name");
	int       scanMode   = originalFile.get_integer("scan_mode");
	RaycAngle fixedAngle = originalFile.get_angle  ("fixed_angle");
	int       volume     = originalFile.get_integer("volume_number");

	DoradeFileName outNamer;
	string outBaseName = outNamer.generate_swp_name(startTime,radarName,scanMode,fixedAngle.value(),volume);

	FilePath outPath;
	string   outputName = outPath.combine(outputDir,outBaseName);
	
	cout << endl;
	cout << "in  " << inputName << endl;
	cout << "out " << outputName << endl;


	newFile.open_file(outputName,true);
	
	newFile.copy_headers(originalFile);

	int numberOfFields = originalFile.get_integer("number_of_fields");	
	for(int fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){
	    char line[2048];
	    string fieldName = originalFile.get_string("field_name"     ,fieldIndex);
	    double scale     = originalFile.get_double("parameter_scale",fieldIndex);
	    double bias      = originalFile.get_double("parameter_bias" ,fieldIndex);

	    sprintf(line,"%8s %8.4f %8.4f\n",fieldName.c_str(),scale,bias);


	    cout << line;

	    if((fieldName == "DBZ") || (fieldName == "ZDR") || (fieldName == "DM")){
		cout << "Change scale and bias for " << fieldName << endl;

		newFile.set_double("parameter_scale",fieldIndex,0.01);
		newFile.set_double("parameter_bias" ,fieldIndex,0.0);
	    }

	}

	newFile.write_ground_headers();



	RayDoubles rayDoubles;

	originalFile.find_first_ray();
	do{
	    newFile.set_time   ("ray_time"          ,originalFile.get_time   ("ray_time"          ));
	    newFile.set_angle  ("ray_azimuth"       ,originalFile.get_angle  ("ray_azimuth"       ));
	    newFile.set_angle  ("ray_elevation"     ,originalFile.get_angle  ("ray_elevation"     ));
	    newFile.set_double ("ray_peak_power"    ,originalFile.get_double ("ray_peak_power"    ));
	    newFile.set_double ("ray_true_scan_rate",originalFile.get_double ("ray_true_scan_rate"));
	    newFile.set_integer("ray_status"        ,originalFile.get_integer("ray_status"        ));

	    for(int fieldIndex = 0; fieldIndex < numberOfFields; fieldIndex++){
		originalFile.get_ray_data(fieldIndex,&rayDoubles);
		newFile.set_ray_data(fieldIndex,rayDoubles);
	    }

	    newFile.write_ground_ray();

	}while(originalFile.find_next_ray());

	newFile.write_ground_tail();
	newFile.close_file();

	originalFile.close_file();

    }catch (Fault &fault){
	char message[2048];
	sprintf(message,"Caught Fault while processing %s.\n",inputName.c_str());
	fault.add_msg(message);
	throw fault;
    }

}
