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
//   $Id: TamdarObs.cc,v 1.4 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TamdarObs: Class that manipulates a TAMDAR observation.
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
#include <toolsa/str.h>

#include "TamdarObs.hh"

using namespace std;

const int TamdarObs::MAX_TOKENS = 50;
const int TamdarObs::MAX_TOKEN_LEN = 20;
const int TamdarObs::MAX_INPUT_LINE_LEN = 1024;


/*********************************************************************
 * Constructors
 */

TamdarObs::TamdarObs(char *data_line, const bool debug_flag) :
  _debug(debug_flag),
  _obsTime(DateTime::NEVER),
  _latitude(Sndg::VALUE_UNKNOWN),
  _longitude(Sndg::VALUE_UNKNOWN),
  _presAlt(Sndg::VALUE_UNKNOWN),
  _gpsAlt(Sndg::VALUE_UNKNOWN),
  _serialNum(0),
  _airSpeed(Sndg::VALUE_UNKNOWN),
  _temperature(Sndg::VALUE_UNKNOWN),
  _windDir(Sndg::VALUE_UNKNOWN),
  _windSpeed(Sndg::VALUE_UNKNOWN),
  _rollFlag(ROLL_UNKNOWN),
  _eddyDissipation(Sndg::VALUE_UNKNOWN),
  _peakEddyTime(Sndg::VALUE_UNKNOWN),
  _icingFlag(ICING_UNKNOWN),
  _rh(Sndg::VALUE_UNKNOWN),
  _rhUncertainty(Sndg::VALUE_UNKNOWN),
  _flightId(-1),
  _soundingId(-1),
  _flightPhase(PHASE_UNKNOWN),
  _soundingStartTime(DateTime::NEVER),
  _soundingEndTime(DateTime::NEVER)
{
  // Extract the data from the data line.  If the extraction is unsuccessful,
  // the class members will retain their default values.

  _extractData(data_line);
}

  
/*********************************************************************
 * Destructor
 */

TamdarObs::~TamdarObs()
{
}


/*********************************************************************
 * print() - Print the object values to the given stream
 */

void TamdarObs::print(ostream &out) const
{
  out << "TAMDAR Observation" << endl;
  out << "------------------" << endl;
  out << "obs time: " << _obsTime << endl;
  out << "latitude: " << _latitude << endl;
  out << "longitude: " << _longitude << endl;
  out << "pressure altitude: " << _presAlt << " feet" << endl;
  out << "GPS altitude: " << _gpsAlt << " feet" << endl;
  out << "serial number: " << _serialNum << endl;
  out << "airspeed: " << _airSpeed << " knots" << endl;
  out << "temperature: " << _temperature << " C" << endl;
  out << "wind direction: " << _windDir << " degN" << endl;
  out << "wind speed: " << _windSpeed << " knots" << endl;
  out << "roll flag: ";
  switch (_rollFlag)
  {
  case ROLL_UNKNOWN :
    out << "Unknown" << endl;
    break;
  case ROLL_G :
    out << "G" << endl;
    break;
  case ROLL_B :
    out << "B" << endl;
    break;
  }
  out << "eddy dissipation: " << _eddyDissipation << endl;
  out << "time of peak eddy dissipation: " << _peakEddyTime << endl;
  out << "icing: ";
  switch (_icingFlag)
  {
  case ICING_UNKNOWN :
    out << "Unknown" << endl;
    break;
  case ICING_D :
    out << "D" << endl;
    break;
  case ICING_I :
    out << "I" << endl;
    break;
  case ICING_H :
    out << "H" << endl;
    break;
  }
  out << "relative humidity: " << _rh << " %" << endl;
  out << "rh uncertainty: " << _rhUncertainty << " %" << endl;
  out << "flight id: " << _flightId << endl;
  out << "sounding id: " << _soundingId << endl;
  out << "flight phase: ";
  switch (_flightPhase)
  {
  case PHASE_UNKNOWN :
    out << "UNKNOWN" << endl;
    break;
  case PHASE_ASCENT :
    out << "ASCENT" << endl;
    break;
  case PHASE_DESCENT :
    out << "DESCENT" << endl;
    break;
  }
  out << "sounding start time: " << _soundingStartTime << endl;
  out << "sounding end time: " << _soundingEndTime << endl;
  
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _extractData() - Extract the data values from the input file line.
 *
 * Returns true on success, false on failure.
 */

bool TamdarObs::_extractData(char *data_line)
{
  static const string method_name = "TamdarObs::_extractData()";
  
  // Extract the tokens from the input line

  char **tokens = new char*[MAX_TOKENS];
  for (int i = 0; i < MAX_TOKENS; ++i)
    tokens[i] = new char[MAX_TOKEN_LEN];
  
  int num_tokens;
  
  num_tokens = STRparse(data_line, tokens, MAX_INPUT_LINE_LEN,
			MAX_TOKENS, MAX_TOKEN_LEN);
  
  if (num_tokens != FLIGHT_ID_TOKEN_NUM + 1 &&
      num_tokens != NUM_TOKENS)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing input line into tokens" << endl;
    cerr << "Input line: <" << data_line << ">" << endl;
    cerr << "Num tokens: " << num_tokens << endl;
    
    for (int i = 0; i < MAX_TOKENS; ++i)
      delete [] tokens[i];
    delete [] tokens;
  
    return false;
  }
  
  // Extract each of the data values from the tokens

  _setObsTime(tokens[OBS_DATE_TOKEN_NUM],
	      tokens[OBS_TIME_TOKEN_NUM]);
  _setLat(tokens[LAT_DIR_TOKEN_NUM],
	  tokens[LAT_DEG_TOKEN_NUM],
	  tokens[LAT_MIN_TOKEN_NUM]);
  _setLon(tokens[LON_DIR_TOKEN_NUM],
	  tokens[LON_DEG_TOKEN_NUM],
	  tokens[LON_MIN_TOKEN_NUM]);
  _setPresAlt(tokens[PRES_ALT_TOKEN_NUM]);
  _setGpsAlt(tokens[GPS_ALT_TOKEN_NUM]);
  _setSerialNum(tokens[SERIAL_NUM_TOKEN_NUM]);
  _setAirspeed(tokens[AIRSPEED_TOKEN_NUM]);
  _setTemperature(tokens[TEMPERATURE_TOKEN_NUM]);
  _setWindDir(tokens[WIND_DIR_TOKEN_NUM]);
  _setWindSpeed(tokens[WIND_SPEED_TOKEN_NUM]);
  _setRollFlag(tokens[ROLL_FLAG_TOKEN_NUM]);
  _setEddyDissipation(tokens[EDDY_DISS_TOKEN_NUM]);
  _setPeakEddyTime(tokens[PEAK_EDDY_TIME_TOKEN_NUM]);
  _setIcing(tokens[ICING_TOKEN_NUM]);
  _setRh(tokens[RH_TOKEN_NUM]);
  _setRhUncertainty(tokens[RH_UNCERTAINTY_TOKEN_NUM]);
  _setFlightId(tokens[FLIGHT_ID_TOKEN_NUM]);


  if (num_tokens == NUM_TOKENS)
  {
    _setSoundingId(tokens[SOUNDING_ID_TOKEN_NUM]);
    _setFlightPhase(tokens[FLIGHT_PHASE_TOKEN_NUM]);
    _setSoundingStartTime(tokens[SOUNDING_START_DATE_TOKEN_NUM],
			  tokens[SOUNDING_START_TIME_TOKEN_NUM]);
    _setSoundingEndTime(tokens[SOUNDING_END_DATE_TOKEN_NUM],
			tokens[SOUNDING_END_TIME_TOKEN_NUM]);
  }
  
  // Reclaim memory

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete [] tokens[i];
  
  delete [] tokens;
  
  return true;
}


/*********************************************************************
 * _setAirspeed() - Set the airspeed value based on the given token
 *                  value.
 */

void TamdarObs::_setAirspeed(const string airspeed_token)
{
  if (_debug)
    cerr << "airspeed token: " << airspeed_token << endl;

  if (airspeed_token == "-")
    return;
  
  _airSpeed = atof(airspeed_token.c_str());
}


/*********************************************************************
 * _setEddyDissipation() - Set the eddy dissipation value based on the
 *                         given token value.
 */

void TamdarObs::_setEddyDissipation(const string eddy_token)
{
  if (_debug)
    cerr << "eddy diss token: " << eddy_token << endl;
  
  if (eddy_token == "-")
    return;
  
  _eddyDissipation = atof(eddy_token.c_str());
}


/*********************************************************************
 * _setFlightId() - Set the flight ID value based on the given token value.
 */

void TamdarObs::_setFlightId(const string flight_id_token)
{
  if (_debug)
    cerr << "flight id token: " << flight_id_token << endl;
  
  if (flight_id_token == "-")
    return;
  
  _flightId = atoi(flight_id_token.c_str());
}


/*********************************************************************
 * _setFlightPhase() - Set the flight phase value based on the given
 *                     token value.
 */

void TamdarObs::_setFlightPhase(const string flight_phase_token)
{
  if (_debug)
    cerr << "flight phase token: " << flight_phase_token << endl;
  
  if (flight_phase_token == "-")
    return;
  
  if (flight_phase_token == "ASC" || flight_phase_token == "asc")
  {
    _flightPhase = PHASE_ASCENT;
  }
  else if (flight_phase_token == "DSC" || flight_phase_token == "dsc")
  {
    _flightPhase = PHASE_DESCENT;
  }
  else
  {
    cerr << "ERROR: Unknown flight phase token: " << flight_phase_token << endl;
    return;
  }
}


/*********************************************************************
 * _setGpsAlt() - Set the GPS altitude value based on the given token
 *                value.
 */

void TamdarObs::_setGpsAlt(const string gps_alt_token)
{
  if (_debug)
    cerr << "gps alt token: " << gps_alt_token << endl;
  
  if (gps_alt_token == "-")
    return;
  
  _gpsAlt = atof(gps_alt_token.c_str());
}


/*********************************************************************
 * _setIcing() - Set the icing value based on the given token value.
 */

void TamdarObs::_setIcing(const string icing_token)
{
  if (_debug)
    cerr << "icing token: " << icing_token << endl;
  
  if (icing_token == "-")
    return;
  
  if (icing_token == "D" || icing_token == "d")
  {
    _icingFlag = ICING_D;
  }
  else if (icing_token == "I" || icing_token == "i")
  {
    _icingFlag = ICING_I;
  }
  else if (icing_token == "H" || icing_token == "h")
  {
    _icingFlag = ICING_H;
  }
  else
  {
    cerr << "ERROR: Unknown icing flag token: " << icing_token << endl;
    return;
  }
}


/*********************************************************************
 * _setLat() - Set the latitude value based on the given token values.
 */

void TamdarObs::_setLat(const string dir_token,
			const string deg_token,
			const string min_token)
{
  if (_debug)
    cerr << "lat tokens: " << dir_token << " " << deg_token << " "
	 << min_token << endl;
  
  if (dir_token == "-" ||
      deg_token == "-" ||
      min_token == "-")
    return;
  
  double sign = 0.0;
  
  if (dir_token == "N" || dir_token == "n")
  {
    sign = 1.0;
  }
  else if (dir_token == "S" || dir_token == "s")
  {
    sign = -1.0;
  }
  else
  {
    cerr << "ERROR: Unknown lat direction: " << dir_token << endl;
    return;
  }
  
  _latitude =
    sign * (atof(deg_token.c_str()) + (atof(min_token.c_str()) / 60.0));
}


/*********************************************************************
 * _setLon() - Set the longitude value based on the given token values.
 */

void TamdarObs::_setLon(const string dir_token,
			const string deg_token,
			const string min_token)
{
  if (_debug)
    cerr << "lon tokens: " << dir_token << " " << deg_token << " "
	 << min_token << endl;
  
  if (dir_token == "-" ||
      deg_token == "-" ||
      min_token == "-")
    return;
  
  double sign = 0.0;
  
  if (dir_token == "E" || dir_token == "e")
  {
    sign = 1.0;
  }
  else if (dir_token == "W" || dir_token == "w")
  {
    sign = -1.0;
  }
  else
  {
    cerr << "ERROR: Unknown lon direction: " << dir_token << endl;
    return;
  }
  
  _longitude =
    sign * (atof(deg_token.c_str()) + (atof(min_token.c_str()) / 60.0));
}


/*********************************************************************
 * _setObsTime() - Set the observation time value based on the given
 *                 token values.
 */

void TamdarObs::_setObsTime(const string obs_date_token,
			    const string obs_time_token)
{
  if (_debug)
    cerr << "obs time tokens: " << obs_date_token << " "
	 << obs_time_token << endl;
  
  if (obs_date_token == "-" ||
      obs_time_token == "-")
    return;
  
  int year, month, day;
  
  if (sscanf(obs_date_token.c_str(), "%2d/%2d/%4d",
	     &month, &day, &year) != 3)
  {
    cerr << "ERROR: Error parsing date token: " << obs_date_token << endl;
    return;
  }
  
  int hour, minute, second;
  
  if (sscanf(obs_time_token.c_str(), "%2d:%2d:%2d",
	     &hour, &minute, &second) != 3)
  {
    cerr << "ERROR: Error parsing time token: " << obs_time_token << endl;
    return;
  }
  
  _obsTime.set(year, month, day, hour, minute, second);
}


/*********************************************************************
 * _setPeakEddyTime() - Set the time of peak eddy dissipation rate value
 *                      based on the given token value.
 */

void TamdarObs::_setPeakEddyTime(const string peak_eddy_token)
{
  if (_debug)
    cerr << "peak eddy time token: " << peak_eddy_token << endl;
  
  if (peak_eddy_token == "-")
    return;
  
  _peakEddyTime = atof(peak_eddy_token.c_str());
}


/*********************************************************************
 * _setPresAlt() - Set the pressure altitude value based on the given
 *                 token value.
 */

void TamdarObs::_setPresAlt(const string pres_alt_token)
{
  if (_debug)
    cerr << "pres alt token: " << pres_alt_token << endl;
  
  if (pres_alt_token == "-")
    return;
  
  _presAlt = atof(pres_alt_token.c_str());
}


/*********************************************************************
 * _setRh() - Set the relative humidity value based on the given token
 *            value.
 */

void TamdarObs::_setRh(const string rh_token)
{
  if (_debug)
    cerr << "rh token: " << rh_token << endl;
  
  if (rh_token == "-")
    return;
  
  _rh = atof(rh_token.c_str());
}


/*********************************************************************
 * _setRhUncertainty() - Set the relative humidity uncertainty value
 *                       based on the given token value.
 */

void TamdarObs::_setRhUncertainty(const string rh_token)
{
  if (_debug)
    cerr << "rh uncertainty token: " << rh_token << endl;
  
  if (rh_token == "-")
    return;
  
  _rhUncertainty = atof(rh_token.c_str());
}


/*********************************************************************
 * _setRollFlag() - Set the roll flag value based on the given token
 *                  value.
 */

void TamdarObs::_setRollFlag(const string roll_flag_token)
{
  if (_debug)
    cerr << "roll flag token: " << roll_flag_token << endl;
  
  if (roll_flag_token == "-")
    return;
  
  if (roll_flag_token == "G" || roll_flag_token == "g")
  {
    _rollFlag = ROLL_G;
  }
  else if (roll_flag_token == "B" || roll_flag_token == "b")
  {
    _rollFlag = ROLL_B;
  }
  else
  {
    cerr << "ERROR: Unknown roll flag: " << roll_flag_token << endl;
    return;
  }
}


/*********************************************************************
 * _setSerialNum() - Set the serial number value based on the given token
 *                   value.
 */

void TamdarObs::_setSerialNum(const string serial_num_token)
{
  if (_debug)
    cerr << "serial num token: " << serial_num_token << endl;
  
  if (serial_num_token == "-")
    return;
  
  _serialNum = atoi(serial_num_token.c_str());
}


/*********************************************************************
 * _setSoundingEndTime() - Set the sounding end time value based on
 *                         the given token values.
 */

void TamdarObs::_setSoundingEndTime(const string date_token,
				    const string time_token)
{
  if (_debug)
    cerr << "sndg end time tokens: " << date_token << " "
	 << time_token << endl;
  
  if (date_token == "-" ||
      time_token == "-")
    return;
  
  int year, month, day;
  
  if (sscanf(date_token.c_str(), "%2d/%2d/%4d",
	     &month, &day, &year) != 3)
  {
    if (sscanf(date_token.c_str(), "%4d-%2d-%2d",
	       &year, &month, &day) != 3)
      {
	cerr << "ERROR: Error parsing date token: " << date_token << endl;
	return;
      }
  }

  int hour, minute, second;
  int ms = 0;
  
  if (sscanf(time_token.c_str(), "%2d:%2d:%2d.%3d",
	     &hour, &minute, &second, &ms) != 4)
  {
    if (sscanf(time_token.c_str(), "%2d:%2d:%2d",
	       &hour, &minute, &second) != 3)
    {
      cerr << "ERROR: Error parsing time token: " << time_token << endl;
      return;
    }
  }
  
  _soundingEndTime.set(year, month, day, hour, minute, second);
}


/*********************************************************************
 * _setSoundingId() - Set the sounding ID value based on the given
 *                    token value.
 */

void TamdarObs::_setSoundingId(const string sndg_id_token)
{
  if (_debug)
    cerr << "sndg id token: " << sndg_id_token << endl;
  
  if (sndg_id_token == "-")
    return;
  
  _soundingId = atoi(sndg_id_token.c_str());
}


/*********************************************************************
 * _setSoundingStartTime() - Set the sounding start time value based on
 *                           the given token values.
 */

void TamdarObs::_setSoundingStartTime(const string date_token,
				      const string time_token)
{
  if (_debug)
    cerr << "sndg start time tokens: " << date_token << " "
	 << time_token << endl;
  
  if (date_token == "-" ||
      time_token == "-")
    return;
  
  int year, month, day;
  
  if (sscanf(date_token.c_str(), "%2d/%2d/%4d",
	     &month, &day, &year) != 3)
  {
    if (sscanf(date_token.c_str(), "%4d-%2d-%2d",
	       &year, &month, &day) != 3)
    {
      cerr << "ERROR: Error parsing date token: " << date_token << endl;
      return;
    }
  }

  int hour, minute, second;
  int ms = 0;
  
  if (sscanf(time_token.c_str(), "%2d:%2d:%2d.%3d",
	     &hour, &minute, &second, &ms) != 4)
  {
    if (sscanf(time_token.c_str(), "%2d:%2d:%2d",
	       &hour, &minute, &second) != 3)
    {
      cerr << "ERROR: Error parsing time token: " << time_token << endl;
      return;
    }
  }
  
  _soundingStartTime.set(year, month, day, hour, minute, second);
}


/*********************************************************************
 * _setTemperature() - Set the temperature value based on the given
 *                     token value.
 */

void TamdarObs::_setTemperature(const string temp_token)
{
  if (_debug)
    cerr << "temperature token: " << temp_token << endl;
  
  if (temp_token == "-")
    return;
  
  _temperature = atof(temp_token.c_str());
}


/*********************************************************************
 * _setWindDir() - Set the wind direction value based on the given
 *                 token value.
 */

void TamdarObs::_setWindDir(const string wind_dir_token)
{
  if (_debug)
    cerr << "wind dir token: " << wind_dir_token << endl;
  
  if (wind_dir_token == "-")
    return;
  
  _windDir = atof(wind_dir_token.c_str());
}


/*********************************************************************
 * _setWindSpeed() - Set the wind speed value based on the given token
 *                   value.
 */

void TamdarObs::_setWindSpeed(const string wind_speed_token)
{
  if (_debug)
    cerr << "wind speed token: " << wind_speed_token << endl;
  
  if (wind_speed_token == "-")
    return;
  
  _windSpeed = atof(wind_speed_token.c_str());
}
