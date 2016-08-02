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
 *   $Date: 2016/03/03 19:23:53 $
 *   $Id: HydroStation.hh,v 1.4 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * HydroStation : Class for manipulating Hydrology Station data in an
 *                SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/


#ifndef HydroStation_hh
#define HydroStation_hh


#include <string>
#include <cstdio>
#include <iostream>

#include <dataport/port_types.h>
#include <toolsa/DateTime.hh>
using namespace std;


class HydroStation
{

public:

  ////////////////////
  // Public members //
  ////////////////////

  static const float DATA_NOT_AVAILABLE;
  
  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  HydroStation();


  /*********************************************************************
   * Destructor
   */

  virtual ~HydroStation();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * clear() - Clears all of the information in the station report, including
   *           the station information.
   */

  void clear()
  {
    _stationName = "";

    _latitude = 0.0;
    _longitude = 0.0;
    _altitude = 0.0;
    
    clearData();
  }
  

  /*********************************************************************
   * clearData() - Clear the data in the station report.  Leaves the station
   *               name and location alone.
   */

  void clearData()
  {
    _reportTime = DateTime::NEVER;
    
    _windSpeed = DATA_NOT_AVAILABLE;
    _windDirection = DATA_NOT_AVAILABLE;
    _temperature = DATA_NOT_AVAILABLE;
    _relHum = DATA_NOT_AVAILABLE;
    _rainfall = DATA_NOT_AVAILABLE;
    _solarRad = DATA_NOT_AVAILABLE;
    _pressure = DATA_NOT_AVAILABLE;
    _soilMoist1 = DATA_NOT_AVAILABLE;
    _soilMoist2 = DATA_NOT_AVAILABLE;
    _soilMoist3 = DATA_NOT_AVAILABLE;
    _soilMoist4 = DATA_NOT_AVAILABLE;
    _soilTemp = DATA_NOT_AVAILABLE;
  }
  

  /*********************************************************************
   * setData() - Set the data in the station report to the given values.
   */

  void setData(const time_t report_time,
	       const float wind_speed = DATA_NOT_AVAILABLE,
	       const float wind_direction = DATA_NOT_AVAILABLE,
	       const float temperature = DATA_NOT_AVAILABLE,
	       const float rel_hum = DATA_NOT_AVAILABLE,
	       const float rainfall = DATA_NOT_AVAILABLE,
	       const float solar_rad = DATA_NOT_AVAILABLE,
	       const float pressure = DATA_NOT_AVAILABLE,
	       const float soil_moist1 = DATA_NOT_AVAILABLE,
	       const float soil_moist2 = DATA_NOT_AVAILABLE,
	       const float soil_moist3 = DATA_NOT_AVAILABLE,
	       const float soil_moist4 = DATA_NOT_AVAILABLE,
	       const float soil_temp = DATA_NOT_AVAILABLE)
  {
    _reportTime.set(report_time);
    
    _windSpeed = wind_speed;
    _windDirection = wind_direction;
    _temperature = temperature;
    _relHum = rel_hum;
    _rainfall = rainfall;
    _solarRad = solar_rad;
    _pressure = pressure;
    _soilMoist1 = soil_moist1;
    _soilMoist2 = soil_moist2;
    _soilMoist3 = soil_moist3;
    _soilMoist4 = soil_moist4;
    _soilTemp = soil_temp;
  }


  void setData(const DateTime& report_time,
	       const float wind_speed = DATA_NOT_AVAILABLE,
	       const float wind_direction = DATA_NOT_AVAILABLE,
	       const float temperature = DATA_NOT_AVAILABLE,
	       const float rel_hum = DATA_NOT_AVAILABLE,
	       const float rainfall = DATA_NOT_AVAILABLE,
	       const float solar_rad = DATA_NOT_AVAILABLE,
	       const float pressure = DATA_NOT_AVAILABLE,
	       const float soil_moist1 = DATA_NOT_AVAILABLE,
	       const float soil_moist2 = DATA_NOT_AVAILABLE,
	       const float soil_moist3 = DATA_NOT_AVAILABLE,
	       const float soil_moist4 = DATA_NOT_AVAILABLE,
	       const float soil_temp = DATA_NOT_AVAILABLE)
  {
    _reportTime = report_time;
    
    _windSpeed = wind_speed;
    _windDirection = wind_direction;
    _temperature = temperature;
    _relHum = rel_hum;
    _rainfall = rainfall;
    _solarRad = solar_rad;
    _pressure = pressure;
    _soilMoist1 = soil_moist1;
    _soilMoist2 = soil_moist2;
    _soilMoist3 = soil_moist3;
    _soilMoist4 = soil_moist4;
    _soilTemp = soil_temp;
  }
  

  /*********************************************************************
   * Access the station name information.
   */

  void setStationName(const string& station_name)
  {
    _stationName = station_name;
  }

  const string& getStationName(void) const
  {
    return _stationName;
  }
  

  /*********************************************************************
   * Access the station location information.
   */

  void setLocation(const float latitude,
		   const float longitude,
		   const float altitude)
  {
    _latitude = latitude;
    _longitude = longitude;
    _altitude = altitude;
  }

  void getLocation(float& latitude,
		   float& longitude,
		   float& altitude) const
  {
    latitude = _latitude;
    longitude = _longitude;
    altitude = _altitude;
  }
  

  double getLatitude(void) const
  {
    return _latitude;
  }
  

  double getLongitude(void) const
  {
    return _longitude;
  }
  

  double getAltitude(void) const
  {
    return _altitude;
  }
  

  /*********************************************************************
   * Access the report time.
   */

  void setReportTime(const DateTime& report_time)
  {
    _reportTime = report_time;
  }

  void setReportTime(const time_t report_time)
  {
    _reportTime.set(report_time);
  }

  DateTime getReportTime(void) const
  {
    return _reportTime;
  }
  

  /*********************************************************************
   * Access the wind speed value
   */

  void setWindSpeed(const float wind_speed)
  {
    _windSpeed = wind_speed;
  }

  float getWindSpeed(void) const
  {
    return _windSpeed;
  }
  

  /*********************************************************************
   * Access the wind direction value
   */

  void setWindDirection(const float wind_direction)
  {
    _windDirection = wind_direction;
  }

  float getWindDirection(void) const
  {
    return _windDirection;
  }
  

  /*********************************************************************
   * Access the temperature value
   */

  void setTemperature(const float temperature)
  {
    _temperature = temperature;
  }

  float getTemperature(void) const
  {
    return _temperature;
  }
  

  /*********************************************************************
   * Access the relative humidity value
   */

  void setRelativeHumidity(const float rel_hum)
  {
    _relHum = rel_hum;
  }

  float getRelativeHumidity(void) const
  {
    return _relHum;
  }
  

  /*********************************************************************
   * Access the rainfall value
   */

  void setRainfall(const float rainfall)
  {
    _rainfall = rainfall;
  }

  float getRainfall(void) const
  {
    return _rainfall;
  }
  

  /*********************************************************************
   * Access the solar radiation value
   */

  void setSolarRadiation(const float solar_rad)
  {
    _solarRad = solar_rad;
  }

  float getSolarRadiation(void) const
  {
    return _solarRad;
  }
  

  /*********************************************************************
   * Access the pressure value
   */

  void setPressure(const float pressure)
  {
    _pressure = pressure;
  }

  float getPressure(void) const
  {
    return _pressure;
  }
  

  /*********************************************************************
   * Access the soil moisture 1 value
   */

  void setSoilMoisture1(const float soil_moist)
  {
    _soilMoist1 = soil_moist;
  }

  float getSoilMoisture1(void) const
  {
    return _soilMoist1;
  }
  

  /*********************************************************************
   * Access the soil moisture 2 value
   */

  void setSoilMoisture2(const float soil_moist)
  {
    _soilMoist2 = soil_moist;
  }

  float getSoilMoisture2(void) const
  {
    return _soilMoist2;
  }
  

  /*********************************************************************
   * Access the soil moisture 3 value
   */

  void setSoilMoisture3(const float soil_moist)
  {
    _soilMoist3 = soil_moist;
  }

  float getSoilMoisture3(void) const
  {
    return _soilMoist3;
  }
  

  /*********************************************************************
   * Access the soil moisture 4 value
   */

  void setSoilMoisture4(const float soil_moist)
  {
    _soilMoist4 = soil_moist;
  }

  float getSoilMoisture4(void) const
  {
    return _soilMoist4;
  }
  

  /*********************************************************************
   * Access the soil temperature value
   */

  void setSoilTemperature(const float soil_temp)
  {
    _soilTemp = soil_temp;
  }

  float getSoilTemperature(void) const
  {
    return _soilTemp;
  }
  

  /////////////////////////////
  // Database access methods //
  /////////////////////////////

  /*********************************************************************
   * disassemble() - Sets the object values based on a buffer retrieved from
   *                 an SPDB database.  This method handles all necessary
   *                 byte swapping.
   *
   * Returns true on success, false on failure.
   */

  bool disassemble(const void *buf, int len);


  /*********************************************************************
   * assemble() - Prepares a buffer to be written to an SPDB database based
   *              on the current object values.  This method handles all
   *              necessary byte swapping.
   *
   * On success, returns a pointer to the static buffer and stores the buffer
   * length in the buffer_len parameter.  On failure, returns 0.
   *
   * Note that the returned pointer points to static memory controlled by
   * this method.  This pointer should NOT be freed by the calling method.
   * Also, subsequent calls to this method will invalidate previous pointers
   * returned by this method.
   */

  const void *assemble(int& buffer_len);


  /*********************************************************************
   * print() - Print the object informationn to the given stream.
   */

  void print(FILE *out) const;
  void print(ostream &out) const;


protected:

  /////////////////////
  // Protected types //
  /////////////////////

#define HYDRO_STATION_NAME_LEN      40
#define NUM_HYDRO_STATION_SPARES    20

  typedef struct
  {
    char station_name[HYDRO_STATION_NAME_LEN];
    fl32 latitude;
    fl32 longitude;
    fl32 altitude;
    ti32 report_time;
    fl32 wind_speed;
    fl32 wind_direction;
    fl32 temperature;
    fl32 rel_hum;
    fl32 rainfall;
    fl32 solar_rad;
    fl32 pressure;
    fl32 soil_moist1;
    fl32 soil_moist2;
    fl32 soil_moist3;
    fl32 soil_moist4;
    fl32 soil_temp;
    fl32 spares[NUM_HYDRO_STATION_SPARES];
  } hydro_station_spdb_t;

    
  ///////////////////////
  // Protected members //
  ///////////////////////
  
  string _stationName;

  float _latitude;
  float _longitude;
  float _altitude;
  
  DateTime _reportTime;
  
  float _windSpeed;
  float _windDirection;
  float _temperature;   // deg C
  float _relHum;        // 0.0 to 1.0
  float _rainfall;
  float _solarRad;
  float _pressure;
  float _soilMoist1;
  float _soilMoist2;
  float _soilMoist3;
  float _soilMoist4;
  float _soilTemp;       // deg C
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  hydro_station_spdb_t _spdbAssembleBuffer;
  
};


#endif
