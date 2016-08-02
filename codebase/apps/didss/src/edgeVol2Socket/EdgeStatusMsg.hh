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
 *   $Date: 2016/03/06 23:53:42 $
 *   $Id: EdgeStatusMsg.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * EdgeStatusMsg : An EDGE status message.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef EdgeStatusMsg_HH
#define EdgeStatusMsg_HH


#include <string>

#include "EdgeMsg.hh"
using namespace std;


class EdgeStatusMsg : public EdgeMsg
{

public:

  ////////////////////
  // Public Methods //
  ////////////////////

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructors
   */

  EdgeStatusMsg();


  /*********************************************************************
   * Destructor
   */

  virtual ~EdgeStatusMsg();


  //////////////////////////////////
  // Instantiated virtual methods //
  //////////////////////////////////

  /*********************************************************************
   * getMsgSize() - Returns the number of bytes needed to store the whole
   *                message for transmission.
   */

  virtual int getMsgSize()
  {
    return _msgHeader.getHeaderSize() + STATUS_MSG_SIZE;
  }
  

  /*********************************************************************
   * writeMsgToBuffer() - Write the message data to the given buffer.
   *                      The buffer must be allocated by the calling
   *                      method.  The needed buffer size can be found
   *                      using getMsgSize();
   *
   * Returns true if the message was successfully written to the buffer,
   * false otherwise.
   */

  virtual bool writeMsgToBuffer(void *msg_buffer);


  ////////////////////
  // Access Methods //
  ////////////////////

  /*********************************************************************
   * Methods for setting the different values in the status message.
   */

  void setPrf1(const unsigned int prf1)
  {
    _prf1 = prf1;
  }
  
  void setPrf2(const unsigned int prf2)
  {
    _prf2 = prf2;
  }
  
  void setRange(const unsigned int range)
  {
    _range = range;
  }
  
  void setUiSamples(const unsigned int ui_samples)
  {
    _uiSamples = ui_samples;
  }
  
  void setTimeSeriesRange(const float time_series_range)
  {
    _timeSeriesRange = time_series_range;
  }
  
  void setProcessingMode(const int processing_mode)
  {
    _processingMode = processing_mode;
  }
  
  void setGw1(const unsigned int gw1_meters)
  {
    _gw1 = gw1_meters;
  }
  
  void setGw2(const unsigned int gw2_meters)
  {
    _gw2 = gw2_meters;
  }
  
  void setGwPartition(const unsigned int gw_partition_meters)
  {
    _gwPartition = gw_partition_meters;
  }
  
  void setRangeAvg(const unsigned int range_avg)
  {
    _rangeAvg = range_avg;
  }
  
  void setGates(const unsigned int gates)
  {
    _gates = gates;
  }
  
  void setMomentEnable(const unsigned int moment_enable)
  {
    _momentEnable = moment_enable;
  }
  
  void setSoftwareSim(const unsigned int software_sim)
  {
    _softwareSim = software_sim;
  }
  
  void setUiScanType(const unsigned int ui_scan_type)
  {
    _uiScanType = ui_scan_type;
  }
  
  void setTargetAzimuth(const double target_azimuth)
  {
    _targetAzimuth = target_azimuth;
  }
  
  void setTargetElevation(const double target_elevation)
  {
    _targetElevation = target_elevation;
  }
  
  void setSpeed(const unsigned int speed)
  {
    _speed = speed;
  }
  
  void setAntennaSpeed(const unsigned int antenna_speed)
  {
    _antennaSpeed = antenna_speed;
  }
  
  void setElevationSpeed(const unsigned int elevation_speed)
  {
    _elevationSpeed = elevation_speed;
  }
  
  void setStartAngle(const unsigned int start_angle)
  {
    _startAngle = start_angle;
  }
  
  void setStopAngle(const unsigned int stop_angle)
  {
    _stopAngle = stop_angle;
  }
  
  void setDTime(const unsigned int d_time)
  {
    _dTime = d_time;
  }
  
  void setSiteName(const string &site_name)
  {
    _siteName = site_name;
  }
  
  void setRadarType(const string &radar_type)
  {
    _radarType = radar_type;
  }
  
  void setJobName(const string &job_name)
  {
    _jobName = job_name;
  }
  
  void setLon(const int lon_deg,
	      const int lon_min,
	      const int lon_sec)
  {
    _lonDeg = lon_deg;
    _lonMin = lon_min;
    _lonSec = lon_sec;
  }
  
  void setLat(const int lat_deg,
	      const int lat_min,
	      const int lat_sec)
  {
    _latDeg = lat_deg;
    _latMin = lat_min;
    _latSec = lat_sec;
  }
  
  void setAzimuth(const double azimuth)
  {
    _azimuth = azimuth;
    _msgHeader.setAzimuth(azimuth);
  }
  
  void setElevation(const double elevation)
  {
    _elevation = elevation;
    _msgHeader.setElevation(elevation);
  }
  
  void setScdFlag(const unsigned int scd_flag)
  {
    _scdFlag = scd_flag;
  }
  
  void setSigprocFlag(const unsigned int sigproc_flag)
  {
    _sigprocFlag = sigproc_flag;
  }
  
  void setInterfaceType(const unsigned int interface_type)
  {
    _interfaceType = interface_type;
  }
  
  void setRadarPower(const unsigned int radar_power)
  {
    _radarPower = radar_power;
  }
  
  void setServo(const unsigned int servo)
  {
    _servo = servo;
  }
  
  void setRadiate(const unsigned int radiate)
  {
    _radiate = radiate;
  }
  
  void setFlags(const unsigned int flags)
  {
    _flags = flags;
  }
  
  void setTcfZ(const unsigned int tcf_z)
  {
    _tcfZ = tcf_z;
  }
  
  void setTcfU(const unsigned int tcf_u)
  {
    _tcfU = tcf_u;
  }
  
  void setTcfV(const unsigned int tcf_v)
  {
    _tcfV = tcf_v;
  }
  
  void setTcfW(const unsigned int tcf_w)
  {
    _tcfW = tcf_w;
  }
  
  void setClutterFilter(const unsigned int clutter_filter)
  {
    _clutterFilter = clutter_filter;
  }
  
  void setSqi(const unsigned int sqi)
  {
    _sqi = sqi;
  }
  
  void setPw(const unsigned int pw)
  {
    _pw = pw;
  }
  
  void setFold(const unsigned int fold)
  {
    _fold = fold;
  }
  
  void setRadarWavelength(const float radar_wavelength)
  {
    _radarWavelength = radar_wavelength;
  }
  


private:

  static const int STATUS_MSG_SIZE;
  
  // Information in the message

  // Record 1

  int _versionNum;
  unsigned int _prf1;                // PRF in Hz
  unsigned int _prf2;                // PRF in Hz
  unsigned int _range;
  unsigned int _uiSamples;
  float _timeSeriesRange;
  int _processingMode;
  
  // Record 2

  unsigned int _gw1;             // meters
  unsigned int _gw2;             // meters
  unsigned int _gwPartition;     // meters
  unsigned int _rangeAvg;
  unsigned int _gates;
  
  // Record 3

  unsigned int _momentEnable;
  unsigned int _softwareSim;   // 0 = off, 1 = on
  
  // Record 4

  unsigned int _uiScanType;
  double _targetAzimuth;
  double _targetElevation;
  unsigned int _speed;
  unsigned int _antennaSpeed;
  unsigned int _elevationSpeed;
  unsigned int _startAngle;
  unsigned int _stopAngle;
  
  // Record 5

  unsigned int _dTime;
  string _siteName;
  string _radarType;
  string _jobName;
  
  // Record 6

  int _lonDeg;
  int _lonMin;
  int _lonSec;
  int _latDeg;
  int _latMin;
  int _latSec;
  int _antennaHeight;     // height above sea level in meters
  
  // Record 7

  double _azimuth;
  double _elevation;
  unsigned int _scdFlag;
  
  // Record 8

  unsigned int _sigprocFlag;
  unsigned int _interfaceType;
  unsigned int _radarPower;
  unsigned int _servo;
  unsigned int _radiate;
  
  // Record 9

  unsigned int _flags;
  unsigned int _tcfZ;
  unsigned int _tcfU;
  unsigned int _tcfV;
  unsigned int _tcfW;
  unsigned int _clutterFilter;
  unsigned int _sqi;
  unsigned int _pw;
  unsigned int _fold;
  float _radarWavelength;
  
  // Currently skip records 10-14 since we don't use them

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _addRecord1ToBuffer() - Write the record 1 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord1ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord2ToBuffer() - Write the record 2 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord2ToBuffer(char *msg_buffer);

  
  /*********************************************************************
   * _addRecord3ToBuffer() - Write the record 3 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord3ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord4ToBuffer() - Write the record 4 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord4ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord5ToBuffer() - Write the record 5 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord5ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord6ToBuffer() - Write the record 6 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord6ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord7ToBuffer() - Write the record 7 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord7ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord8ToBuffer() - Write the record 8 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord8ToBuffer(char *msg_buffer);
  

  /*********************************************************************
   * _addRecord9ToBuffer() - Write the record 9 information to the status
   *                         buffer.
   *
   * Returns the number of bytes written to the buffer.
   */

  int _addRecord9ToBuffer(char *msg_buffer);
  
};

#endif
