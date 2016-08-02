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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:39:55 $
//   $Id: SndgReader.cc,v 1.2 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SndgReader: Class that reads the Sndg information from an old format
 *             SPDB sounding database.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Spdb/SoundingGet.hh>
#include <toolsa/str.h>

#include "SndgReader.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

SndgReader::SndgReader(const string &input_url,
			       const bool debug_flag) :
  SpdbReader(input_url, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

SndgReader::~SndgReader()
{
}


/*********************************************************************
 * readSoundings() - Read the soundings for the given time from the
 *                   SPDB database and convert them to sounding plus
 *                   format.  Return the soundings in the given vector.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool SndgReader::readSoundings(const DateTime &data_time,
				   vector< Sndg > &sndg_vector)
{
  static const string method_name = "SndgReader::readSoundings()";
  
  // Read the chunks from the database

  SoundingGet sndg_get;
  
  sndg_get.init(_inputUrl.c_str());
  
  int num_sndgs;
  
  if ((num_sndgs = sndg_get.readSounding(data_time.utime())) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting soundings from input database:" << endl;
    cerr << "   URL: " << _inputUrl << endl;
    cerr << "   data time: " << data_time << endl;
    
    return false;
  }
  
  // Process the soundings

  for (int sndg_num = 0; sndg_num < num_sndgs; ++sndg_num)
  {
    // Load the sounding information

    sndg_get.loadProduct(sndg_num);
    double missing_data_value = sndg_get.getMissingValue();
    
    // Copy the sounding information into the Sndg object

    Sndg sounding;
    
    Sndg::header_t sndg_hdr;
    memset(&sndg_hdr, 0, sizeof(sndg_hdr));
    
    sndg_hdr.launchTime = sndg_get.getLaunchTime();
    sndg_hdr.sourceId = sndg_get.getSourceId();
    sndg_hdr.lat = sndg_get.getLat();
    sndg_hdr.lon = sndg_get.getLon();
    sndg_hdr.alt = sndg_get.getAlt();
    STRcopy(sndg_hdr.sourceName, sndg_get.getSourceName(),
	    Sndg::SRC_NAME_LEN);
    STRcopy(sndg_hdr.siteName, sndg_get.getSiteName().c_str(),
	    Sndg::SITE_NAME_LEN);
    
    sounding.setHeader(sndg_hdr);
    
    double *pres = sndg_get.getPres();
    double *alts = sndg_get.getAlts();
    double *u = sndg_get.getU();
    double *v = sndg_get.getV();
    double *w = sndg_get.getW();
    double *rh = sndg_get.getRH();
    double *temp = sndg_get.getTemp();
    double *wind_speed = sndg_get.getWindSpeed();
    double *wind_dir = sndg_get.getWindDir();
    
    for (int i = 0; i < sndg_get.getNumPoints(); ++i)
    {
      Sndg::point_t point;
      memset(&point, 0, sizeof(point));
      
      if (pres == 0 || pres[i] == missing_data_value)
	point.pressure = Sndg::VALUE_UNKNOWN;
      else
	point.pressure = pres[i];
      if (alts == 0 || alts[i] == missing_data_value)
	point.altitude = Sndg::VALUE_UNKNOWN;
      else
	point.altitude = alts[i];
      if (u == 0 || u[i] == missing_data_value)
	point.u = Sndg::VALUE_UNKNOWN;
      else
	point.u = u[i];
      if (v == 0 || v[i] == missing_data_value)
	point.v = Sndg::VALUE_UNKNOWN;
      else
	point.v = v[i];
      if (w == 0 || w[i] == missing_data_value)
	point.w = Sndg::VALUE_UNKNOWN;
      else
	point.w = w[i];
      if (rh == 0 || rh[i] == missing_data_value)
	point.rh = Sndg::VALUE_UNKNOWN;
      else
	point.rh = rh[i];
      if (temp == 0 || temp[i] == missing_data_value)
	point.temp = Sndg::VALUE_UNKNOWN;
      else
	point.temp = temp[i];
      point.dewpt = Sndg::VALUE_UNKNOWN;
      if (wind_speed == 0 || wind_speed[i] == missing_data_value)
	point.windSpeed = Sndg::VALUE_UNKNOWN;
      else
	point.windSpeed = wind_speed[i];
      if (wind_dir == 0 || wind_dir[i] == missing_data_value)
	point.windDir = Sndg::VALUE_UNKNOWN;
      else
	point.windDir = wind_dir[i];
      point.ascensionRate = Sndg::VALUE_UNKNOWN;
      point.longitude = Sndg::VALUE_UNKNOWN;
      point.latitude = Sndg::VALUE_UNKNOWN;
      point.pressureQC = Sndg::VALUE_UNKNOWN;
      point.tempQC = Sndg::VALUE_UNKNOWN;
      point.humidityQC = Sndg::VALUE_UNKNOWN;
      point.uwindQC = Sndg::VALUE_UNKNOWN;
      point.vwindQC = Sndg::VALUE_UNKNOWN;
      point.ascensionRateQC = Sndg::VALUE_UNKNOWN;
      
      sounding.addPoint(point, true);
      
    } /* endfor - i */

    // Add the new sounding to the return vector

    sndg_vector.push_back(sounding);
    
  } /* endfor - sndg_num */
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
