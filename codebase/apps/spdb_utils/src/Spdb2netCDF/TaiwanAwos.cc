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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: TaiwanAwos.cc,v 1.2 2016/03/07 01:39:56 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	TaiwanAwos
//
// Author:	G. M. Cunning
//
// Date:	Sun Oct 19 09:55:23 2008
//
// Description: This class manages Taiwan ASOS/AWOS SPDB messages.
//
//


// C++ include files

// System/RAP include files

// Local include files
#include <dataport/bigend.h>
#include <toolsa/udatetime.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>

#include "TaiwanAwos.hh"

using namespace std;

// define any constants
const string TaiwanAwos::_className    = "TaiwanAwos";
const float TaiwanAwos::missingVal = -9999.0;

const string TaiwanAwos::AIRPORT_ID_VAR_NAME = "airport_id";
const string TaiwanAwos::AWOS_ID_VAR_NAME = "awos_id";
const string TaiwanAwos::INFO_VAR_NAME = "info";
const string TaiwanAwos::LATITUDE_VAR_NAME = "latitude";
const string TaiwanAwos::LONGITUDE_VAR_NAME = "longitude";
const string TaiwanAwos::ALTITUDE_VAR_NAME = "altitude";

const string TaiwanAwos::VALID_TIME_VAR_NAME = "valid_time";
const string TaiwanAwos::INSTANT_WIND_SPEED_VAR_NAME = "instant_wind_speed";
const string TaiwanAwos::INSTANT_WIND_DIR_VAR_NAME = "instant_wind_dir";
const string TaiwanAwos::AVG_WIND_SPD_2MIN_VAR_NAME = "avg_wind_speed_2_min";
const string TaiwanAwos::AVG_WIND_DIR_2MIN_VAR_NAME = "avg_wind_dir_2_min";
const string TaiwanAwos::MAX_WIND_SPD_2MIN_VAR_NAME = "max_wind_speed_2_min";
const string TaiwanAwos::MIN_WIND_SPD_2MIN_VAR_NAME = "min_wind_speed_2_min";
const string TaiwanAwos::MAX_WIND_DIR_2MIN_VAR_NAME = "max_wind_dir_2_min";
const string TaiwanAwos::MIN_WIND_DIR_2MIN_VAR_NAME = "min_wind_dir_2_min";
const string TaiwanAwos::AVG_WIND_SPD_10MIN_VAR_NAME = "avg_wind_speed_10_min";
const string TaiwanAwos::AVG_WIND_DIR_10MIN_VAR_NAME = "avg_wind_dir_10_min";
const string TaiwanAwos::MAX_WIND_SPD_10MIN_VAR_NAME = "max_wind_speed_10_min";
const string TaiwanAwos::MIN_WIND_SPD_10MIN_VAR_NAME = "min_wind_speed_10_min";
const string TaiwanAwos::MAX_WIND_DIR_10MIN_VAR_NAME = "max_wind_dir_10_min";
const string TaiwanAwos::MIN_WIND_DIR_10MIN_VAR_NAME = "min_wind_dir_10_min";
const string TaiwanAwos::QNH_VAR_NAME = "qnh";
const string TaiwanAwos::QFF_VAR_NAME = "qff";
const string TaiwanAwos::QFE_VAR_NAME = "qfe";
const string TaiwanAwos::RVR_VAR_NAME = "rvr";
const string TaiwanAwos::MIN_RVR_VAR_NAME = "min_rvr";
const string TaiwanAwos::RVR_10MIN_AVG_VAR_NAME = "rvr_10_min_avg";
const string TaiwanAwos::MIN_RVR_10MIN_AVG_VAR_NAME = "min_rvr_10_min_avg";
const string TaiwanAwos::VIS_VAR_NAME = "vis";
const string TaiwanAwos::MIN_VIS_VAR_NAME = "min_vis";
const string TaiwanAwos::VIS_10MIN_AVG_VAR_NAME = "vis_10_min_avg";
const string TaiwanAwos::MIN_VIS_10MIN_AVG_VAR_NAME = "min_vis_10_min_avg";
const string TaiwanAwos::TEMPERATURE_VAR_NAME = "temperature";
const string TaiwanAwos::DEWPOINT_VAR_NAME = "dewpoint";
const string TaiwanAwos::HUMIDITY_VAR_NAME = "humidity";
const string TaiwanAwos::RAINFALL_ACC_1HR_VAR_NAME = "rainfall_acc_1_hr";
const string TaiwanAwos::RAINFALL_ACC_6HR_VAR_NAME = "rainfall_acc_6_hr";
const string TaiwanAwos::RAINFALL_ACC_12HR_VAR_NAME = "rainfall_acc_12_hr";
const string TaiwanAwos::RAINFALL_ACC_24HR_VAR_NAME = "rainfall_acc_24_hr";
const string TaiwanAwos::LOW_CLOUDINESS_VAR_NAME = "low_cloudiness";
const string TaiwanAwos::MED_CLOUDINESS_VAR_NAME = "med_cloudiness";
const string TaiwanAwos::HIGH_CLOUDINESS_VAR_NAME = "high_cloudiness";
const string TaiwanAwos::LOW_CLOUD_HT_VAR_NAME = "low_cloud_hgt";
const string TaiwanAwos::MED_CLOUD_HT_VAR_NAME = "med_cloud_hgt";
const string TaiwanAwos::HIGH_CLOUD_HT_VAR_NAME = "high_cloud_hgt";
const string TaiwanAwos::MIN_LOW_CLOUD_HT_VAR_NAME = "min_low_cloud_hgt";
const string TaiwanAwos::MIN_MED_CLOUD_HT_VAR_NAME = "min_med_cloud_hgt";
const string TaiwanAwos::MIN_HIGH_CLOUD_HT_VAR_NAME = "min_high_cloud_hgt";

const string TaiwanAwos::COORDINATES_ATTR_STRING =
  VALID_TIME_VAR_NAME + " " + LATITUDE_VAR_NAME + " " + LONGITUDE_VAR_NAME +
  " " + ALTITUDE_VAR_NAME;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

TaiwanAwos::TaiwanAwos() :
  Feature()
{
  clear();
}

TaiwanAwos::TaiwanAwos(const TaiwanAwos& from) :
  Feature()
{
  _copy(from);
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
TaiwanAwos::~TaiwanAwos()
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::operator=
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

TaiwanAwos&  
TaiwanAwos::operator=(const TaiwanAwos& from)
{
  if(this == &from) {
    return *this;
  }

  _copy(from);

  return *this;
}
/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::check
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool 
TaiwanAwos::check() const
{
  _errStr = "ERROR - TaiwanAwos::check()\n";
  
  if(_airportId.size() == 0) {
    _errStr += " The airport ID must be set.\n";
    return false;
  }

  if(_awosId.size() == 0) {
    _errStr += " The AOWS ID must be set.\n";
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::clear
//
// Description:	reseot or clear the object
//
// Returns:	none
//
// Notes:	clear the position info. because the possibility exists
//		that one object could be used for multiple sites
//
//

void 
TaiwanAwos::clear(bool only_obs)
{
  // allow retension of definition members
  if(only_obs == false) {
    _airportId = "";
    _awosId = "";
    _info = "";
    _latitude = missingVal;
    _longitude = missingVal;
    _altitude = missingVal; 
  }

  _validTime = 0;
  _qnh = missingVal;
  _qff = missingVal;
  _qfe = missingVal;
  _rvr = missingVal;
  _minimumRvr = false; 
  _avgRvr10Min = missingVal; 
  _minimumAvgRvr10Min = false; 
  _vis = missingVal;
  _minimumVis = false; 
  _avgVis10Min = missingVal; 
  _minimumAvgVis10Min = false; 
  _temperature = missingVal;
  _dewpoint = missingVal;
  _humidity = missingVal;
  _rainfallAcc1Hr = missingVal;
  _rainfallAcc6Hr = missingVal;
  _rainfallAcc12Hr = missingVal;
  _rainfallAcc24Hr = missingVal;
  _lowCloudiness = (int)missingVal;
  _medCloudiness = (int)missingVal;
  _highCloudiness = (int)missingVal;
  _lowCloudHgt = missingVal;
  _medCloudHgt = missingVal;
  _highCloudHgt = missingVal;
  _minimumLowCloudHgt = false;
  _minimumMedCloudHgt = false;
  _minimumHighCloudHgt = false;
  _instantWindSpeed = missingVal;
  _instantWindDir = missingVal;
  _avgWindSpeed2Min = missingVal;
  _avgWindDir2Min = missingVal;
  _maxWindSpeed2Min = missingVal;
  _minWindSpeed2Min = missingVal;
  _maxWindDir2Min = missingVal;
  _minWindDir2Min = missingVal;
  _avgWindSpeed10Min = missingVal;
  _avgWindDir10Min = missingVal;
  _maxWindSpeed10Min = missingVal;
  _minWindSpeed10Min = missingVal;
  _maxWindDir10Min = missingVal;
  _minWindDir10Min = missingVal;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::disassemble
//
// Description:	Disassembles a buffer, sets the object values.
//
// Returns:	Returns 0 on success, -1 on failure
//  out << "  visibility: " <<  _visibility << endl;

// Notes:	Handles byte swapping.
//
//

int
TaiwanAwos::disassemble(const void *buf, int len)
{
  clear();
  _errStr = "ERROR - TaiwanAwos::disassemble()\n";
  
  // check minimum len for header
  
  if (len < (int) sizeof(awos_header_t)) {
    TaStr::AddInt(_errStr, "  Buffer too short for header, len: ", len);
    TaStr::AddInt(_errStr, "  Minimum valid len: ",
		  sizeof(awos_header_t));
    return -1;
  }
  
  // local copy of buffer

  _memBuf.free();
  _memBuf.add(buf, len);
  
  // get header
  
  awos_header_t *hdr = (awos_header_t *) _memBuf.getPtr();
  _BE_to_header(*hdr);

  hdr->airport_id[hdr->airport_id_len - 1] = '\0';
  _airportId = hdr->airport_id;

  hdr->awos_id[hdr->awos_id_len - 1] = '\0';
  _awosId = hdr->awos_id;

  hdr->info[hdr->info_len - 1] = '\0';
  _info = hdr->info;

  _latitude = hdr->latitude;
  _longitude = hdr->longitude;
  _altitude = hdr->altitude;

  // check expected len
  
  if (len != hdr->buf_len) {
    TaStr::AddInt(_errStr, "  Buffer wrong length, len: ", len);
    TaStr::AddInt(_errStr, "  Expected len: ", hdr->buf_len);
    return -1;
  }
  
  // data values

  awos_obs_t *obs = (awos_obs_t *) ((char *) _memBuf.getPtr() + sizeof(awos_header_t));
  _BE_to_obs(*obs);

  _validTime = (time_t) obs->valid_time;
  _qnh = (float) obs->qnh;
  _qff = (float) obs->qff;
  _qfe = (float) obs->qfe;
  _rvr = (float) obs->rvr; 
  _minimumRvr = (bool) obs->min_rvr; 
  _avgRvr10Min = (float) obs->rvr_10_min_avg; 
  _minimumAvgRvr10Min = (bool) obs->min_rvr_10_min_avg; 
  _vis = (float) obs->vis; 
  _minimumVis = (bool) obs->min_vis; 
  _avgVis10Min = (float) obs->vis_10_min_avg; 
  _minimumAvgVis10Min = (bool) obs->min_vis_10_min_avg; 
  _temperature = (float) obs->temperature;
  _dewpoint = (float) obs->dewpoint;
  _humidity = (float) obs->humidity;
  _rainfallAcc1Hr = (float) obs->rainfall_acc_1_hr;
  _rainfallAcc6Hr = (float) obs->rainfall_acc_1_hr;
  _rainfallAcc12Hr = (float) obs->rainfall_acc_1_hr;
  _rainfallAcc24Hr = (float) obs->rainfall_acc_1_hr;
  _lowCloudiness = (int) obs->low_cloudiness;
  _medCloudiness = (int) obs->med_cloudiness;
  _highCloudiness = (int) obs->high_cloudiness;
  _lowCloudHgt = (float) obs->low_cloud_hgt;
  _medCloudHgt = (float) obs->med_cloud_hgt;
  _highCloudHgt = (float) obs->high_cloud_hgt;
  _minimumLowCloudHgt = (bool) obs->min_low_cloud_hgt;
  _minimumMedCloudHgt = (bool) obs->min_med_cloud_hgt;
  _minimumHighCloudHgt = (bool) obs->min_high_cloud_hgt;
  _instantWindSpeed = (float) obs->instant_wind_speed;
  _instantWindDir = (float) obs->instant_wind_dir;
  _avgWindSpeed2Min = (float) obs->avg_wind_speed_2_min;
  _avgWindDir2Min = (float) obs->avg_wind_dir_2_min;
  _maxWindSpeed2Min = (float) obs->max_wind_speed_2_min;
  _minWindSpeed2Min = (float) obs->min_wind_speed_2_min;
  _maxWindDir2Min = (float) obs->max_wind_dir_2_min;
  _minWindDir2Min = (float) obs->min_wind_dir_2_min;
  _avgWindSpeed10Min = (float) obs->avg_wind_speed_10_min;
  _avgWindDir10Min = (float) obs->avg_wind_dir_10_min;
  _maxWindSpeed10Min = (float) obs->max_wind_speed_10_min;
  _minWindSpeed10Min = (float) obs->min_wind_speed_10_min;
  _maxWindDir10Min = (float) obs->max_wind_dir_10_min;
  _minWindDir10Min = (float) obs->min_wind_dir_10_min;
  
  return 0;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::assemble
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

int 
TaiwanAwos::assemble()
{
  // check if we have a valid object

  if (!check()) {
    _errStr += "ERROR - TaiwanAwos::assemble()\n";
    return -1;
  }


  awos_header_t hdr;
  MEM_zero(hdr);

  STRcopy(hdr.airport_id, _airportId.c_str(), AIRPORT_ID_STR_LEN);
  STRcopy(hdr.awos_id, _awosId.c_str(), AWOS_ID_STR_LEN);
  STRcopy(hdr.info, _info.c_str(), INFO_STR_LEN);
  hdr.latitude = (fl32) _latitude;
  hdr.longitude = (fl32) _longitude;
  hdr.altitude = (fl32) _altitude;
  hdr.buf_len = sizeof(awos_header_t) + sizeof(awos_obs_t);
  hdr.airport_id_len = (si32) _airportId.size() + 1;
  hdr.awos_id_len = (si32) _awosId.size() + 1;
  hdr.info_len = (si32) _info.size() + 1;

  _BE_from_header(hdr);

  awos_obs_t obs;
  MEM_zero(obs);

  obs.valid_time = (si32) _validTime;
  obs.instant_wind_speed = (fl32) _instantWindSpeed;
  obs.instant_wind_dir = (fl32) _instantWindDir;
  obs.avg_wind_speed_2_min = (fl32) _avgWindSpeed2Min;
  obs.avg_wind_dir_2_min = (fl32) _avgWindDir2Min;
  obs.max_wind_speed_2_min = (fl32) _maxWindSpeed2Min;
  obs.min_wind_speed_2_min = (fl32) _minWindSpeed2Min;
  obs.max_wind_dir_2_min = (fl32) _maxWindDir2Min;
  obs.min_wind_dir_2_min = (fl32)_minWindDir2Min ;
  obs.avg_wind_speed_10_min = (fl32) _avgWindSpeed10Min;
  obs.avg_wind_dir_10_min = (fl32) _avgWindDir10Min;
  obs.max_wind_speed_10_min = (fl32) _maxWindSpeed10Min;
  obs.min_wind_speed_10_min = (fl32) _minWindSpeed10Min;
  obs.max_wind_dir_10_min = (fl32) _maxWindDir10Min;
  obs.min_wind_dir_10_min = (fl32) _minWindDir10Min;
  obs.qnh = (fl32) _qnh;
  obs.qff = (fl32) _qff;
  obs.qfe = (fl32) _qfe;
  obs.rvr = (fl32) _rvr; 
  obs.min_rvr = (si32) _minimumRvr; 
  obs.rvr_10_min_avg = (fl32) _avgRvr10Min;
  obs.min_rvr_10_min_avg = (si32) _minimumAvgRvr10Min;
  obs.vis = (fl32) _vis; 
  obs.min_vis = (si32) _minimumVis; 
  obs.vis_10_min_avg = (fl32) _avgVis10Min;
  obs.min_vis_10_min_avg = (si32) _minimumAvgVis10Min;
  obs.temperature = (fl32) _temperature;
  obs.dewpoint = (fl32) _dewpoint;
  obs.humidity = (fl32) _humidity;
  obs.rainfall_acc_1_hr = (fl32) _rainfallAcc1Hr;
  obs.rainfall_acc_6_hr = (fl32) _rainfallAcc6Hr;
  obs.rainfall_acc_12_hr = (fl32) _rainfallAcc12Hr;
  obs.rainfall_acc_24_hr = (fl32) _rainfallAcc24Hr;
  obs.low_cloudiness = (si32) _lowCloudiness;
  obs.med_cloudiness = (si32) _medCloudiness;
  obs.high_cloudiness = (si32) _highCloudiness;
  obs.low_cloud_hgt = (fl32) _lowCloudHgt;
  obs.med_cloud_hgt = (fl32) _medCloudHgt;
  obs.high_cloud_hgt = (fl32) _highCloudHgt;
  obs.min_low_cloud_hgt = (si32) _minimumLowCloudHgt;
  obs.min_med_cloud_hgt = (si32) _minimumMedCloudHgt;
  obs.min_high_cloud_hgt = (si32) _minimumHighCloudHgt;

  _BE_from_obs(obs);

  // assemble buffer
  _memBuf.free();
  _memBuf.add(&hdr, sizeof(awos_header_t));
  _memBuf.add(&obs, sizeof(awos_obs_t));

  return 0;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::print
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::print(FILE *out) const
{
  fprintf(out, "  ==================================\n");
  fprintf(out, "  TaiwanAwos - Taiwan ASOS/AWOS data\n");
  fprintf(out, "  ==================================\n");
  fprintf(out, "  airportId: %s\n", _airportId.c_str());
  fprintf(out, "  awosId :%s\n", _awosId.c_str());
  fprintf(out, "  info: %s\n", _info.c_str());
  fprintf(out, "  latitude: %f\n",  _latitude);
  fprintf(out, "  longitude: %f\n",  _longitude);
  fprintf(out, "  altitude: %f\n",  _altitude);
  fprintf(out, "  validTime: %s\n", utimstr(_validTime));
  fprintf(out, "  qnh: %f\n",  _qnh);
  fprintf(out, "  qff: %f\n",  _qff);
  fprintf(out, "  qfe: %f\n",  _qfe);
  fprintf(out, "  rvr: %f(%d)\n",  _rvr, _minimumRvr); 
  fprintf(out, "  10-min average rvr: %f(%d)\n",  _avgRvr10Min,  _minimumAvgRvr10Min); 
  fprintf(out, "  vis: %f(%d)\n",  _vis, _minimumVis); 
  fprintf(out, "  10-min average vis: %f(%d)\n",  _avgVis10Min,  _minimumAvgVis10Min); 
  fprintf(out, "  temperature: %f\n",  _temperature);
  fprintf(out, "  dewpoint: %f\n",  _dewpoint);
  fprintf(out, "  humidity: %f\n",  _humidity);
  fprintf(out, "  instant wind speed: %f\n", _instantWindSpeed );
  fprintf(out, "  instant wind direction: %f\n", _instantWindDir );
  fprintf(out, "  Two minute wind speed:\n");
  fprintf(out, "  average   max    min\n");
  fprintf(out, "   %f      %f      %f\n",  _avgWindSpeed2Min, _maxWindSpeed2Min,
	  _minWindSpeed2Min);
  fprintf(out, "  Two minute wind direction:\n");
  fprintf(out, "  average   max    min\n");
  fprintf(out, "   %f      %f     %f\n",  _avgWindDir2Min, 
	  _maxWindDir2Min, _minWindDir2Min);
  fprintf(out, "  Ten minute wind speed:\n");
  fprintf(out, "  average   max    min\n");
  fprintf(out, "   %f      %f       %f\n",  _avgWindSpeed10Min, _maxWindSpeed10Min,
	  _minWindSpeed10Min);
  fprintf(out, "  Ten minute wind direction:\n");
  fprintf(out, "  average   max    min\n");
  fprintf(out, "   %f      %f     %f\n",  _avgWindDir10Min, 
	  _maxWindDir10Min, _minWindDir10Min);
  fprintf(out, "  Rainfall accumulations:\n");
  fprintf(out, "  1-hr    6-hr    12-hr    24-hr \n");
  fprintf(out, "   %f      %f     %f     %f\n",  _rainfallAcc1Hr,
	  _rainfallAcc6Hr, _rainfallAcc12Hr, _rainfallAcc24Hr);
  fprintf(out, "  Cloudiness:\n");
  fprintf(out, "  low      med      high\n");
  fprintf(out, "   %d      %d     %d\n", _lowCloudiness, 
	  _medCloudiness, _highCloudiness);
  fprintf(out, "  Cloud height:\n");
  fprintf(out, "     low          med         high\n");
  fprintf(out, "   %f(%d)    %f(%d)     %f(%d)\n", _lowCloudHgt, _minimumLowCloudHgt, 
	  _medCloudHgt, _minimumMedCloudHgt, _highCloudHgt, _minimumHighCloudHgt);

}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::print
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::print(ostream &out) const
{
  out << "  ==================================" << endl;
  out << "  TaiwanAwos - Taiwan ASOS/AWOS data" << endl;
  out << "  ==================================" << endl;
  out << "  airportId: " << _airportId << endl;
  out << "  awosId : " << _awosId << endl;
  out << "  info: " << _info << endl;
  out << "  latitude: " <<  _latitude << endl;
  out << "  longitude: " <<  _longitude << endl;
  out << "  altitude: " <<  _altitude << endl;
  out << "  validTime: " << utimstr(_validTime) << endl;
  out << "  qnh: " <<  _qnh << endl;
  out << "  qff: " <<  _qff << endl;
  out << "  qfe: " <<  _qfe << endl;
  out << "  rvr: " <<  _rvr << "(" << _minimumRvr << ")" << endl; 
  out << "  10-min average rvr: " <<  _avgRvr10Min << "(" << _minimumAvgRvr10Min << ")" << endl; 
  out << "  vis: " <<  _vis << "(" << _minimumVis << ")" << endl; 
  out << "  10-min average vis: " <<  _avgVis10Min << "(" << _minimumAvgVis10Min << ")" << endl; 
  out << "  temperature: " <<  _temperature << endl;
  out << "  dewpoint: " <<  _dewpoint << endl;
  out << "  humidity: " <<  _humidity << endl;
  out << "  instant wind speed: " << _instantWindSpeed << endl;
  out << "  instant wind direction: " << _instantWindDir << endl;
  out << "  Two minute wind speed:" << endl;
  out << "  average    max    min" << endl;
  out << "   " << _avgWindSpeed2Min << "     " << 
    _maxWindSpeed2Min << "     " <<  _minWindSpeed2Min << endl;
  out << "  Two minute wind direction:" << endl;
  out << "  average   max    min" << endl;
  out << "   " << _avgWindDir2Min << "      " << 
    _maxWindDir2Min << "     " << _minWindDir2Min << endl;
  out << "  Ten minute wind speed:" << endl;
  out << "  average   max    min" << endl;
  out << "   " << _avgWindSpeed10Min << "      " << 
    _maxWindSpeed10Min << "      " << _minWindSpeed10Min << endl;
  out << "  Ten minute wind direction:" << endl;
  out << "  average   max    min" << endl;
  out << "   " << _avgWindDir10Min << "      " << 
    _maxWindDir10Min << "     " << _minWindDir10Min<< endl;

  out << "  Rainfall accumulations:" << endl;
  out << "  1-hr    6-hr    12-hr    24-hr" << endl;
  out << "   " << _rainfallAcc1Hr << "     " << _rainfallAcc6Hr << "     " 
      << _rainfallAcc12Hr << "     " << _rainfallAcc24Hr << endl;
  out << "  Cloudiness:" << endl;
  out << "  low      med      high" << endl;
  out << "   " << _lowCloudiness << "     " << _medCloudiness << "     " 
      << _highCloudiness  << endl;
  out << "  Cloud height:"   << endl;
  out << "  low      med      high"   << endl;
  out << "   " << _lowCloudHgt << "(" <<  _minimumLowCloudHgt <<  ")    "<< _medCloudHgt << "(" 
      << _minimumMedCloudHgt << ")     "<< _highCloudHgt << "(" << _minimumHighCloudHgt << ")" << endl;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::fromSpdbChunk
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool TaiwanAwos::fromSpdbChunk(const Spdb::chunk_t &chunk)
{
  // Disassemble the buffer directly in the chunk

  if (disassemble(chunk.data, chunk.len) == 0)
    return true;

  return false;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::toNetcdf
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool TaiwanAwos::toNetcdf(FeatureNcFile &nc_file) const
{
  static const string method_name = "WxObs::toNetcdf()";
  
  // Initialize the netCDF file
  
  if (!_initFeatureNcFileForOutput(nc_file))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing feature netCDF file for output" << endl;
    
    return false;
  }
  
  // Get the current number of records so we know where to add the new values

  size_t num_records = nc_file.getNumRecords();

  // Write the record values

  if (!nc_file.appendValue(AIRPORT_ID_VAR_NAME, getAirportId(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending airport id to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(AWOS_ID_VAR_NAME, getId(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending awos id to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(INFO_VAR_NAME, getInfo(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending info to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(LATITUDE_VAR_NAME, getLatitude(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending latitude to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(LONGITUDE_VAR_NAME, getLongitude(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending longitude to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(ALTITUDE_VAR_NAME, getAltitude(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending altitude to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(VALID_TIME_VAR_NAME, (double)getValidTime(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending valid time to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(INSTANT_WIND_SPEED_VAR_NAME, getInstantWindSpeed(),
			   num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending instant wind speed to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(INSTANT_WIND_DIR_VAR_NAME, getInstantWindDir(),
			   num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending instant wind dir to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(AVG_WIND_SPD_2MIN_VAR_NAME, getAvgWindSpeed2Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending avg wind speed 2 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(AVG_WIND_DIR_2MIN_VAR_NAME, getAvgWindDir2Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending avg wind dir 2 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MAX_WIND_SPD_2MIN_VAR_NAME, getMaxWindSpeed2Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending max wind speed 2 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_WIND_SPD_2MIN_VAR_NAME, getMinWindSpeed2Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending min wind speed 2 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MAX_WIND_DIR_2MIN_VAR_NAME, getMaxWindDir2Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending max wind dir 2 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_WIND_DIR_2MIN_VAR_NAME, getMinWindDir2Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending min wind dir 2 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(AVG_WIND_SPD_10MIN_VAR_NAME, getAvgWindSpeed10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending avg wind speed 10 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(AVG_WIND_DIR_10MIN_VAR_NAME, getAvgWindDir10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending avg wind dir 10 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MAX_WIND_SPD_10MIN_VAR_NAME, getMaxWindSpeed10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending max wind speed 10 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_WIND_SPD_10MIN_VAR_NAME, getMinWindSpeed10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending min wind speed 10 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MAX_WIND_DIR_10MIN_VAR_NAME, getMaxWindDir10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending max wind dir 10 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_WIND_DIR_10MIN_VAR_NAME, getMinWindDir10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending min wind dir 10 min to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(QNH_VAR_NAME, getQnh(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending qnh to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(QFF_VAR_NAME, getQff(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending qff to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(QFE_VAR_NAME, getQfe(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending qfe to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(RVR_VAR_NAME, getRvr(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending rvr to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_RVR_VAR_NAME, getMinimumRvr(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum rvr to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(RVR_10MIN_AVG_VAR_NAME, getAvgRvr10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending rvr 10 min avg to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_RVR_10MIN_AVG_VAR_NAME, getMinimumAvgRvr10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum rvr 10 min avg to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(VIS_VAR_NAME, getVis(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending vis to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_VIS_VAR_NAME, getMinimumVis(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum vis to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(VIS_10MIN_AVG_VAR_NAME, getAvgVis10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending vis 10 min avg to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_VIS_10MIN_AVG_VAR_NAME,
			   getMinimumAvgVis10Min(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum vis 10 min avg to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(TEMPERATURE_VAR_NAME, getTemperature(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending temperature to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(DEWPOINT_VAR_NAME, getDewpoint(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending dewpoint to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(HUMIDITY_VAR_NAME, getHumidity(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending humidity to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(RAINFALL_ACC_1HR_VAR_NAME, getRainfallAcc1Hr(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending rainfall acc 1 hr to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(RAINFALL_ACC_6HR_VAR_NAME, getRainfallAcc6Hr(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending rainfall acc 6 hr to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(RAINFALL_ACC_12HR_VAR_NAME, getRainfallAcc12Hr(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending rainfall acc 12 hr to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(RAINFALL_ACC_24HR_VAR_NAME, getRainfallAcc24Hr(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending rainfall acc 24 hr to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(LOW_CLOUDINESS_VAR_NAME, getLowCloudiness(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending low cloudiness to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MED_CLOUDINESS_VAR_NAME, getMedCloudiness(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending med cloudiness to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(HIGH_CLOUDINESS_VAR_NAME, getHighCloudiness(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending high cloudiness to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(LOW_CLOUD_HT_VAR_NAME, getLowCloudHeight(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending low cloud height to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MED_CLOUD_HT_VAR_NAME, getMedCloudHeight(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending med cloud height to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(HIGH_CLOUD_HT_VAR_NAME, getHighCloudHeight(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending high cloud height to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_LOW_CLOUD_HT_VAR_NAME,
			   getMinimumLowCloudHeight(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum low cloud height to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_MED_CLOUD_HT_VAR_NAME,
			   getMinimumMedCloudHeight(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum med cloud height to netCDF file" << endl;
    
    return false;
  }
  
  if (!nc_file.appendValue(MIN_HIGH_CLOUD_HT_VAR_NAME,
			   getMinimumHighCloudHeight(), num_records))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error appending minimum high cloud height to netCDF file" << endl;
    
    return false;
  }
  
  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::_initFeatureNcFileForOutput
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool TaiwanAwos::_initFeatureNcFileForOutput(FeatureNcFile &nc_file) const
{
  static const string method_name = "WxObs::_initFeatureNcFileForOutput()";
  
  // If the file is already initialized, we don't need to do anything

  if (nc_file.isVarListInitialized())
    return true;
  
  // Create the needed variables

  if (!nc_file.addVar(VALID_TIME_VAR_NAME,
		      VALID_TIME_VAR_NAME,
		      VALID_TIME_VAR_NAME,
		      ncDouble,
		      "seconds since 1970-01-01 00:00:00"))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for valid time" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(LATITUDE_VAR_NAME,
		      LATITUDE_VAR_NAME,
		      LATITUDE_VAR_NAME,
		      ncDouble,
		      "degrees_north"))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for latitude" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(LONGITUDE_VAR_NAME,
		      LONGITUDE_VAR_NAME,
		      LONGITUDE_VAR_NAME,
		      ncDouble,
		      "degrees_east"))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for longitude" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addAltVar(ALTITUDE_VAR_NAME,
			 ALTITUDE_VAR_NAME,
			 ALTITUDE_VAR_NAME,
			 ncDouble,
			 "m"))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for elevation" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(AIRPORT_ID_VAR_NAME,
		      AIRPORT_ID_VAR_NAME,
		      AIRPORT_ID_VAR_NAME,
		      ncString,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for airport id" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(AWOS_ID_VAR_NAME,
		      AWOS_ID_VAR_NAME,
		      AWOS_ID_VAR_NAME,
		      ncString,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for awos id" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }

  if (!nc_file.addVar(INFO_VAR_NAME,
		      INFO_VAR_NAME,
		      INFO_VAR_NAME,
		      ncString,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for info" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(INSTANT_WIND_SPEED_VAR_NAME,
		      INSTANT_WIND_SPEED_VAR_NAME,
		      INSTANT_WIND_SPEED_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for instant wind speed" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(INSTANT_WIND_DIR_VAR_NAME,
		      INSTANT_WIND_DIR_VAR_NAME,
		      INSTANT_WIND_DIR_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for instant wind dir" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(AVG_WIND_SPD_2MIN_VAR_NAME,
		      AVG_WIND_SPD_2MIN_VAR_NAME,
		      AVG_WIND_SPD_2MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for avg wind speed 2 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(AVG_WIND_DIR_2MIN_VAR_NAME,
		      AVG_WIND_DIR_2MIN_VAR_NAME,
		      AVG_WIND_DIR_2MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for avg wind dir 2 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MAX_WIND_SPD_2MIN_VAR_NAME,
		      MAX_WIND_SPD_2MIN_VAR_NAME,
		      MAX_WIND_SPD_2MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for max wind speed 2 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_WIND_SPD_2MIN_VAR_NAME,
		      MIN_WIND_SPD_2MIN_VAR_NAME,
		      MIN_WIND_SPD_2MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min wind speed 2 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MAX_WIND_DIR_2MIN_VAR_NAME,
		      MAX_WIND_DIR_2MIN_VAR_NAME,
		      MAX_WIND_DIR_2MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for max wind dir 2 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_WIND_DIR_2MIN_VAR_NAME,
		      MIN_WIND_DIR_2MIN_VAR_NAME,
		      MIN_WIND_DIR_2MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min wind dir 2 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(AVG_WIND_SPD_10MIN_VAR_NAME,
		      AVG_WIND_SPD_10MIN_VAR_NAME,
		      AVG_WIND_SPD_10MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for avg wind speed 10 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(AVG_WIND_DIR_10MIN_VAR_NAME,
		      AVG_WIND_DIR_10MIN_VAR_NAME,
		      AVG_WIND_DIR_10MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for avg wind dir 10 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MAX_WIND_SPD_10MIN_VAR_NAME,
		      MAX_WIND_SPD_10MIN_VAR_NAME,
		      MAX_WIND_SPD_10MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for max wind speed 10 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_WIND_SPD_10MIN_VAR_NAME,
		      MIN_WIND_SPD_10MIN_VAR_NAME,
		      MIN_WIND_SPD_10MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min wind speed 10 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MAX_WIND_DIR_10MIN_VAR_NAME,
		      MAX_WIND_DIR_10MIN_VAR_NAME,
		      MAX_WIND_DIR_10MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for max wind dir 10 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_WIND_DIR_10MIN_VAR_NAME,
		      MIN_WIND_DIR_10MIN_VAR_NAME,
		      MIN_WIND_DIR_10MIN_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min wind dir 10 min" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(QNH_VAR_NAME,
		      QNH_VAR_NAME,
		      QNH_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for qnh" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(QFF_VAR_NAME,
		      QFF_VAR_NAME,
		      QFF_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for qff" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(QFE_VAR_NAME,
		      QFE_VAR_NAME,
		      QFE_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for qfe" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(RVR_VAR_NAME,
		      RVR_VAR_NAME,
		      RVR_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for rvr" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_RVR_VAR_NAME,
		      MIN_RVR_VAR_NAME,
		      MIN_RVR_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min rvr" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(RVR_10MIN_AVG_VAR_NAME,
		      RVR_10MIN_AVG_VAR_NAME,
		      RVR_10MIN_AVG_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for rvr 10 min avg" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_RVR_10MIN_AVG_VAR_NAME,
		      MIN_RVR_10MIN_AVG_VAR_NAME,
		      MIN_RVR_10MIN_AVG_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min rvr 10 min avg" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(VIS_VAR_NAME,
		      VIS_VAR_NAME,
		      VIS_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for vis" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_VIS_VAR_NAME,
		      MIN_VIS_VAR_NAME,
		      MIN_VIS_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min vis" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(VIS_10MIN_AVG_VAR_NAME,
		      VIS_10MIN_AVG_VAR_NAME,
		      VIS_10MIN_AVG_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for vis 10 min avg" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_VIS_10MIN_AVG_VAR_NAME,
		      MIN_VIS_10MIN_AVG_VAR_NAME,
		      MIN_VIS_10MIN_AVG_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min vis 10 min avg" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(TEMPERATURE_VAR_NAME,
		      TEMPERATURE_VAR_NAME,
		      TEMPERATURE_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for temperature" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(DEWPOINT_VAR_NAME,
		      DEWPOINT_VAR_NAME,
		      DEWPOINT_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for dewpoint" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(HUMIDITY_VAR_NAME,
		      HUMIDITY_VAR_NAME,
		      HUMIDITY_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for humidity" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(RAINFALL_ACC_1HR_VAR_NAME,
		      RAINFALL_ACC_1HR_VAR_NAME,
		      RAINFALL_ACC_1HR_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for rainfall acc 1 hr" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(RAINFALL_ACC_6HR_VAR_NAME,
		      RAINFALL_ACC_6HR_VAR_NAME,
		      RAINFALL_ACC_6HR_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for rainfall acc 6 hr" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(RAINFALL_ACC_12HR_VAR_NAME,
		      RAINFALL_ACC_12HR_VAR_NAME,
		      RAINFALL_ACC_12HR_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for rainfall acc 12 hr" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(RAINFALL_ACC_24HR_VAR_NAME,
		      RAINFALL_ACC_24HR_VAR_NAME,
		      RAINFALL_ACC_24HR_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for rainfall acc 24 hr" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(LOW_CLOUDINESS_VAR_NAME,
		      LOW_CLOUDINESS_VAR_NAME,
		      LOW_CLOUDINESS_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for low cloudiness" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MED_CLOUDINESS_VAR_NAME,
		      MED_CLOUDINESS_VAR_NAME,
		      MED_CLOUDINESS_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for med cloudiness" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(HIGH_CLOUDINESS_VAR_NAME,
		      HIGH_CLOUDINESS_VAR_NAME,
		      HIGH_CLOUDINESS_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for high cloudiness" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(LOW_CLOUD_HT_VAR_NAME,
		      LOW_CLOUD_HT_VAR_NAME,
		      LOW_CLOUD_HT_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for low cloud ht" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MED_CLOUD_HT_VAR_NAME,
		      MED_CLOUD_HT_VAR_NAME,
		      MED_CLOUD_HT_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for med cloud ht" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(HIGH_CLOUD_HT_VAR_NAME,
		      HIGH_CLOUD_HT_VAR_NAME,
		      HIGH_CLOUD_HT_VAR_NAME,
		      ncDouble,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for high cloud ht" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_LOW_CLOUD_HT_VAR_NAME,
		      MIN_LOW_CLOUD_HT_VAR_NAME,
		      MIN_LOW_CLOUD_HT_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min low cloud ht" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_MED_CLOUD_HT_VAR_NAME,
		      MIN_MED_CLOUD_HT_VAR_NAME,
		      MIN_MED_CLOUD_HT_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min med cloud ht" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  if (!nc_file.addVar(MIN_HIGH_CLOUD_HT_VAR_NAME,
		      MIN_HIGH_CLOUD_HT_VAR_NAME,
		      MIN_HIGH_CLOUD_HT_VAR_NAME,
		      ncInt,
		      "",
		      COORDINATES_ATTR_STRING))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating netCDF variable for min high cloud ht" << endl;
    cerr << nc_file.getErrStr() << endl;
    
    return false;
  }
  
  // Set the flag so we know the file was initialized

  nc_file.setVarListInitialized(true);
  
  return true;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::_BE_from_header
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::_BE_from_header(awos_header_t &hdr)
{
  char* start32 =  ((char *) &hdr) + HEADER_32_OFFSET;
  BE_from_array_32(start32, HEADER_32_SIZE);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::_BE_to_header
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::_BE_to_header(awos_header_t &hdr)
{
  char* start32 =  ((char *) &hdr) + HEADER_32_OFFSET;
  BE_to_array_32(start32, HEADER_32_SIZE);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::_BE_from_obs
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::_BE_from_obs(awos_obs_t &obs)
{
  BE_from_array_32(&obs, sizeof(awos_obs_t));

}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::_BE_to_obs
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::_BE_to_obs(awos_obs_t &obs)
{
  BE_to_array_32(&obs, sizeof(awos_obs_t));
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	TaiwanAwos::_copy
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
TaiwanAwos::_copy(const TaiwanAwos &from)
{
  this->_airportId = from._airportId;
  this->_awosId = from._awosId;
  this->_info = from._info;
  this->_latitude = from._latitude;
  this->_longitude = from._longitude;
  this->_altitude = from._altitude;
  this->_validTime = from._validTime;
  this->_qnh = from._qnh;
  this->_qff = from._qff;
  this->_qfe = from._qfe;
  this->_rvr = from._rvr;
  this->_minimumRvr = from._minimumRvr;
  this->_avgRvr10Min = from._avgRvr10Min;
  this->_minimumAvgRvr10Min = from._minimumAvgRvr10Min;
  this->_vis = from._vis;
  this->_minimumVis = from._minimumVis;
  this->_avgVis10Min = from._avgVis10Min;
  this->_minimumAvgVis10Min = from._minimumAvgVis10Min;
  this->_temperature = from._temperature;
  this->_dewpoint = from._dewpoint;
  this->_humidity = from._humidity;
  this->_rainfallAcc1Hr = from._rainfallAcc1Hr;
  this->_rainfallAcc1Hr = from._rainfallAcc1Hr;
  this->_rainfallAcc1Hr = from._rainfallAcc1Hr;
  this->_rainfallAcc1Hr = from._rainfallAcc1Hr;
  this->_lowCloudiness = from._lowCloudiness;
  this->_medCloudiness = from._medCloudiness;
  this->_highCloudiness = from._highCloudiness;
  this->_lowCloudHgt = from._lowCloudHgt;
  this->_medCloudHgt = from._medCloudHgt;
  this->_highCloudHgt = from._highCloudHgt;
  this->_minimumLowCloudHgt = from._minimumLowCloudHgt;
  this->_minimumMedCloudHgt = from._minimumMedCloudHgt;
  this->_minimumHighCloudHgt = from._minimumHighCloudHgt;
  this->_instantWindSpeed = from._instantWindSpeed;
  this->_instantWindDir = from._instantWindDir;
  this->_avgWindSpeed2Min = from._avgWindSpeed2Min;
  this->_avgWindDir2Min = from._avgWindDir2Min;
  this->_maxWindSpeed2Min = from._maxWindSpeed2Min;
  this->_minWindSpeed2Min = from._minWindSpeed2Min;
  this->_maxWindDir2Min = from._maxWindDir2Min;
  this->_minWindDir2Min = from._minWindDir2Min;
  this->_avgWindSpeed10Min = from._avgWindSpeed10Min;
  this->_avgWindDir10Min = from._avgWindDir10Min;
  this->_maxWindSpeed10Min = from._maxWindSpeed10Min;
  this->_minWindSpeed10Min = from._minWindSpeed10Min;
  this->_maxWindDir10Min = from._maxWindDir10Min;
  this->_minWindDir10Min = from._minWindDir10Min;
}



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Helper Functions
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	operator==
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool  
operator==(const TaiwanAwos &left, 
	   const TaiwanAwos &right)
{
  
  if((left._airportId == right._airportId) &&
     (left._awosId == right._awosId) &&
     (left._info == right._info) &&
     (left._awosId == right._awosId) &&
     (left._info == right._info) &&
     (left._latitude == right._latitude) &&
     (left._longitude == right._longitude) &&
     (left._altitude == right._altitude) &&
     (left._validTime == right._validTime) &&
     (left._qnh == right._qnh) &&
     (left._qff == right._qff) &&
     (left._qfe == right._qfe) &&
     (left._rvr == right._rvr) && 
     (left._minimumRvr == right._minimumRvr) && 
     (left._avgRvr10Min == right._avgRvr10Min) && 
     (left._minimumAvgRvr10Min == right._minimumAvgRvr10Min) && 
     (left._vis == right._vis) && 
     (left._minimumVis == right._minimumVis) && 
     (left._avgVis10Min == right._avgVis10Min) && 
     (left._minimumAvgVis10Min == right._minimumAvgVis10Min) && 
     (left._temperature == right._temperature) &&
     (left._dewpoint == right._dewpoint) &&
     (left._humidity == right._humidity) &&
     (left._avgWindSpeed2Min == right._avgWindSpeed2Min) &&
     (left._avgWindDir2Min == right._avgWindDir2Min) &&
     (left._maxWindSpeed2Min == right._maxWindSpeed2Min) &&
     (left._minWindSpeed2Min == right._minWindSpeed2Min) &&
     (left._maxWindDir2Min == right._maxWindDir2Min) &&
     (left._minWindDir2Min == right._minWindDir2Min) &&
     (left._avgWindSpeed10Min == right._avgWindSpeed10Min) &&
     (left._avgWindDir10Min == right._avgWindDir10Min) &&
     (left._maxWindSpeed10Min == right._maxWindSpeed10Min) &&
     (left._minWindSpeed10Min == right._minWindSpeed10Min) &&
     (left._maxWindDir10Min == right._maxWindDir10Min) &&
     (left._minWindDir10Min == right._minWindDir10Min) &&
     (left._instantWindSpeed == right._instantWindSpeed) &&
     (left._instantWindDir == right._instantWindDir)) {
    
    return true;
  }

  return false;
}

