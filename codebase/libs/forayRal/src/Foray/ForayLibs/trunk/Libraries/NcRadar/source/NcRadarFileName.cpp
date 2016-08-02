//
//
//
//

#include <RayConst.h>

#include "NcRadarFileName.h"
using namespace std;
using namespace ForayUtility;


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
NcRadarFileName::NcRadarFileName(){



}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
NcRadarFileName::~NcRadarFileName(){



}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string NcRadarFileName::generate_sweep_name(const RaycTime    &time,
					    const std::string radar,
					    const int         scanId,
					    const double      fixedAngle,
					    const int         volumeNumber,
					    const int         sweepNumber) throw(Fault){

    
    char cname[1024];

    try{

	int year        = time.get_year();
	int month       = time.get_month();
	int day         = time.get_day();

	int hour        = time.get_hour();
	int minute      = time.get_minute();
	int second      = time.get_second();

	int nanosecond  = time.get_nanosecond();
	int millisecond = nanosecond / 1000000;

	string scanType(RayConst::scanModes[scanId]);

	//                    R    Y   M   D    H   M   S   ms     v     s     f  s
	sprintf(cname,"ncswp_%s_%04d%02d%02d_%02d%02d%02d.%03d_v%03d_s%03d_%05.1f_%s_.nc",
		radar.c_str(),
		year,month,day,
		hour,minute,second,
		millisecond,
		volumeNumber,
		sweepNumber,
		fixedAngle,
		scanType.c_str());

    }catch(Fault &fault){

	char message[2048];
	sprintf(message,"NcRadarFileName::generate_sweep_name: Caught Fault. \n");
	fault.add_msg(message);
	throw fault;
    }

    return string(cname);
}

string NcRadarFileName::generate_sweep_name(RayFile &rayFile) throw(Fault){

    try{
	RaycTime  startTime  = rayFile.get_time   ("start_time");
	string    radarName  = rayFile.get_string ("platform_name");
	int       scanMode   = rayFile.get_integer("scan_mode");
	RaycAngle fixedAngle = rayFile.get_angle  ("fixed_angle");
	int       volume     = rayFile.get_integer("volume_number");
	int       sweep      = rayFile.get_integer("sweep_number");
	
	return this->generate_sweep_name(startTime,radarName,scanMode,fixedAngle.value(),volume,sweep);

    }catch(Fault &fault){
	fault.add_msg("NcRadarFileName::generate_sweep_name(RayFile): caught fault\n");
	throw fault;
    }
}
