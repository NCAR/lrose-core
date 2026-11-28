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
//   $Id: AncAsciiReader.cc,v 1.2 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AncAsciiReader: Class that reads Sndg information from an AutoNowcaster
 *                 format ASCII file.
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

#include "AncAsciiReader.hh"

using namespace std;


const double AncAsciiReader::MISSING_DATA_VALUE = -999.0;
const int AncAsciiReader::INPUT_LINE_LEN = 1024;

/*********************************************************************
 * Constructors
 */

AncAsciiReader::AncAsciiReader(const bool debug_flag) :
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

AncAsciiReader::~AncAsciiReader()
{
  delete [] _nextInputLine;
}


/*********************************************************************
 * getNextSndg() - Read the next sounding from the input file.
 *
 * Returns TRUE if a sounding was read, FALSE if there are no more
 * soundings in the file.
 */

bool AncAsciiReader::getNextSndg(Sndg &sounding)
{
  static const string method_name = "AncAsciiReader::getNextSndg()";
  
  map< int, Sndg >::iterator sounding_iter = _soundingList.begin();
  
  if (sounding_iter == _soundingList.end())
    return false;
  
  sounding = sounding_iter->second;
  
  map< int, Sndg >::iterator next_iter = sounding_iter;
  ++next_iter;
  
  _soundingList.erase(sounding_iter, next_iter);
  
  return true;
}


/*********************************************************************
 * openFile() - Open the input file.
 *
 * Returns true on success, false on failure.
 */

bool AncAsciiReader::openFile(const string &ascii_filepath)
{
  static const string method_name = "AncAsciiReader::openFile()";
    
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
  
  // Finally, read all of the soundings in the file and save them
  // in vectors.

  if (!_readSoundings())
    return false;
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _createNewPoint() - Create a new sounding point using the infromation
 *                     from the current TAMDAR observation.
 */

void AncAsciiReader::_createNewPoint(Sndg::point_t &new_pt,
				     const TamdarObs &obs) const
{
  static const string method_name = "AncAsciiReader::_createNewPoint()";
  
  memset(&new_pt, 0, sizeof(new_pt));
  
  double altitude_m = obs.getGpsAlt();
  if (altitude_m != Sndg::VALUE_UNKNOWN)
    altitude_m *= M_PER_FT;

  double wind_speed_ms = obs.getWindSpeed();
  if (wind_speed_ms != Sndg::VALUE_UNKNOWN)
    wind_speed_ms *= KNOTS_TO_MS;

  new_pt.time = obs.getObsTime().utime() - obs.getSoundingStartTime().utime();
  if (obs.getPresAlt() == Sndg::VALUE_UNKNOWN)
    new_pt.pressure = Sndg::VALUE_UNKNOWN;
  else
    new_pt.pressure =
      10.0 * pow(2.4081 - obs.getPresAlt() * 0.000016562, 5.2549);
  new_pt.altitude = altitude_m;
  new_pt.windSpeed = wind_speed_ms;
  new_pt.windDir = obs.getWindDir();
  if (new_pt.windSpeed == Sndg::VALUE_UNKNOWN ||
      new_pt.windDir == Sndg::VALUE_UNKNOWN)
  {
    new_pt.u = Sndg::VALUE_UNKNOWN;
    new_pt.v = Sndg::VALUE_UNKNOWN;
  }
  else
  {
    new_pt.u = PHYwind_u(wind_speed_ms, new_pt.windDir);
    new_pt.v = PHYwind_v(wind_speed_ms, new_pt.windDir);
  }
  new_pt.w = Sndg::VALUE_UNKNOWN;
  new_pt.rh = obs.getRelHum();
  new_pt.temp = obs.getTemperature();
  if (new_pt.temp == Sndg::VALUE_UNKNOWN ||
      new_pt.rh == Sndg::VALUE_UNKNOWN)
    new_pt.dewpt = Sndg::VALUE_UNKNOWN;
  else
    new_pt.dewpt = PHYrhdp(new_pt.temp,
			   new_pt.rh);
  new_pt.ascensionRate = Sndg::VALUE_UNKNOWN;
  new_pt.longitude = obs.getLongitude();
  new_pt.latitude = obs.getLatitude();
  new_pt.pressureQC = Sndg::VALUE_UNKNOWN;
  new_pt.tempQC = Sndg::VALUE_UNKNOWN;
  new_pt.humidityQC = Sndg::VALUE_UNKNOWN;
  new_pt.uwindQC = Sndg::VALUE_UNKNOWN;
  new_pt.vwindQC = Sndg::VALUE_UNKNOWN;
  new_pt.ascensionRateQC = Sndg::VALUE_UNKNOWN;
  for (int i = 0; i < Sndg::PT_SPARE_FLOATS; ++i)
    new_pt.spareFloats[i] = Sndg::VALUE_UNKNOWN;
}


/*********************************************************************
 * _createNewSounding() - Create a new sounding using the infromation
 *                        from the current TAMDAR observation.
 */

void AncAsciiReader::_createNewSounding(Sndg &new_sndg,
					const TamdarObs &obs) const
{
  static const string method_name = "AncAsciiReader::_createNewSounding()";
  
  Sndg::header_t hdr;
  memset(&hdr, 0, sizeof(hdr));
  
  double altitude_m = obs.getGpsAlt() * M_PER_FT;
  
  hdr.launchTime = obs.getSoundingStartTime().utime();
  hdr.nPoints = 0;
  hdr.sourceId = obs.getFlightId();
  hdr.leadSecs = 0;
  hdr.lat = obs.getLatitude();
  hdr.lon = obs.getLongitude();
  hdr.alt = altitude_m;
  STRcopy(hdr.sourceName, "TAMDAR", Sndg::SRC_NAME_LEN);
  STRcopy(hdr.sourceFmt, "ASCII", Sndg::SRC_FMT_LEN);
  sprintf(hdr.siteName, "%d", obs.getFlightId());
  
  new_sndg.setHeader(hdr);
  
  Sndg::point_t point;
  _createNewPoint(point, obs);
  
  new_sndg.addPoint(point, true);
}


/*********************************************************************
 * _readSoundings() - Read all of the soundings in the input file and
 *                    save them.
 *
 * Returns true on success, false on failure.
 */

bool AncAsciiReader::_readSoundings()
{
  static const string method_name = "AncAsciiReader::_readSoundings()";
    
  // Preocess each line in the file

  while (!feof(_asciiFile))
  {
    // Read the next line in the file

    if (fgets(_nextInputLine, INPUT_LINE_LEN, _asciiFile) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading data line from ASCII file: "
	   << _asciiFilePath << endl;
      cerr << "Assuming end of file..." << endl;
      
      break;
    }
  
    // Extract the data from the current input line

    TamdarObs obs(_nextInputLine, _debug);
  
    if (_debug)
      obs.print(cerr);
  
    // Add the TAMDAR obs to the appropriate sounding

    int sounding_id = obs.getSoundingId();
    
    if (sounding_id < 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Invalid sounding id: " << sounding_id << endl;
      cerr << "Skipping observation..." << endl;
      
      continue;
    }
    
    map< int, Sndg >::iterator sounding_iter;
    
    if ((sounding_iter = _soundingList.find(sounding_id))
	== _soundingList.end())
    {
      Sndg new_sndg;
      
      _createNewSounding(new_sndg, obs);
      
      _soundingList[sounding_id] = new_sndg;
    }
    else
    {
      Sndg::point_t new_pt;
      _createNewPoint(new_pt, obs);
      
      sounding_iter->second.addPoint(new_pt, true);
    }
    
  }
  
  if (_debug)
  {
    cerr << "Soundings from file " << _asciiFilePath << ":" << endl;
    
    map< int, Sndg >::const_iterator sounding_iter;
    
    for (sounding_iter = _soundingList.begin();
	 sounding_iter != _soundingList.end(); ++sounding_iter)
    {
      cerr << "Sounding ID: " << sounding_iter->first << endl;
      sounding_iter->second.print(cerr, "   ");
    }
    
  }
  
  return true;
}
