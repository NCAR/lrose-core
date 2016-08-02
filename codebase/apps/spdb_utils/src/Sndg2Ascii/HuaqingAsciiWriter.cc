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
//   $Date: 2016/03/07 01:39:56 $
//   $Id: HuaqingAsciiWriter.cc,v 1.2 2016/03/07 01:39:56 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * HuaqingAsciiWriter: Class that writes Sndg information to a Huaqing
 *                     format ASCII file.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/DateTime.hh>

#include "HuaqingAsciiWriter.hh"

using namespace std;


const double HuaqingAsciiWriter::MISSING_DATA_VALUE = -999.0;

/*********************************************************************
 * Constructors
 */

HuaqingAsciiWriter::HuaqingAsciiWriter(const bool debug_flag) :
  AsciiWriter(debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

HuaqingAsciiWriter::~HuaqingAsciiWriter()
{
  // Do nothing
}


/*********************************************************************
 * writeSndg() - Write the sounding information to the ASCII file.
 */

void HuaqingAsciiWriter::writeSndg(const Sndg &sounding)
{
  // Write the sounding header information to the output file
    
  Sndg::header_t sndg_hdr = sounding.getHeader();
    
  DateTime sndg_time(sndg_hdr.launchTime);
    
  char output_string[BUFSIZ];
  
  sprintf(output_string, "%s   %02d %02d %02d %02d %02d   %f %f",
	  sndg_hdr.siteName,
	  sndg_time.getMonth(), sndg_time.getDay(),
	  sndg_time.getHour(), sndg_time.getMin(), sndg_time.getSec(),
	  sndg_hdr.lat, sndg_hdr.lon);
    
  if (_debug)
    cerr << output_string << endl;
  
  fprintf(_asciiFile, "%s\n", output_string);

  // Write each sounding level to the output file

  const vector< Sndg::point_t > points = sounding.getPoints();
  
  vector< Sndg::point_t >::const_iterator sndg_pt;
  
  for (sndg_pt = points.begin(); sndg_pt != points.end(); ++sndg_pt)
  {
    Sndg::point_t output_pt = *sndg_pt;
    
    // Replace the Sndg missing data values with the values expected
    // in the output file

    if (output_pt.pressure == Sndg::VALUE_UNKNOWN)
      output_pt.pressure = MISSING_DATA_VALUE;
    if (output_pt.temp == Sndg::VALUE_UNKNOWN)
      output_pt.temp = MISSING_DATA_VALUE;
    if (output_pt.dewpt == Sndg::VALUE_UNKNOWN)
      output_pt.dewpt = MISSING_DATA_VALUE;
    if (output_pt.windSpeed == Sndg::VALUE_UNKNOWN)
      output_pt.windSpeed = MISSING_DATA_VALUE;
    if (output_pt.windDir == Sndg::VALUE_UNKNOWN)
      output_pt.windDir = MISSING_DATA_VALUE;
    if (output_pt.altitude == Sndg::VALUE_UNKNOWN)
      output_pt.altitude = MISSING_DATA_VALUE;
    
    // Create the output line

    sprintf(output_string,
	    "     %6.1f %6.1f %6.1f %6.1f %6.1f %6.1f",
	    output_pt.pressure, output_pt.temp, output_pt.dewpt,
	    output_pt.windSpeed, output_pt.windDir, output_pt.altitude);
    
    if (_debug)
      cerr << output_string << endl;
    
    fprintf(_asciiFile, "%s\n", output_string);
    
  } /* endfor - sndg_pt */
  
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
