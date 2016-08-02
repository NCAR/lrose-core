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
 *  $Id: BufrDecoder.cc,v 1.17 2016/03/07 01:22:59 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	BufrDecoder
//
// Author:	G. M. cunning
//
// Date:	Mon Mar 19 11:00 2012
//
// Description: Decoder subclass that handles BUFR-formatted messages 
//		following FM 94. See WMO AMDAR reference manual.
// 


// C++ include files
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

// System/RAP include files
#include <toolsa/DateTime.hh>

// Local include files
#include "BufrDecoder.hh"

using namespace std;

// define any constants
const string BufrDecoder::_className = "BufrDecoder";

const string BufrDecoder::_endSectionID = "7777";

const int BufrDecoder::_linesReport1 = 19;
const int BufrDecoder::_linesReport2 = 25;
const int BufrDecoder::_linesReport3 = 22;

const int BufrDecoder::_dataWidthOnRight = 15;

// a bunch of strings that are the FXY values found in decbufr
// output. use the strings to match values to variables

const string BufrDecoder::_missingStr = "missing";

// different report types
const string BufrDecoder::_reportType1FXY = "3 11   5";
const string BufrDecoder::_reportType2FXY = "3 11   1"; 
const string BufrDecoder::_reportType3FXY = "3 11   8"; 

// FXY associated with report support
const string BufrDecoder::_obsSequenceNumFXY = "0  1  23"; // goes with _reportType1FXY
const string BufrDecoder::_aircraftNavSystemFXY = "0  2  61"; // goes with _reportType2FXY
const string BufrDecoder::_tailNumberFXY = "0  1   8"; // goes with _reportType2FXY

// FXY associated with date & time -- used in all 3 report types
const string BufrDecoder::_yearFXY = "0  4   1";
const string BufrDecoder::_monthFXY = "0  4   2";
const string BufrDecoder::_dayFXY = "0  4   3";
const string BufrDecoder::_hourFXY = "0  4   4";
const string BufrDecoder::_minuteFXY = "0  4   5";
const string BufrDecoder::_secondFXY = "0  4   6";

// FXY associated with position
const string BufrDecoder::_latitudeFXY = "0  5   1"; // goes in all 3
const string BufrDecoder::_longitudeFXY = "0  6   1";  // goes in all 3
const string BufrDecoder::_flightLevelFXY = "0  7  10"; 
const string BufrDecoder::_altitudeFXY = "0  7   2"; // goes with _reportType2FXY
const string BufrDecoder::_detailedPhaseOfFlightFXY = "0  8   9"; // goes with _reportType1FXY
const string BufrDecoder::_phaseOfFlightFXY = "0  8   4";

// FXY associated with weather 
const string BufrDecoder::_windDirFXY = "0 11   1";
const string BufrDecoder::_windspeedFXY = "0 11   2";
const string BufrDecoder::_temperatureFXY = "0 12   1";
const string BufrDecoder::_airTemperatureFXY = "0 12 101";
const string BufrDecoder::_dewpointTemperatureFXY = "0 12 103";

// FXY associated with turbulence
const string BufrDecoder::_degreeOfTubulenceFXY = "0 11  31"; // goes with _reportType1FXY
const string BufrDecoder::_maxVertGustFXY = "0 11  36"; // goes with _reportType1FXY
const string BufrDecoder::_degreeOfTurbulenceFXY = "0 11  31";
const string BufrDecoder::_turbBaseHeightFXY = "0 11  32";
const string BufrDecoder::_turbTopHeightFXY = "0 11  33";
const string BufrDecoder::_peakTurbulenceIntensityFXY = "0 11  76";

// FXY associated with airframe icing 
const string BufrDecoder::_airframeIcingFXY = "0 20  41";

// FXY associated with naviagtion system
const string BufrDecoder::_rollAngleQualityFXY = "0  2  64";
const string BufrDecoder::_temperaturePrecisionFXY = "0  2   5";
const string BufrDecoder::_aircraftDataRelaySysFXY = "0  2  62";
const string BufrDecoder::_aircraftNavSysFXY = "0  2  61";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

BufrDecoder::BufrDecoder() :
  Decoder()
{
  _setID();

  // for debugging
  _amdarCounter = 0;


}

BufrDecoder::BufrDecoder(const Params *params) :
  Decoder(params)
{
  _setID();

  // for debugging
  _amdarCounter = 0;

}

BufrDecoder::BufrDecoder(const BufrDecoder &from) :
  Decoder(from)
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
  
BufrDecoder::~BufrDecoder()
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
// checkFormat
//
BufrDecoder::msg_type_t 
BufrDecoder::checkFormat(const string &hdr, const string &msg)
{
  if(hdr.find(_wmoId) != string::npos && msg.find(_typeId) != string::npos) {
    return MSG_TYPE_BUFR;
  }

  return MSG_TYPE_UNKNOWN;
} 

/////////////////////////////////////////////////////////////////////////
// process
//
int 
BufrDecoder::process(const string& amdar_str, vector<Amdar*>& amdars)
{
  const string methodName = _className + string( "::process" );

  // set the appropriate decoder for message
  if(amdar_str.find(_endSectionID) == string::npos) {
    if(static_cast<int>(_params->debug) > static_cast<int>(Params::DEBUG_VERBOSE)) {
      cerr << "WARNING: " << methodName << " -- incomplete message." << endl;
    }
    return 1;
  }

  // write BUFR message to temporary file
  string bufrFilename = string("/d1/aoaws/data/tmp/amdar.bufr");
  string decodedFilename = string("/d1/aoaws/data/tmp/amdar.decoded");
  ofstream amdarFile(bufrFilename.c_str(), ios::binary);
  amdarFile << amdar_str;
  amdarFile.close();

  // run decbufr and write decoded message to temporary file
  string wSpace = " ";
  string toDevNull = " 2>&1 /dev/null"; // system runs command in Bourne shell
  string decbufrCommand = string(_params->decbufr_path) + string(" -d ") + 
    string(_params->table_dir) + wSpace + bufrFilename + wSpace + 
    decodedFilename + toDevNull;

  int retCode = 0;
  retCode = system(decbufrCommand.c_str());
  if(retCode == -1) {
      cerr << "WARNING: " << methodName << " -- calling decbufr failed." << endl;
      cerr << "Command: " << decbufrCommand << endl;
      return 1;
  }

  std::ifstream decodedStream;
  decodedStream.open(decodedFilename.c_str(), ifstream::in);

  if(decodedStream.good() == false) {
    cerr << "Error:: " << methodName << " -- unable to open " << 
      decodedFilename << endl;
    return 1;
  }

  vector< string > decodeStrings;
  char eol = '\n';
  while (decodedStream.eof() == false) {
    string line;
    getline(decodedStream, line, eol);
    decodeStrings.push_back(line);
  }

  decodedStream.close();

  // identify the report type and call a method to parse for each type
  //
  // the FXY value for the report type is in the the 0th string

  retCode = 0;
  vector< Amdar::bufr_std_amdar_bulletin_t > bufrAmdars;

  if(decodeStrings[0].find(_reportType1FXY) != string::npos) {
    retCode = _parseReportType1(decodeStrings, bufrAmdars);
  }
  else if(decodeStrings[0].find(_reportType2FXY) != string::npos) {
    retCode = _parseReportType2(decodeStrings, bufrAmdars);
  } 
  else if(decodeStrings[0].find(_reportType3FXY) != string::npos) {
    retCode = _parseReportType3(decodeStrings, bufrAmdars);
  }
  else {
    cerr << "Error:: " << methodName << " -- unable to match report type to parser " << endl;
    retCode = 1;
  }

  for(size_t i = 0; i < bufrAmdars.size(); i++) {
    Amdar *amdar =  new Amdar(bufrAmdars[i]);
    amdars.push_back(amdar);
  }

  return retCode;

}

/////////////////////////////////////////////////////////////////////////
// operator=
//
BufrDecoder& 
BufrDecoder::operator=(const BufrDecoder& from)
{
  if(this == &from) {
    return *this;
  }

  _copy(from);

  return *this;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// _parseReportType1
//
// Example type 1 report output from decbufr
//
//
//  3 11   5        'HL7708  '
//      1.0000000   0  1  23 Observation sequence number
//               36.0000000   0  5   1 Latitude (high accuracy)
//              126.9866600   0  6   1 Longitude (high accuracy)
//             2013.0000000   0  4   1 Year
//               10.0000000   0  4   2 Month
//               26.0000000   0  4   3 Day
//               23.0000000   0  4   4 Hour
//               39.0000000   0  4   5 Minute
//                0.0000000   0  4   6 Second
//             8324.0000000   0  7  10 Flight level
//                3.0000000   0  8   9 Detailed phase of flight
//              282.0000000   0 11   1 Wind direction
//               17.0000000   0 11   2 Wind speed
//                missing   0 11  31 Degree of turbulence
//                missing   0 11  36 Maximum derived equivalent vertical gust speed
//              232.4500000   0 12 101 Temperature/dry-bulb temperature
//                3.0000000   0 33  25 ACARS interpolated values
// 0  2  64       0.0000000            Aircraft roll angle quality
//
//
//
int 
BufrDecoder::_parseReportType1(const std::vector< std::string >& decoded_strings, 
			       vector< Amdar::bufr_std_amdar_bulletin_t >& bufr_amdars)
{
  const string methodName = _className + string( "::_parseReportType1" );

  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
    cerr << methodName << " -- in the right place for report type 1" << endl;
    cerr << decoded_strings[0] << endl;
  }

  Amdar::bufr_std_amdar_bulletin_t bufrAmdar;
  float year;
  float month;
  float day;
  float hour;
  float minute;
  float second;

  int reportLineCnt = 1;

  // start at 0 for this report because the first line has the tail number
  for(size_t i = 0; i < decoded_strings.size(); i++, reportLineCnt++) {

    size_t fxyIndex = string::npos;
    // tail number
    string tailNumber;
    fxyIndex = decoded_strings[i].find(_reportType1FXY);
    if(fxyIndex != string::npos) {

      // look for '
      size_t idx = decoded_strings[i].find("'");
      tailNumber = decoded_strings[i].substr(idx+1, 8);
      bufrAmdar.acIdent = tailNumber;
      continue;
    }

    float seqNum; // not sure how this will be used
    if(_extractFloat(decoded_strings[i], _obsSequenceNumFXY, seqNum) == 0) {
      bufrAmdar.sequenceNum = static_cast<int>(seqNum);
      continue;
    }

    float latitude;
    if(_extractFloat(decoded_strings[i], _latitudeFXY, latitude) == 0) {
      bufrAmdar.latitude = static_cast<double>(latitude);
      continue;
    }

    float longitude;
    if(_extractFloat(decoded_strings[i], _longitudeFXY, longitude) == 0) {
      bufrAmdar.longitude = static_cast<double>(longitude);
      continue;
    }

    if(_extractFloat(decoded_strings[i], _yearFXY, year) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _monthFXY, month) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _dayFXY, day) == 0) {
      continue;
    }
 
    if(_extractFloat(decoded_strings[i], _hourFXY, hour) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _minuteFXY, minute) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _secondFXY, second) == 0) {
      if(second < 0) {
	second = 0;
      }
      continue;
    }

    float flightLevel;
    if(_extractFloat(decoded_strings[i], _flightLevelFXY, flightLevel) == 0) {
      bufrAmdar.flightLevel = static_cast<double>(flightLevel);
      continue;
    }

    float phaseFlight;
    if(_extractFloat(decoded_strings[i], _detailedPhaseOfFlightFXY, phaseFlight) == 0) {
      // convert val to string here
      bufrAmdar.detailPhaseFlight = _setDetailedPhaseOfFlight(static_cast<int>(phaseFlight));
      continue;
    }

    float windDir;
    if(_extractFloat(decoded_strings[i], _windDirFXY, windDir) == 0) {
      bufrAmdar.windDirection = static_cast<double>(windDir);
      continue;
    }

    float windspeed;
    if(_extractFloat(decoded_strings[i], _windspeedFXY, windspeed) == 0) {
      bufrAmdar.windSpeed = static_cast<double>(windspeed);
      continue;
    }

    float degreeOfTubulence;
    if(_extractFloat(decoded_strings[i], _degreeOfTubulenceFXY, degreeOfTubulence) == 0) {
      // convert val to string here
      bufrAmdar.degreeTurb = static_cast<int>(degreeOfTubulence);
      continue;
    }

    float maxVertGust;
    if(_extractFloat(decoded_strings[i], _maxVertGustFXY, maxVertGust) == 0) {
      bufrAmdar.derEqVertGustVel = static_cast<double>(maxVertGust);
      continue;
    }

    float airTemperature;
    if(_extractFloat(decoded_strings[i], _airTemperatureFXY, airTemperature) == 0) {
      bufrAmdar.temperature = static_cast<double>(airTemperature);
      continue;
    }
 

    if(reportLineCnt == _linesReport1) {
      reportLineCnt = 0;
      DateTime issueTime(year, month, day, hour, minute, second);
      bufrAmdar.issueTime =  issueTime.utime();
      bufr_amdars.push_back(bufrAmdar);      
    } 

  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////
// _parseReportType2
//
// There can be more than one report of this type in one bulletin
//
// Example type 2 report output from decbufr
//
//   3 11   1        '        '
//        0.0000000   0  2  61 Aircraft navigational system
//               2013.0000000   0  4   1 Year
//                 10.0000000   0  4   2 Month
//                 26.0000000   0  4   3 Day
//                 23.0000000   0  4   4 Hour
//                 52.0000000   0  4   5 Minute
//                 40.4516700   0  5   1 Latitude (high accuracy)
//                -74.9266700   0  6   1 Longitude (high accuracy)
//                  6.0000000   0  8   4 Phase of aircraft flight
//               7610.0000000   0  7   2 Height or altitude
//                240.7000000   0 12   1 Temperature/dry-bulb temperature
//                243.0000000   0 11   1 Wind direction
//                 36.0000000   0 11   2 Wind speed
//                  missing   0 11  31 Degree of turbulence
//                  missing   0 11  32 Height of base of turbulence
//                  missing   0 11  33 Height of top of turbulence
//                  missing   0 20  41 Airframe icing
//   0  7   7       missing            Height
//   0 11  36       missing            Maximum derived equivalent vertical gust speed
//   0 11  76       missing            Peak turbulence intensity (eddy dissipation rate)
//   0  1   8        'CNJCA119'
//   0  2   5       missing            Precision of temperature observation
//   0  2  62       3.0000000            Type of aircraft data relay system
//   0  2  64       0.0000000            Aircraft roll angle quality
//  
//
int 
BufrDecoder::_parseReportType2(const std::vector< std::string >& decoded_strings, 
			       vector< Amdar::bufr_std_amdar_bulletin_t >& bufr_amdars)
{
  const string methodName = _className + string( "::_parseReportType2" );

  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
    cerr << methodName << " -- in the right place for report type 2" << endl;
    cerr << decoded_strings[0] << endl;
  }

  Amdar::bufr_std_amdar_bulletin_t bufrAmdar;
  float year;
  float month;
  float day;
  float hour;
  float minute;
  float second = 0;

  int reportLineCnt = 1;

  // start at 0 for this report because the first line has the tail number
  for(size_t i = 0; i < decoded_strings.size(); i++, reportLineCnt++) {
    
    size_t fxyIndex = string::npos;
    // tail number
    string tailNumber;
    fxyIndex = decoded_strings[i].find(_tailNumberFXY);
    if(fxyIndex != string::npos) {

      // look for '
      size_t idx = decoded_strings[i].find("'");
      tailNumber = decoded_strings[i].substr(idx+1, 8);
      bufrAmdar.acIdent = tailNumber;
      continue;
    }

    float latitude;
    if(_extractFloat(decoded_strings[i], _latitudeFXY, latitude) == 0) {
      bufrAmdar.latitude = static_cast<double>(latitude);
      continue;
    }

    float longitude;
    if(_extractFloat(decoded_strings[i], _longitudeFXY, longitude) == 0) {
      bufrAmdar.longitude = static_cast<double>(longitude);
      continue;
    }

    if(_extractFloat(decoded_strings[i], _yearFXY, year) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _monthFXY, month) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _dayFXY, day) == 0) {
      continue;
    }
 
    if(_extractFloat(decoded_strings[i], _hourFXY, hour) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _minuteFXY, minute) == 0) {
      continue;
    }

    float flightLevel;
    if(_extractFloat(decoded_strings[i], _altitudeFXY, flightLevel) == 0) {
      bufrAmdar.flightLevel = static_cast<double>(flightLevel);
      continue;
    }

    float phaseFlight;
    if(_extractFloat(decoded_strings[i], _phaseOfFlightFXY, phaseFlight) == 0) {
      // convert val to string here
      bufrAmdar.detailPhaseFlight = _setPhaseOfFlight(static_cast<int>(phaseFlight));
      continue;
    }

    float windDir;
    if(_extractFloat(decoded_strings[i], _windDirFXY, windDir) == 0) {
      bufrAmdar.windDirection = static_cast<double>(windDir);
      continue;
    }

    float windspeed;
    if(_extractFloat(decoded_strings[i], _windspeedFXY, windspeed) == 0) {
      bufrAmdar.windSpeed = static_cast<double>(windspeed);
      continue;
    }

    float degreeOfTubulence;
    if(_extractFloat(decoded_strings[i], _degreeOfTubulenceFXY, degreeOfTubulence) == 0) {
      // convert val to string here
      bufrAmdar.degreeTurb = static_cast<int>(degreeOfTubulence);
      continue;
    }

    float maxVertGust;
    if(_extractFloat(decoded_strings[i], _maxVertGustFXY, maxVertGust, false) == 0) {
      bufrAmdar.derEqVertGustVel = static_cast<double>(maxVertGust);
      continue;
    }

    float temperature;
    if(_extractFloat(decoded_strings[i], _temperatureFXY, temperature) == 0) {
      bufrAmdar.temperature = static_cast<double>(temperature);
      continue;
    }
 

    if(reportLineCnt == _linesReport2) {
      reportLineCnt = 0;
      DateTime issueTime(year, month, day, hour, minute, second);
      bufrAmdar.issueTime =  issueTime.utime();
      bufr_amdars.push_back(bufrAmdar);      
    } 

  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////
// _parseReportType3
//
// There can be more than one report of this type in one bulletin
//
// Example type 3 report output from decbufr
//
//   3 11   8        'NZL025  '
//      2013.0000000   0  4   1 Year
//                  10.0000000   0  4   2 Month
//                  26.0000000   0  4   3 Day
//                  23.0000000   0  4   4 Hour
//                  43.0000000   0  4   5 Minute
//                   missing   0  4   6 Second
//                 -29.0300000   0  5   1 Latitude (high accuracy)
//                 167.9200000   0  6   1 Longitude (high accuracy)
//                   5.0000000   0  8   4 Phase of aircraft flight
//                   1.0000000   0 31   1 Delayed descriptor replication factor
//                 304.0000000   0  7  10 Flight level
//                 136.0000000   0 11   1 Wind direction
//                   5.1000000   0 11   2 Wind speed
//                   missing   0  2  64 Aircraft roll angle quality
//                 288.9500000   0 12 101 Temperature/dry-bulb temperature
//                   missing   0 12 103 Dew-point temperature
//    0  2  61       0.0000000            Aircraft navigational system
//    0  2  62       3.0000000            Type of aircraft data relay system
//    0  2   5       1.0000000            Precision of temperature observation
//    0 11  31       0.0000000            Degree of turbulence
//    0 11  36       0.9000000            Maximum derived equivalent vertical gust speed
//
int 
BufrDecoder::_parseReportType3(const std::vector< std::string >& decoded_strings, 
			       vector< Amdar::bufr_std_amdar_bulletin_t >& bufr_amdars)
{
  const string methodName = _className + string( "::_parseReportType3" );

  if(static_cast<int>(_params->debug) >= static_cast<int>(Params::DEBUG_NORM)) {
    cerr << methodName << " -- in the right place for report type 3" << endl;
    cerr << decoded_strings[0] << endl;
  }

  Amdar::bufr_std_amdar_bulletin_t bufrAmdar;
  float year;
  float month;
  float day;
  float hour;
  float minute;
  float second;
  int reportLineCnt = 1;

  // start at 0 for this report because the first line has the tail number
  for(size_t i = 0; i < decoded_strings.size(); i++, reportLineCnt++) {
    
    size_t fxyIndex = string::npos;
    // tail number
    string tailNumber;
    fxyIndex = decoded_strings[i].find(_reportType3FXY);
    if(fxyIndex != string::npos) {

      // look for '
      size_t idx = decoded_strings[i].find("'");
      tailNumber = decoded_strings[i].substr(idx+1, 8);
      bufrAmdar.acIdent = tailNumber;
      continue;
    }

    float latitude;
    if(_extractFloat(decoded_strings[i], _latitudeFXY, latitude) == 0) {
      bufrAmdar.latitude = static_cast<double>(latitude);
      continue;
    }

    float longitude;
    if(_extractFloat(decoded_strings[i], _longitudeFXY, longitude) == 0) {
      bufrAmdar.longitude = static_cast<double>(longitude);
      continue;
    }

    if(_extractFloat(decoded_strings[i], _yearFXY, year) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _monthFXY, month) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _dayFXY, day) == 0) {
      continue;
    }
 
    if(_extractFloat(decoded_strings[i], _hourFXY, hour) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _minuteFXY, minute) == 0) {
      continue;
    }

    if(_extractFloat(decoded_strings[i], _secondFXY, second) == 0) {
      if(second < 0) {
	second = 0;
      }
      continue;
    }

    float flightLevel;
    if(_extractFloat(decoded_strings[i], _flightLevelFXY, flightLevel) == 0) {
      bufrAmdar.flightLevel = static_cast<double>(flightLevel);
      continue;
    }

    float phaseFlight;
    if(_extractFloat(decoded_strings[i], _phaseOfFlightFXY, phaseFlight) == 0) {
      // convert val to string here
      bufrAmdar.detailPhaseFlight = _setPhaseOfFlight(static_cast<int>(phaseFlight));
      continue;
    }

    float windDir;
    if(_extractFloat(decoded_strings[i], _windDirFXY, windDir) == 0) {
      bufrAmdar.windDirection = static_cast<double>(windDir);
      continue;
    }

    float windspeed;
    if(_extractFloat(decoded_strings[i], _windspeedFXY, windspeed) == 0) {
      bufrAmdar.windSpeed = static_cast<double>(windspeed);
      continue;
    }

    float degreeOfTubulence;
    if(_extractFloat(decoded_strings[i], _degreeOfTubulenceFXY, 
		     degreeOfTubulence, false) == 0) {
      // convert val to string here
      bufrAmdar.degreeTurb = static_cast<int>(degreeOfTubulence);
      continue;
    }

    float airTemperature;
    if(_extractFloat(decoded_strings[i], _airTemperatureFXY, airTemperature) == 0) {
      bufrAmdar.temperature = static_cast<double>(airTemperature);
      continue;
    }
 
    // put in struct later
    //float dewpoint;
    //if(_extractFloat(decoded_strings[i], _airTemperatureFXY, dewpoint) == 0) {
    //  bufrAmdar.dewpoint = static_cast<double>(dewpoint);
    //  continue;
    //}
 
    float maxVertGust;
    if(_extractFloat(decoded_strings[i], _maxVertGustFXY, maxVertGust, false) == 0) {
      bufrAmdar.derEqVertGustVel = static_cast<double>(maxVertGust);
      // dont't continue in this case because this element is report
    }


    if(reportLineCnt == _linesReport3) {
      reportLineCnt = 0;
      DateTime issueTime(year, month, day, hour, minute, second);
      bufrAmdar.issueTime =  issueTime.utime();
      bufr_amdars.push_back(bufrAmdar); 
    } 

  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////
// _extractFloat
//
int 
BufrDecoder::_extractFloat(const string decoded_string, const string key, 
			   float& val, bool left_side /* = true */ )
{
  size_t index = decoded_string.find(key);
  if(index != string::npos) {
    string valStr;
    if(left_side == true) {
      valStr = decoded_string.substr(0,(index-1));
    }
    else {    
      valStr = decoded_string.substr((index+key.size()+2), _dataWidthOnRight);
    }   
   
    if(valStr.find(_missingStr) != string::npos) {
      val = Amdar::MISSING_VALUE;
    }
    else {
      val = atof(valStr.c_str());
    }            
    return 0;
  }
  return 1;
}


/////////////////////////////////////////////////////////////////////////
// _setDetailedPhaseOfFlight
//
string 
BufrDecoder::_setDetailedPhaseOfFlight(int val)
{

  switch(val) {
  case 0:
  case 1:
  case 2:
    return string("UNS");
  case 3:
    return string("LVR");
  case 4:
    return string("LVW");
  case 5:
    return string("ASC");
  case 6:
    return string("DES");
  case 7:
  case 8:
  case 9:
  case 10:
    return string("ASC");
  case 11:
  case 12:
  case 13:
  case 14:
    return string("DES");
  case 15:
    return string("///");
  default:
    return string("");
  }
  
}

/////////////////////////////////////////////////////////////////////////
// _setPhaseOfFlight
//
string 
BufrDecoder::_setPhaseOfFlight(int val)
{

  switch(val) {
  case 2:
    return string("UNS");
  case 3:
    return string("LVR");
  case 4:
    return string("LVW");
  case 5:
    return string("ASC");
  case 6:
    return string("DES");
  case 7:
    return string("///");
  default:
    return string("");
  }
  
}

/////////////////////////////////////////////////////////////////////////
// _setID
//
void 
BufrDecoder::_setID()
{
  _wmoId = "IUA";
  _typeId = "BUFR";
}


/////////////////////////////////////////////////////////////////////////
// _copy
//
void 
BufrDecoder::_copy(const BufrDecoder& from)
{

}
