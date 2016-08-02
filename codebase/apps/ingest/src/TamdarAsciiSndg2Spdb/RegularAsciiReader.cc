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
//   $Date: 2016/03/07 01:23:06 $
//   $Id: RegularAsciiReader.cc,v 1.2 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RegularAsciiReader: Class that reads Sndg information from a regular
 *                     format ASCII file.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <physics/physics.h>
#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

#include "RegularAsciiReader.hh"
#include "TamdarObs.hh"

using namespace std;


const double RegularAsciiReader::MISSING_DATA_VALUE = -999.0;
const int RegularAsciiReader::INPUT_LINE_LEN = 1024;

/*********************************************************************
 * Constructors
 */

RegularAsciiReader::RegularAsciiReader(const bool debug_flag) :
  AsciiReader(debug_flag)
{
  // Allocate space for the next input line

  _nextInputLine = new char[INPUT_LINE_LEN];
  _nextInputLine[0] = '\0';
  cerr << "_nextInputLine = <" << _nextInputLine << ">" << endl;
}

  
/*********************************************************************
 * Destructor
 */

RegularAsciiReader::~RegularAsciiReader()
{
  delete [] _nextInputLine;
}


/*********************************************************************
 * getNextSndg() - Read the next sounding from the input file.
 *
 * Returns TRUE if a sounding was read, FALSE if there are no more
 * soundings in the file.
 */

bool RegularAsciiReader::getNextSndg(Sndg &sounding)
{
  static const string method_name = "RegularAsciiReader::getNextSndg()";
  
  // Clear out the old sounding points, just in case

  sounding.clearPoints();
  
  // Check for end of file

  if (feof(_asciiFile))
    return false;

  // Extract the data from the current input line

  TamdarObs obs(_nextInputLine, _debug);
  
  if (_debug)
    obs.print(cerr);
  
//  // Read the header from the input file
//
//  Sndg::header_t sndg_hdr;
//  memset(&sndg_hdr, 0, sizeof(sndg_hdr));
//  
//  int year, month, day, hour, minute, second;
//  
//  if (sscanf(_nextInputLine, "%s %d %d %d %d %d %f %f",
//	     sndg_hdr.siteName,
//	     &month, &day, &hour, &minute, &second,
//	     &sndg_hdr.lat, &sndg_hdr.lon) != 8)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error parsing header line from file:" << endl;
//    cerr << "   <" << _nextInputLine << ">" << endl;
//    
//    return false;
//  }
//  
//  DateTime launch_time(year, month, day,
//		       hour, minute, second);
//
//  sndg_hdr.launchTime = launch_time.utime();
//  sndg_hdr.nPoints = 0;
//  sndg_hdr.sourceId = 0;
//  sndg_hdr.leadSecs = 0;
//  if (sndg_hdr.lat == MISSING_DATA_VALUE)
//    sndg_hdr.lat = Sndg::VALUE_UNKNOWN;
//  if (sndg_hdr.lon == MISSING_DATA_VALUE)
//    sndg_hdr.lon = Sndg::VALUE_UNKNOWN;
//  sndg_hdr.alt = Sndg::VALUE_UNKNOWN;
//  for (int i = 0; i < Sndg::HDR_SPARE_FLOATS; ++i)
//    sndg_hdr.spareFloats[i] = Sndg::VALUE_UNKNOWN;
//  sndg_hdr.version = 1;
//  STRcopy(sndg_hdr.sourceName, "SndgAscii2Spdb", Sndg::SRC_NAME_LEN);
//  
//  sounding.setHeader(sndg_hdr);
//  
//  if (fgets(_nextInputLine, INPUT_LINE_LEN, _asciiFile) == 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error reading points from input file" << endl;
//    
//    return false;
//  }
//  
//  // Read the points from the input file
//
//  while (_nextInputLine[0] == ' ')
//  {
//    Sndg::point_t sndg_pt;
//    memset(&sndg_pt, 0, sizeof(sndg_pt));
//    
//    if (sscanf(_nextInputLine,"%f %f %f %f %f %f",
//	       &sndg_pt.pressure, &sndg_pt.temp, &sndg_pt.dewpt,
//	       &sndg_pt.windSpeed, &sndg_pt.windDir, &sndg_pt.altitude) != 6)
//    {
//      cerr << "ERROR: " << method_name << endl;
//      cerr << "Error parsing point line from file:" << endl;
//      cerr << "   <" << _nextInputLine << ">" << endl;
//      
//      return false;
//    }
//    
//    if (sndg_pt.pressure == MISSING_DATA_VALUE)
//      sndg_pt.pressure = Sndg::VALUE_UNKNOWN;
//    if (sndg_pt.temp == MISSING_DATA_VALUE)
//      sndg_pt.temp = Sndg::VALUE_UNKNOWN;
//    if (sndg_pt.dewpt == MISSING_DATA_VALUE)
//      sndg_pt.dewpt = Sndg::VALUE_UNKNOWN;
//    if (sndg_pt.windSpeed == MISSING_DATA_VALUE)
//      sndg_pt.windSpeed = Sndg::VALUE_UNKNOWN;
//    if (sndg_pt.windDir == MISSING_DATA_VALUE)
//      sndg_pt.windDir = Sndg::VALUE_UNKNOWN;
//    if (sndg_pt.altitude == MISSING_DATA_VALUE)
//      sndg_pt.altitude = Sndg::VALUE_UNKNOWN;
//    
//    sndg_pt.time = 0;
//    if (sndg_pt.windDir == Sndg::VALUE_UNKNOWN ||
//        sndg_pt.windSpeed == Sndg::VALUE_UNKNOWN)
//    {
//      sndg_pt.u = Sndg::VALUE_UNKNOWN;
//      sndg_pt.v = Sndg::VALUE_UNKNOWN;
//    }
//    else
//    {
//      sndg_pt.u = -sndg_pt.windSpeed * sin(sndg_pt.windDir * DEG_TO_RAD);
//      sndg_pt.v = -sndg_pt.windSpeed * cos(sndg_pt.windDir * DEG_TO_RAD);
//    }
//
//    sndg_pt.w = Sndg::VALUE_UNKNOWN;
//
//    if (sndg_pt.temp == Sndg::VALUE_UNKNOWN ||
//	sndg_pt.dewpt == Sndg::VALUE_UNKNOWN)
//      sndg_pt.rh = Sndg::VALUE_UNKNOWN;
//    else
//      sndg_pt.rh = PHYhumidity(TEMP_C_TO_K(sndg_pt.temp),
//			       TEMP_C_TO_K(sndg_pt.dewpt)) * 100.0;
//
//    sndg_pt.ascensionRate = Sndg::VALUE_UNKNOWN;
//    sndg_pt.longitude = Sndg::VALUE_UNKNOWN;
//    sndg_pt.latitude = Sndg::VALUE_UNKNOWN;
//    sndg_pt.pressureQC = Sndg::VALUE_UNKNOWN;
//    sndg_pt.tempQC = Sndg::VALUE_UNKNOWN;
//    sndg_pt.humidityQC = Sndg::VALUE_UNKNOWN;
//    sndg_pt.uwindQC = Sndg::VALUE_UNKNOWN;
//    sndg_pt.vwindQC = Sndg::VALUE_UNKNOWN;
//    sndg_pt.ascensionRateQC = Sndg::VALUE_UNKNOWN;
//    for (int i = 0; i < Sndg::PT_SPARE_FLOATS; ++i)
//      sndg_pt.spareFloats[i] = Sndg::VALUE_UNKNOWN;
//    
//    sounding.addPoint(sndg_pt, true);
//    
    if (fgets(_nextInputLine, INPUT_LINE_LEN, _asciiFile) == 0)
      return true;
//  }
  
  return true;
}


/*********************************************************************
 * openFile() - Open the input file.
 *
 * Returns true on success, false on failure.
 */

bool RegularAsciiReader::openFile(const string &ascii_filepath)
{
  static const string method_name = "RegularAsciiReader::openFile()";
    
  // First open the file as we do for all of the readers

  if (!AsciiReader::openFile(ascii_filepath))
    return false;

  // Now skip past the header and read in the first line of data

  if (fgets(_nextInputLine, INPUT_LINE_LEN, _asciiFile) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading header line from ASCII file: "
	 << ascii_filepath << endl;
    
    return false;
  }
  
  if (fgets(_nextInputLine, INPUT_LINE_LEN, _asciiFile) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading first data line from ASCII file: "
	 << ascii_filepath << endl;
    
    return false;
  }
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
