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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:23:06 $
 *   $Id: TamdarObs.hh,v 1.2 2016/03/07 01:23:06 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TamdarObs: Class that manipulates a TAMDAR observation.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TamdarObs_H
#define TamdarObs_H

#include <rapformats/Sndg.hh>
#include <toolsa/DateTime.hh>

using namespace std;


class TamdarObs
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    ROLL_UNKNOWN,
    ROLL_G,
    ROLL_B
  } roll_flag_t;
  

  typedef enum
  {
    ICING_UNKNOWN,
    ICING_D,
    ICING_I,
    ICING_H
  } icing_flag_t;
  

  typedef enum
  {
    PHASE_UNKNOWN,
    PHASE_ASCENT,
    PHASE_DESCENT
  } flight_phase_t;
  
  
  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructors
   */

  TamdarObs(char *data_line, const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~TamdarObs();


  ////////////////////
  // Output methods //
  ////////////////////

  /*********************************************************************
   * print() - Print the object values to the given stream
   */

  void print(ostream &out) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getAirSpeed() - Retrieve the air speec in knots.
   */

  double getAirSpeed() const
  {
    return _airSpeed;
  }
  

  /*********************************************************************
   * getEddyDissipation() - Retrieve the eddy dissipation rate.
   */

  double getEddyDissipation() const
  {
    return _eddyDissipation;
  }
  

  /*********************************************************************
   * getGpsAlt() - Retrieve the GPS altitude in feet.
   */

  double getGpsAlt() const
  {
    return _gpsAlt;
  }
  

  /*********************************************************************
   * getFlightId() - Retrieve the flight ID value.
   */

  int getFlightId() const
  {
    return _flightId;
  }
  

  /*********************************************************************
   * getFlightPhase() - Retrieve the flight phase value.
   */

  flight_phase_t getFlightPhase() const
  {
    return _flightPhase;
  }
  

  /*********************************************************************
   * getIcingFlag() - Retrieve the icing value.
   */

  icing_flag_t getIcingFlag() const
  {
    return _icingFlag;
  }
  

  /*********************************************************************
   * getLatitude() - Retrieve the latitude.
   */

  double getLatitude() const
  {
    return _latitude;
  }
  

  /*********************************************************************
   * getLocation() - Retrieve the location.
   */

  void getLocation(double &lat, double &lon) const
  {
    lat = _latitude;
    lon = _longitude;
  }
  

  /*********************************************************************
   * getLongitude() - Retrieve the longitude.
   */

  double getLongitude() const
  {
    return _longitude;
  }
  

  /*********************************************************************
   * getObsTime() - Retrieve the observation time.
   */

  DateTime getObsTime() const
  {
    return _obsTime;
  }
  

  /*********************************************************************
   * getPeakEddyTime() - Retrieve the time of the peak eddy dissipation
   *                     rate.
   */

  double getPeakEddyTime() const
  {
    return _peakEddyTime;
  }
  

  /*********************************************************************
   * getPresAlt() - Retrieve the pressure altitude in feet.
   */

  double getPresAlt() const
  {
    return _presAlt;
  }
  

  /*********************************************************************
   * getRelHum() - Retrieve the relative humidity value in % (0-100).
   */

  double getRelHum() const
  {
    return _rh;
  }
  

  /*********************************************************************
   * getRelHumUncertainty() - Retrieve the relative humidity uncertainty
   *                          value in % (0-100).
   */

  double getRelHumUncertainty() const
  {
    return _rhUncertainty;
  }
  

  /*********************************************************************
   * getRollFlag() - Retrieve the roll flag.
   */

  roll_flag_t getRollFlag() const
  {
    return _rollFlag;
  }
  

  /*********************************************************************
   * getSerialNum() - Retrieve the serial number.
   */

  int getSerialNum() const
  {
    return _serialNum;
  }
  

  /*********************************************************************
   * getSoundingId() - Retrieve the sounding ID value.
   */

  int getSoundingId() const
  {
    return _soundingId;
  }
  

  /*********************************************************************
   * getSoundingStartTime() - Retrieve the sounding start time.
   */

  DateTime getSoundingStartTime() const
  {
    return _soundingStartTime;
  }
  

  /*********************************************************************
   * getSoundingEndTime() - Retrieve the sounding end time.
   */

  DateTime getSoundingEndTime() const
  {
    return _soundingEndTime;
  }
  

  /*********************************************************************
   * getTemperature() - Retrieve the temperature in C.
   */

  double getTemperature() const
  {
    return _temperature;
  }
  

  /*********************************************************************
   * getWindDir() - Retrieve the wind direction in degrees north.
   */

  double getWindDir() const
  {
    return _windDir;
  }
  

  /*********************************************************************
   * getWindSpeed() - Retrieve the wind speed in knots.
   */

  double getWindSpeed() const
  {
    return _windSpeed;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  static const int MAX_INPUT_LINE_LEN;
  
  typedef enum
  {
    OBS_DATE_TOKEN_NUM,
    OBS_TIME_TOKEN_NUM,
    LAT_DIR_TOKEN_NUM,
    LAT_DEG_TOKEN_NUM,
    LAT_MIN_TOKEN_NUM,
    LON_DIR_TOKEN_NUM,
    LON_DEG_TOKEN_NUM,
    LON_MIN_TOKEN_NUM,
    PRES_ALT_TOKEN_NUM,
    GPS_ALT_TOKEN_NUM,
    SERIAL_NUM_TOKEN_NUM,
    AIRSPEED_TOKEN_NUM,
    TEMPERATURE_TOKEN_NUM,
    WIND_DIR_TOKEN_NUM,
    WIND_SPEED_TOKEN_NUM,
    ROLL_FLAG_TOKEN_NUM,
    EDDY_DISS_TOKEN_NUM,
    PEAK_EDDY_TIME_TOKEN_NUM,
    ICING_TOKEN_NUM,
    RH_TOKEN_NUM,
    RH_UNCERTAINTY_TOKEN_NUM,
    FLIGHT_ID_TOKEN_NUM,
    SOUNDING_ID_TOKEN_NUM,
    FLIGHT_PHASE_TOKEN_NUM,
    SOUNDING_START_DATE_TOKEN_NUM,
    SOUNDING_START_TIME_TOKEN_NUM,
    SOUNDING_END_DATE_TOKEN_NUM,
    SOUNDING_END_TIME_TOKEN_NUM,
    NUM_TOKENS
  } input_tokens_t;
  
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  DateTime _obsTime;
  double _latitude;
  double _longitude;
  double _presAlt;           // feet
  double _gpsAlt;            // feet
  int _serialNum;
  double _airSpeed;          // knots
  double _temperature;       // C
  double _windDir;           // degN
  double _windSpeed;         // knots
  roll_flag_t _rollFlag;
  double _eddyDissipation;
  double _peakEddyTime;
  icing_flag_t _icingFlag;
  double _rh;                // 0-100 %
  double _rhUncertainty;     // 0-100 %
  int _flightId;
  int _soundingId;
  flight_phase_t _flightPhase;
  DateTime _soundingStartTime;
  DateTime _soundingEndTime;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _extractData() - Extract the data values from the input file line.
   *
   * Returns true on success, false on failure.
   */

  bool _extractData(char *data_line);
  

  /*********************************************************************
   * _setAirspeed() - Set the airspeed value based on the given token
   *                  value.
   */

  void _setAirspeed(const string airspeed_token);
  

  /*********************************************************************
   * _setEddyDissipation() - Set the eddy dissipation value based on the
   *                         given token value.
   */

  void _setEddyDissipation(const string eddy_token);
  

  /*********************************************************************
   * _setFlightId() - Set the flight ID value based on the given token value.
   */

  void _setFlightId(const string flight_id_token);
  

  /*********************************************************************
   * _setFlightPhase() - Set the flight phase value based on the given
   *                     token value.
   */

  void _setFlightPhase(const string flight_phase_token);
  

  /*********************************************************************
   * _setGpsAlt() - Set the GPS altitude value based on the given token
   *                value.
   */

  void _setGpsAlt(const string gps_alt_token);
  

  /*********************************************************************
   * _setIcing() - Set the icing value based on the given token value.
   */

  void _setIcing(const string icing_token);
  

  /*********************************************************************
   * _setLat() - Set the latitude value based on the given token values.
   */

  void _setLat(const string dir_token,
	       const string deg_token,
	       const string min_token);
  

  /*********************************************************************
   * _setLon() - Set the longitude value based on the given token values.
   */

  void _setLon(const string dir_token,
	       const string deg_token,
	       const string min_token);
  

  /*********************************************************************
   * _setObsTime() - Set the observation time value based on the given
   *                 token values.
   */

  void _setObsTime(const string obs_date_token,
		   const string obs_time_token);
  

  /*********************************************************************
   * _setPeakEddyTime() - Set the time of peak eddy dissipation rate value
   *                      based on the given token value.
   */

  void _setPeakEddyTime(const string peak_eddy_token);
  

  /*********************************************************************
   * _setPresAlt() - Set the pressure altitude value based on the given
   *                 token value.
   */

  void _setPresAlt(const string pres_alt_token);
  

  /*********************************************************************
   * _setRh() - Set the relative humidity value based on the given token
   *            value.
   */

  void _setRh(const string rh_token);
  

  /*********************************************************************
   * _setRhUncertainty() - Set the relative humidity uncertainty value
   *                       based on the given token value.
   */

  void _setRhUncertainty(const string rh_token);
  

  /*********************************************************************
   * _setRollFlag() - Set the roll flag value based on the given token
   *                  value.
   */

  void _setRollFlag(const string roll_flag_token);
  

  /*********************************************************************
   * _setSerialNum() - Set the serial number value based on the given token
   *                   value.
   */

  void _setSerialNum(const string serial_num_token);
  

  /*********************************************************************
   * _setSoundingEndTime() - Set the sounding end time value based on
   *                         the given token values.
   */

  void _setSoundingEndTime(const string date_token,
			   const string time_token);
  

  /*********************************************************************
   * _setSoundingId() - Set the sounding ID value based on the given
   *                    token value.
   */

  void _setSoundingId(const string sndg_id_token);
  

  /*********************************************************************
   * _setSoundingStartTime() - Set the sounding start time value based on
   *                           the given token values.
   */

  void _setSoundingStartTime(const string date_token,
			     const string time_token);
  

  /*********************************************************************
   * _setTemperature() - Set the temperature value based on the given
   *                     token value.
   */

  void _setTemperature(const string temp_token);
  

  /*********************************************************************
   * _setWindDir() - Set the wind direction value based on the given
   *                 token value.
   */

  void _setWindDir(const string wind_dir_token);
  

  /*********************************************************************
   * _setWindSpeed() - Set the wind speed value based on the given token
   *                   value.
   */

  void _setWindSpeed(const string wind_speed_token);
  

};

#endif
