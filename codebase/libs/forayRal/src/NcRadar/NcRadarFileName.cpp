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
