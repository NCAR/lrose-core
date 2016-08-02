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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:45:40 $
//   $Id: HydroStation.cc,v 1.5 2016/03/03 18:45:40 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * HydroStation : Class for manipulating Hydrology Station data in an
 *                SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <dataport/bigend.h>
#include <rapformats/HydroStation.hh>
#include <toolsa/str.h>
using namespace std;


// Global constants

const float HydroStation::DATA_NOT_AVAILABLE = -999.0;


/*********************************************************************
 * Constructors
 */

HydroStation::HydroStation()
{
  // Clear all of the information in the station report

  clear();

  // Set all of the spare values in the SPDB assembly buffer to indicate
  // they are not available and swap the value so we don't have to do this
  // every time we assemble a buffer.

  for (int i = 0; i < NUM_HYDRO_STATION_SPARES; ++i)
    _spdbAssembleBuffer.spares[i] = DATA_NOT_AVAILABLE;

  BE_from_array_32(_spdbAssembleBuffer.spares,
		   NUM_HYDRO_STATION_SPARES * sizeof(fl32));
}


/*********************************************************************
 * Destructor
 */

HydroStation::~HydroStation()
{
  // Do nothing
}


/*********************************************************************
 * disassemble() - Sets the object values based on a buffer retrieved from
 *                 an SPDB database.  This method handles all necessary
 *                 byte swapping.
 *
 * Returns true on success, false on failure.
 */

bool HydroStation::disassemble(const void *buf, int len)
{
  static const string method_name = "HydroStation::disassemble()";
  
  // Make sure the incoming buffer is of the correct length

  if (len != (int)sizeof(hydro_station_spdb_t))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Incoming buffer contains " << len << " bytes" << endl;
    cerr << "Buffer should contain " << sizeof(hydro_station_spdb_t) << " bytes" << endl;
    
    return false;
  }
  
  // Clear out the current station data

  clear();
  
  // Copy the station information into a local buffer and do the
  // necessary byte swapping. Note that we can skip swapping the spare
  // values because these are not currently used within the object.

  hydro_station_spdb_t spdb_data;
  
  memcpy(&spdb_data, buf, len);
  
  BE_to_fl32(&spdb_data.latitude, &spdb_data.latitude);
  BE_to_fl32(&spdb_data.longitude, &spdb_data.longitude);
  BE_to_fl32(&spdb_data.altitude, &spdb_data.altitude);
  spdb_data.report_time = BE_to_ti32(spdb_data.report_time);
  BE_to_fl32(&spdb_data.wind_speed, &spdb_data.wind_speed);
  BE_to_fl32(&spdb_data.wind_direction, &spdb_data.wind_direction);
  BE_to_fl32(&spdb_data.temperature, &spdb_data.temperature);
  BE_to_fl32(&spdb_data.rel_hum, &spdb_data.rel_hum);
  BE_to_fl32(&spdb_data.rainfall, &spdb_data.rainfall);
  BE_to_fl32(&spdb_data.solar_rad, &spdb_data.solar_rad);
  BE_to_fl32(&spdb_data.pressure, &spdb_data.pressure);
  BE_to_fl32(&spdb_data.soil_moist1, &spdb_data.soil_moist1);
  BE_to_fl32(&spdb_data.soil_moist2, &spdb_data.soil_moist2);
  BE_to_fl32(&spdb_data.soil_moist3, &spdb_data.soil_moist3);
  BE_to_fl32(&spdb_data.soil_moist4, &spdb_data.soil_moist4);
  BE_to_fl32(&spdb_data.soil_temp, &spdb_data.soil_temp);
  
  // Set the object values from the local buffer values

  _stationName = spdb_data.station_name;
  _latitude = spdb_data.latitude;
  _longitude = spdb_data.longitude;
  _altitude = spdb_data.altitude;
  
  setData(spdb_data.report_time,
	  spdb_data.wind_speed,
	  spdb_data.wind_direction,
	  spdb_data.temperature,
	  spdb_data.rel_hum,
	  spdb_data.rainfall,
	  spdb_data.solar_rad,
	  spdb_data.pressure,
	  spdb_data.soil_moist1,
	  spdb_data.soil_moist2,
	  spdb_data.soil_moist3,
	  spdb_data.soil_moist4,
	  spdb_data.soil_temp);

  return true;
}


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

const void *HydroStation::assemble(int& buffer_len)
{
  // Load the assembly buffer

  STRcopy(_spdbAssembleBuffer.station_name, _stationName.c_str(),
	  HYDRO_STATION_NAME_LEN);
  _spdbAssembleBuffer.latitude = _latitude;
  _spdbAssembleBuffer.longitude = _longitude;
  _spdbAssembleBuffer.altitude = _altitude;
  _spdbAssembleBuffer.report_time = _reportTime.utime();
  _spdbAssembleBuffer.wind_speed = _windSpeed;
  _spdbAssembleBuffer.wind_direction = _windDirection;
  _spdbAssembleBuffer.temperature = _temperature;
  _spdbAssembleBuffer.rel_hum = _relHum;
  _spdbAssembleBuffer.rainfall = _rainfall;
  _spdbAssembleBuffer.solar_rad = _solarRad;
  _spdbAssembleBuffer.pressure = _pressure;
  _spdbAssembleBuffer.soil_moist1 = _soilMoist1;
  _spdbAssembleBuffer.soil_moist2 = _soilMoist2;
  _spdbAssembleBuffer.soil_moist3 = _soilMoist3;
  _spdbAssembleBuffer.soil_moist4 = _soilMoist4;
  _spdbAssembleBuffer.soil_temp = _soilTemp;
  
  // Byte swap the values in the buffer

  BE_from_fl32(&_spdbAssembleBuffer.latitude, &_spdbAssembleBuffer.latitude);
  BE_from_fl32(&_spdbAssembleBuffer.longitude, &_spdbAssembleBuffer.longitude);
  BE_from_fl32(&_spdbAssembleBuffer.altitude, &_spdbAssembleBuffer.altitude);
  _spdbAssembleBuffer.report_time =
    BE_from_ti32(_spdbAssembleBuffer.report_time);
  BE_from_fl32(&_spdbAssembleBuffer.wind_speed, &_spdbAssembleBuffer.wind_speed);
  BE_from_fl32(&_spdbAssembleBuffer.wind_direction, &_spdbAssembleBuffer.wind_direction);
  BE_from_fl32(&_spdbAssembleBuffer.temperature, &_spdbAssembleBuffer.temperature);
  BE_from_fl32(&_spdbAssembleBuffer.rel_hum, &_spdbAssembleBuffer.rel_hum);
  BE_from_fl32(&_spdbAssembleBuffer.rainfall, &_spdbAssembleBuffer.rainfall);
  BE_from_fl32(&_spdbAssembleBuffer.solar_rad, &_spdbAssembleBuffer.solar_rad);
  BE_from_fl32(&_spdbAssembleBuffer.pressure, &_spdbAssembleBuffer.pressure);
  BE_from_fl32(&_spdbAssembleBuffer.soil_moist1, &_spdbAssembleBuffer.soil_moist1);
  BE_from_fl32(&_spdbAssembleBuffer.soil_moist2, &_spdbAssembleBuffer.soil_moist2);
  BE_from_fl32(&_spdbAssembleBuffer.soil_moist3, &_spdbAssembleBuffer.soil_moist3);
  BE_from_fl32(&_spdbAssembleBuffer.soil_moist4, &_spdbAssembleBuffer.soil_moist4);
  BE_from_fl32(&_spdbAssembleBuffer.soil_temp, &_spdbAssembleBuffer.soil_temp);

  // Return the assembled buffer

  buffer_len = sizeof(_spdbAssembleBuffer);
  return (void *)(&_spdbAssembleBuffer);
}


/*********************************************************************
 * print() - Print the object informationn to the given stream.
 */

void HydroStation::print(FILE *out) const
{
  fprintf(out, "  =================\n");
  fprintf(out, "  HydroStation data\n");
  fprintf(out, "  =================\n");
  fprintf(out, "  station name: %s\n", _stationName.c_str());
  fprintf(out, "  latitude: %f\n", _latitude);
  fprintf(out, "  longitude: %f\n", _longitude);
  fprintf(out, "  altitude: %f\n", _altitude);
  fprintf(out, "\n");
  fprintf(out, "  report time: %s\n", _reportTime.ctime());

  fprintf(out, "  wind speed: ");
  if (_windSpeed == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _windSpeed);
  
  fprintf(out, "  wind direction: ");
  if (_windDirection == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _windDirection);
  
  fprintf(out, "  temperature: ");
  if (_temperature == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f C\n", _temperature);
  
  fprintf(out, "  relative humidity: ");
  if (_relHum == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f %%\n", _relHum * 100.0);
  
  fprintf(out, "  rainfall: ");
  if (_rainfall == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _rainfall);
  
  fprintf(out, "  solar radiation: ");
  if (_solarRad == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _solarRad);
  
  fprintf(out, "  pressure: ");
  if (_pressure == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _pressure);
  
  fprintf(out, "  soil moisture 1: ");
  if (_soilMoist1 == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _soilMoist1);
  
  fprintf(out, "  soil moisture 2: ");
  if (_soilMoist2 == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _soilMoist2);
  
  fprintf(out, "  soil moisture 3: ");
  if (_soilMoist3 == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _soilMoist3);
  
  fprintf(out, "  soil moisture 4: ");
  if (_soilMoist4 == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f\n", _soilMoist4);
  
  fprintf(out, "  soil temperature: ");
  if (_soilTemp == DATA_NOT_AVAILABLE)
    fprintf(out, "NOT AVAILABLE\n");
  else
    fprintf(out, "%f C\n", _soilTemp);
  
  
}

void HydroStation::print(ostream &out) const
{
  out << "  =================" << endl;
  out << "  HydroStation data" << endl;
  out << "  =================" << endl;
  out << "  station name: " << _stationName << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  longitude: " << _longitude << endl;
  out << "  altitude: " << _altitude << endl;
  out << endl;
  out << "  report time: " << _reportTime << endl;

  out << "  wind speed: ";
  if (_windSpeed == DATA_NOT_AVAILABLE)
    cerr << "NOT AVAILABLE" << endl;
  else
    cerr << _windSpeed << endl;

  out << "  wind direction: ";
  if (_windDirection == DATA_NOT_AVAILABLE)
    cerr << "NOT AVAILABLE" << endl;
  else
    cerr << _windDirection << endl;

  out << "  temperature: ";
  if (_temperature == DATA_NOT_AVAILABLE)
    cerr << "NOT AVAILABLE" << endl;
  else
    cerr << _temperature << " C" << endl;

  out << "  relative humidity: ";
  if (_relHum == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << (_relHum * 100.0) << " %" << endl;

  out << "  rainfall: ";
  if (_rainfall == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _rainfall << endl;

  out << "  solar radiation: ";
  if (_solarRad == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _solarRad << endl;

  out << "  pressure: ";
  if (_pressure == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _pressure << endl;

  out << "  soil moisture 1: ";
  if (_soilMoist1 == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _soilMoist1 << endl;

  out << "  soil moisture 2: ";
  if (_soilMoist2 == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _soilMoist2 << endl;

  out << "  soil moisture 3: ";
  if (_soilMoist3 == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _soilMoist3 << endl;

  out << "  soil moisture 4: ";
  if (_soilMoist4 == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _soilMoist4 << endl;

  out << "  soil temperature: ";
  if (_soilTemp == DATA_NOT_AVAILABLE)
    out << "NOT AVAILABLE" << endl;
  else
    out << _soilTemp << " C" << endl;
}
