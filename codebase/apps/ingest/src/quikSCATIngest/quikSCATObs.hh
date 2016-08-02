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

/************************************************************************
 * quikSCATObs: Class that manipulates a quikSCAT observation.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef quikSCATObs_H
#define quikSCATObs_H

#include <toolsa/DateTime.hh>

using namespace std;


class quikSCATObs
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructors
   */

  quikSCATObs(DateTime time = DateTime::NEVER, double lat = 0.0, double lon = 0.0, 
	      double wind_speed = 0.0, double wind_dir = 0.0, bool rain = false, 
	      bool nsol = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~quikSCATObs();


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
   * setLat() - Set the latitude value.
   */

  void setLat(double lat)
  {
    _latitude = lat;
  }


  /*********************************************************************
   * setLon() - Set the longitude value.
   */

  void setLon(double lon)
  {
    _longitude = lon;
  }
  

  /*********************************************************************
   * setObsTime() - Set the observation time.
   */

  void setObsTime(DateTime obsTime)
  {
    _obsTime = obsTime;
  }
  

  /*********************************************************************
   * setRainFlag() - Set the rain flag value.
   */

  void setRainFlag(bool rain_flag)
  {
    _rainFlag = rain_flag;
  }

  
  /*********************************************************************
   * setNsolFlag() - Set the nsol flag value.
   */

  void setNsolFlag(bool nsol_flag)
  {
    _nsolFlag = nsol_flag;
  }
  

  /*********************************************************************
   * setWindSpeed() - Set the wind speed value.
   */

  void setWindSpeed(double wind_speed)
  {
    _windSpeed = wind_speed;
  }


  /*********************************************************************
   * setWindDir() - Set the wind direction value.
   */

  void setWindDir(double wind_dir)
  {
    _windDir = wind_dir;
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
   * getRainFlag() - Retrieve the rain flag.
   */

  bool getRainFlag() const
  {
    return _rainFlag;
  }
  

  /*********************************************************************
   * getNsolFlag() - Retrieve the nsol flag.
   */

  bool getNsolFlag() const
  {
    return _nsolFlag;
  }
  

  /*********************************************************************
   * getWindSpeed() - Retrieve the wind speed in meters per second
   */

  double getWindSpeed() const
  {
    return _windSpeed;
  }
  

  /*********************************************************************
   * getWindDir() - Retrieve the wind direction in degrees north.
   */

  double getWindDir() const
  {
    return _windDir;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  DateTime _obsTime;
  double _latitude;
  double _longitude;
  double _windSpeed;         // meters per second
  double _windDir;           // degN
  bool _rainFlag;
  bool _nsolFlag;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////


};

#endif
