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
/*********************************************************************
 * ClassAsciiWriter: Class that writes Sndg information to a Class
 *                     format ASCII file.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>
#include <physics/physics.h>
#include <toolsa/DateTime.hh>

#include "ClassAsciiWriter.hh"

using namespace std;


const double ClassAsciiWriter::MISSING_DATA_VALUE = -999.0;

/*********************************************************************
 * Constructors
 */

ClassAsciiWriter::ClassAsciiWriter(const bool debug_flag) :
  AsciiWriter(debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

ClassAsciiWriter::~ClassAsciiWriter()
{
  // Do nothing
}


/*********************************************************************
 * writeSndg() - Write the sounding information to the ASCII file.
 */

void ClassAsciiWriter::writeSndg(const Sndg &sounding)
{
  // Write the sounding header information to the output file
    
  Sndg::header_t sndg_hdr = sounding.getHeader();
    
  DateTime sndg_time(sndg_hdr.launchTime);
    
  char output_string[BUFSIZ];

  double lon_fract, lon_int, lon_min;
  string lon_dir;

  lon_fract = modf( sndg_hdr.lon, &lon_int);
  lon_min = fabs(lon_fract * 60.0);
  if( lon_int < 0)
    lon_dir = "W";
  else
    lon_dir = "E";
  int lon_deg  = fabs(lon_int);
  
  double lat_fract, lat_int, lat_min;
  string lat_dir;

  lat_fract = modf( sndg_hdr.lat, &lat_int);
  lat_min = fabs(lat_fract * 60.0);
  if( lat_int < 0)
    lat_dir = "S";
  else
    lat_dir = "N";
  int lat_deg  = fabs(lat_int);

  sprintf(output_string, 
	  "%s\n%s\n%s\n%s%s\n%s%03d %4.2f\'%s %10.6f, %02d %4.2f\'%s %9.6f, %8.2f\n%s%04d, %02d, %02d, %02d:%02d:%02d\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
	  "Data Type/Direction:                       Sndg PROCESSED DATA, FileFormat 1/Ascending",
	  "File Format/Version:                       EOL Sounding Format/1.0",
	  "Project Name/Platform:                     RAL/NCAR Sndg",
	  "Launch Site:                               ", sndg_hdr.siteName,
	  "Launch Location (lon,lat,alt):             ", lon_deg, lon_min, lon_dir.c_str(), sndg_hdr.lon, 
	  lat_deg, lat_min, lat_dir.c_str(), sndg_hdr.lat, sndg_hdr.alt,
	  "UTC Launch Time (y,m,d,h,m,s):             ", sndg_time.getYear(), sndg_time.getMonth(),
	  sndg_time.getDay(), sndg_time.getHour(), sndg_time.getMin(), sndg_time.getSec(),
	  "Sonde Id/Sonde Type:                       /unknown",
	  "Reference Launch Data Source/Time:         /unknown",
	  "System Operator/Comments:                  /",
	  "Post Processing Comments:                  /",
	  "/",
	  " Time   -- UTC  --   Press    Temp   Dewpt    RH     Uwind   Vwind   Wspd     Dir     dZ    GeoPoAlt     Lon         Lat      GPSA",
	  "  sec   hh mm   ss     mb      C       C       %      m/s     m/s     m/s     deg     m/s       m        deg         deg         m",
	  "------- -- -- ----- ------- ------- ------- ------- ------- ------- ------- ------- ------- -------- ----------- ----------- -----"
	  );

  if (_debug)
    cerr << output_string << endl;
  
  fprintf(_asciiFile, "%s", output_string);

  // Write each sounding level to the output file

  const vector< Sndg::point_t > points = sounding.getPoints();
  
  vector< Sndg::point_t >::const_iterator sndg_pt;

  DateTime pt_time(sndg_hdr.launchTime);
  for (sndg_pt = points.begin(); sndg_pt != points.end(); ++sndg_pt)
  {
    Sndg::point_t output_pt = *sndg_pt;
    
    // Replace the Sndg missing data values with the values expected
    // in the output file

    int hh, mm;
    float ss;

    if (output_pt.time == Sndg::VALUE_UNKNOWN)
      output_pt.time = MISSING_DATA_VALUE;

    if (output_pt.time == MISSING_DATA_VALUE )
     {
      hh = -9;
      mm = -9;
      ss = -9.00;
    }
    else
    {
      pt_time = pt_time + output_pt.time;
      hh = pt_time.getHour();
      mm = pt_time.getMin();
      ss = pt_time.getSec();
    }

    if (output_pt.pressure == Sndg::VALUE_UNKNOWN)
      output_pt.pressure = MISSING_DATA_VALUE;

    if (output_pt.temp == Sndg::VALUE_UNKNOWN)
      output_pt.temp = MISSING_DATA_VALUE;

    double rh;
    if (output_pt.rh == Sndg::VALUE_UNKNOWN)
    {
      output_pt.rh = MISSING_DATA_VALUE;
      if(output_pt.dewpt != Sndg::VALUE_UNKNOWN &&
	 output_pt.temp != MISSING_DATA_VALUE)
	rh = PHYrelh(output_pt.temp, output_pt.dewpt);
      else
	rh = MISSING_DATA_VALUE;
    }
    else
    {
      rh = output_pt.rh;
    }

    double dewpt;
    if (output_pt.dewpt == Sndg::VALUE_UNKNOWN)
    {
      output_pt.dewpt = MISSING_DATA_VALUE;
      if(output_pt.temp != MISSING_DATA_VALUE &&
	 output_pt.rh != MISSING_DATA_VALUE)
	dewpt = PHYrhdp(output_pt.temp, output_pt.rh);
      else
	dewpt = MISSING_DATA_VALUE;
    }
    else
    {
      dewpt = output_pt.dewpt;
    }

    if (output_pt.u == Sndg::VALUE_UNKNOWN)
      output_pt.u = MISSING_DATA_VALUE;

    if (output_pt.v == Sndg::VALUE_UNKNOWN)
      output_pt.v = MISSING_DATA_VALUE;

    if (output_pt.w == Sndg::VALUE_UNKNOWN)
      output_pt.w = MISSING_DATA_VALUE;

    double wspd;
    if (output_pt.windSpeed == Sndg::VALUE_UNKNOWN)
    {
      output_pt.windSpeed = MISSING_DATA_VALUE;
      if(output_pt.u != MISSING_DATA_VALUE &&
	 output_pt.v != MISSING_DATA_VALUE)
	wspd = PHYwind_speed(output_pt.u, output_pt.v);
      else
	wspd = MISSING_DATA_VALUE;
    }
    else 
    {
      wspd = output_pt.windSpeed;
    }

    double wdir;
    if (output_pt.windDir == Sndg::VALUE_UNKNOWN)
    {
      output_pt.windDir = MISSING_DATA_VALUE;
      if(output_pt.u != MISSING_DATA_VALUE &&
	 output_pt.v != MISSING_DATA_VALUE)
	wdir = PHYwind_dir(output_pt.u, output_pt.v);
      else
	wdir = MISSING_DATA_VALUE;
    }
    else
    {
      wdir = output_pt.windDir;
    }

    if (output_pt.altitude == Sndg::VALUE_UNKNOWN)
      output_pt.altitude = MISSING_DATA_VALUE;
    else
      output_pt.altitude = output_pt.altitude + sndg_hdr.alt;

    if (output_pt.longitude == Sndg::VALUE_UNKNOWN)
      output_pt.longitude = MISSING_DATA_VALUE;

    if (output_pt.latitude == Sndg::VALUE_UNKNOWN)
      output_pt.latitude = MISSING_DATA_VALUE;
    
    // Create the output line

    /*    sprintf(output_string,
	    "%7.1f     %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f",
	    output_pt.time, output_pt.pressure, output_pt.temp, output_pt.dewpt,
	    output_pt.windSpeed, output_pt.windDir, output_pt.altitude);*/
    sprintf(output_string,
	    "%7.1f %02d %02d %5.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %8.2f %11.6f %11.6f %8.2f",
	    output_pt.time, hh, mm , ss, output_pt.pressure, output_pt.temp,
	    dewpt, rh, output_pt.u, output_pt.v, wspd,
	    wdir, MISSING_DATA_VALUE, output_pt.altitude, 
	    output_pt.latitude, output_pt.longitude, MISSING_DATA_VALUE);
    
    if (_debug)
      cerr << output_string << endl;
    
    fprintf(_asciiFile, "%s\n", output_string);
    
  } /* endfor - sndg_pt */
  
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
