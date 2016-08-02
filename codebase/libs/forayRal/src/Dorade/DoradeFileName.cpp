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

#include <stdio.h>

#include "Dorade.h"
#include "DoradeFileName.h"
using namespace std;
using namespace ForayUtility;


//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeFileName::DoradeFileName(){



}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
DoradeFileName::~DoradeFileName(){



}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeFileName::generate_sweep_name(const RaycTime &time,
					 const string   radar,
					 const int      scanId,
					 const double   fixedAngle,
					 const int      volume)          throw(Fault){


    char cname[1024];

    if((scanId < Dorade::scanModeCAL) || (scanId > Dorade::scanModeHOR)){
	char msg[2048];
	sprintf(msg,"DoradeFileName::swp_name: scanId value of %d not valid. \n",scanId);
	throw Fault(msg);
    }

    try{
	int year        = time.get_year() - 1900;
	int month       = time.get_month();
	int day         = time.get_day();

	int hour        = time.get_hour();
	int minute      = time.get_minute();
	int second      = time.get_second();

	int nanosecond  = time.get_nanosecond();
	int millisecond = nanosecond / 1000000;

	string scanType(Dorade::scanModes[scanId]);
	
	//                    Y   M   D   H   M   S  R   MS     F  T     V
	// sprintf(cname,"swp.%03d%02d%02d%02d%02d%02d.%s.%3d.%05.1f_%s_v%03d",
	sprintf(cname,"swp.%03d%02d%02d%02d%02d%02d.%s.%d.%05.1f_%s_v%03d",
		year,month,day,
		hour,minute,second,
		radar.c_str(),
		millisecond,
		fixedAngle,
		scanType.c_str(),
		volume);

    }catch(Fault &fault){

	char msg[2048];
	sprintf(msg,"DoradeFileName::generate_sweep_name: Caught Fault. \n");
	fault.add_msg(msg);
	throw fault;
    }

    return string(cname);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeFileName::generate_sweep_name(RayFile &rayFile) throw (Fault){

    try{
	RaycTime  startTime  = rayFile.get_time   ("start_time");
	string    radarName  = rayFile.get_string ("platform_name");
	int       scanMode   = rayFile.get_integer("scan_mode");
	RaycAngle fixedAngle = rayFile.get_angle  ("fixed_angle");
	int       volume     = rayFile.get_integer("volume_number");
	
	return this->generate_sweep_name(startTime,radarName,scanMode,fixedAngle.value(),volume);

    }catch(Fault &fault){
	fault.add_msg("DoradeFileName::generate_sweep_name(RayFile): caught fault\n");
	throw fault;
    }
}



//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeFileName::generate_swp_name(const RaycTime &time,
					 const string   radar,
					 const int      scanId,
					 const double   fixedAngle,
					 const int      volume)          throw(Fault){

    return generate_sweep_name(time,radar,scanId,fixedAngle,volume);
}

//////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////
string DoradeFileName::generate_swp_name(RayFile &rayFile) throw (Fault){
    
    return generate_sweep_name(rayFile);
}
