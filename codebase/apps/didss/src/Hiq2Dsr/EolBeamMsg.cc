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
/*********************************************************************
 * EolBeamMsg : Class representing a VIRAQ header read from a HiQ
 *               data stream.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dataport/smallend.h>
#include <toolsa/DateTime.hh>

#include "EolBeamMsg.hh"
using namespace std;


// Define globals

const int EolBeamMsg::HEADER_SIZE = 189;

EolBeamMsg::EolBeamMsg(const bool debug) :
  HiqMsg(debug),
  _abp(0),
  _abpSize(0)
{
}

EolBeamMsg::EolBeamMsg(const EolBeamMsg &rhs) :
  HiqMsg(rhs._debug)
{
  memcpy(_description, rhs._description, 4);
  _recordLen = rhs._recordLen;
  _gates = rhs._gates;
  _hits = rhs._hits;
  _rcvrPulseWidth = rhs._rcvrPulseWidth;
  _prt = rhs._prt;
  _delay = rhs._delay;
  _clutterFilter = rhs._clutterFilter;
  _timeSeries = rhs._timeSeries;
  _tsGate = rhs._tsGate;
  _time = rhs._time;
  _subSeconds = rhs._subSeconds;
  _azimuth = rhs._azimuth;
  _elevation = rhs._elevation;
  _radarLongitude = rhs._radarLongitude;
  _radarLatitude = rhs._radarLatitude;
  _radarAltitude = rhs._radarAltitude;
  _ewVelocity = rhs._ewVelocity;
  _nsVelocity = rhs._nsVelocity;
  _vertVelocity = rhs._vertVelocity;
  _dataFormat = rhs._dataFormat;
  _prt2 = rhs._prt2;
  _fixedAngle = rhs._fixedAngle;
  _scanType = rhs._scanType;
  _scanNum = rhs._scanNum;
  _volumeNum = rhs._volumeNum;
  _rayCount = rhs._rayCount;
  _transition = rhs._transition;
  _horizXmitPower = rhs._horizXmitPower;
  _vertXmitPower = rhs._vertXmitPower;
  memcpy(_spare, rhs._spare, 100);
  _abpSize = rhs._abpSize;
  _abp = new si16[_abpSize];
  memcpy(_abp, rhs._abp, _abpSize * sizeof(si16));
}

EolBeamMsg::~EolBeamMsg() 
{
  delete [] _abp;
}


bool EolBeamMsg::init(const char *buffer)
{
  char *buffer_ptr = (char *)buffer;

  // Copy each data field from the message.  Note that each field
  // must be individually copied using the memcpy() routine because
  // the data fields do not respect word boundaries.  The data in the
  // message is in small-endian format so each field is byte-swapped
  // as it is read, using the appropriate byte-swapping routine (byte
  // fields do not need swapping).

  memcpy(_description, buffer_ptr, sizeof(_description));
  buffer_ptr += sizeof(_description);

  memcpy(&_recordLen, buffer_ptr, sizeof(_recordLen));
  _recordLen = SE_to_si16(_recordLen);
  buffer_ptr += sizeof(_recordLen);

  memcpy(&_gates, buffer_ptr, sizeof(_gates));
  _gates = SE_to_si16(_gates);
  buffer_ptr += sizeof(_gates);

  memcpy(&_hits, buffer_ptr, sizeof(_hits));
  _hits = SE_to_si16(_hits);
  buffer_ptr += sizeof(_hits);

  memcpy(&_rcvrPulseWidth, buffer_ptr, sizeof(_rcvrPulseWidth));
  SE_swap_array_32((void *)&_rcvrPulseWidth, sizeof(_rcvrPulseWidth));
  buffer_ptr += sizeof(_rcvrPulseWidth);

  memcpy(&_prt, buffer_ptr, sizeof(_prt));
  SE_swap_array_32((void *)&_prt, sizeof(_prt));
  buffer_ptr += sizeof(_prt);

  memcpy(&_delay, buffer_ptr, sizeof(_delay));
  SE_swap_array_32((void *)&_delay,sizeof(_delay));
  buffer_ptr += sizeof(_delay);

  memcpy(&_clutterFilter, buffer_ptr, sizeof(_clutterFilter));
  buffer_ptr += sizeof(_clutterFilter);

  memcpy(&_timeSeries, buffer_ptr, sizeof(_timeSeries));
  buffer_ptr += sizeof(_timeSeries);

  memcpy(&_tsGate, buffer_ptr, sizeof(_tsGate));
  _tsGate = SE_to_si16(_tsGate);
  buffer_ptr += sizeof(_tsGate);

  memcpy(&_time, buffer_ptr, sizeof(_time));
  _time = SE_to_si32(_time);
  buffer_ptr += sizeof(_time);

  memcpy(&_subSeconds, buffer_ptr, sizeof(_subSeconds));
  _subSeconds = SE_to_si16(_subSeconds);
  buffer_ptr += sizeof(_subSeconds);

  memcpy(&_azimuth, buffer_ptr, sizeof(_azimuth));
  SE_swap_array_32((void *)&_azimuth, sizeof(_azimuth));
  buffer_ptr += sizeof(_azimuth);

  memcpy(&_elevation, buffer_ptr, sizeof(_elevation));
  SE_swap_array_32((void *)&_elevation, sizeof(_elevation));
  buffer_ptr += sizeof(_elevation);

  memcpy(&_radarLongitude, buffer_ptr, sizeof(_radarLongitude));
  SE_swap_array_32((void *)&_radarLongitude, sizeof(_radarLongitude));
  buffer_ptr += sizeof(_radarLongitude);

  memcpy(&_radarLatitude, buffer_ptr, sizeof(_radarLatitude));
  SE_swap_array_32((void *)&_radarLatitude, sizeof(_radarLatitude));
  buffer_ptr += sizeof(_radarLatitude);

  memcpy(&_radarAltitude, buffer_ptr, sizeof(_radarAltitude));
  SE_swap_array_32((void *)&_radarAltitude, sizeof(_radarAltitude));
  buffer_ptr += sizeof(_radarAltitude);

  memcpy(&_ewVelocity, buffer_ptr, sizeof(_ewVelocity));
  SE_swap_array_32((void *)&_ewVelocity, sizeof(_ewVelocity));
  buffer_ptr += sizeof(_ewVelocity);

  memcpy(&_nsVelocity, buffer_ptr, sizeof(_nsVelocity));
  SE_swap_array_32((void *)&_nsVelocity, sizeof(_nsVelocity));
  buffer_ptr += sizeof(_nsVelocity);

  memcpy(&_vertVelocity, buffer_ptr, sizeof(_vertVelocity));
  SE_swap_array_32((void *)&_vertVelocity, sizeof(_vertVelocity));
  buffer_ptr += sizeof(_vertVelocity);

  memcpy(&_dataFormat, buffer_ptr, sizeof(_dataFormat));
  buffer_ptr += sizeof(_dataFormat);

  memcpy(&_prt2, buffer_ptr, sizeof(_prt2));
  SE_swap_array_32((void *)&_prt2, sizeof(_prt2));
  buffer_ptr += sizeof(_prt2);

  memcpy(&_fixedAngle, buffer_ptr, sizeof(_fixedAngle));
  SE_swap_array_32((void *)&_fixedAngle, sizeof(_fixedAngle));
  buffer_ptr += sizeof(_fixedAngle);

  memcpy(&_scanType, buffer_ptr, sizeof(_scanType));
  buffer_ptr += sizeof(_scanType);

  memcpy(&_scanNum, buffer_ptr, sizeof(_scanNum));
  buffer_ptr += sizeof(_scanNum);

  memcpy(&_volumeNum, buffer_ptr, sizeof(_volumeNum));
  buffer_ptr += sizeof(_volumeNum);

  memcpy(&_rayCount, buffer_ptr, sizeof(_rayCount));
  _rayCount = SE_to_si32(_rayCount);
  buffer_ptr += sizeof(_rayCount);

  memcpy(&_transition, buffer_ptr, sizeof(_transition));
  buffer_ptr += sizeof(_transition);

  memcpy(&_horizXmitPower, buffer_ptr, sizeof(_horizXmitPower));
  SE_swap_array_32((void *)&_horizXmitPower, sizeof(_horizXmitPower));
  buffer_ptr += sizeof(_horizXmitPower);

  memcpy(&_vertXmitPower, buffer_ptr, sizeof(_vertXmitPower));
  SE_swap_array_32((void *)&_vertXmitPower, sizeof(_vertXmitPower));
  buffer_ptr += sizeof(_vertXmitPower);

  memcpy(_spare, buffer_ptr, sizeof(_spare));
  buffer_ptr += sizeof(_spare);

  if (buffer_ptr - buffer != HEADER_SIZE)
  {
    cerr << "*******ERROR******" << endl;
    cerr << "     HEADER_SIZE = " << HEADER_SIZE << endl;
    cerr << "     buffer_ptr - buffer = " << (buffer_ptr - buffer) << endl;
  }

  int data_bytes = (_recordLen - HEADER_SIZE);
  int array_size = data_bytes / sizeof(si16);

  if (_debug)
    cerr << "   Allocating " << data_bytes << " bytes for data pointer" << endl;

  delete [] _abp;
  _abp = new si16[array_size];
  memcpy(_abp, buffer_ptr, data_bytes);
  _abpSize = array_size;
  
  return true;
}


void EolBeamMsg::print(ostream &stream) const
{
  stream << "VIRAQ Header" << endl;
  stream << "------------" << endl;
  stream << "desc = <";
  for (int i = 0; i < 4; ++i)
  {
    if (isprint(_description[i]))
      stream << _description[i];
    else
      stream << "*";
  }
  stream << ">" << endl;
  stream << "record_len = " << _recordLen << endl;
  stream << "gates = " << _gates << endl;
  stream << "hits = " << _hits << endl;
  stream << "rcvr_pulsewidth = " << _rcvrPulseWidth << endl;
  stream << "prt = " << _prt << endl;
  stream << "delay = " << _delay << endl;
  stream << "clutterfilter = " << (int)_clutterFilter << endl;
  stream << "timeseries = " << (int)_timeSeries << endl;
  stream << "tsgate = " << _tsGate << endl;
  stream << "time = " << DateTime::str(_time) << endl;
  stream << "subsec = " << _subSeconds << endl;
  stream << "az = " << _azimuth << endl;
  stream << "el = " << _elevation << endl;
  stream << "radar_longitude = " << _radarLongitude << endl;
  stream << "radar_latitude = " << _radarLatitude << endl;
  stream << "radar_altitude = " << _radarAltitude << endl;
  stream << "ew_velocity = " << _ewVelocity << endl;
  stream << "ns_velocity = " << _nsVelocity << endl;
  stream << "vert_velocity = " << _vertVelocity << endl;
  stream << "dataformat = " << (int)_dataFormat << endl;
  stream << "prt2 = " << _prt2 << endl;
  stream << "fxd_angle = " << _fixedAngle << endl;
  stream << "scan_type = " << (int)_scanType << endl;
  stream << "scan_num = " << (int)_scanNum << endl;
  stream << "vol_num = " << (int)_volumeNum << endl;
  stream << "ray_count = " << _rayCount << endl;
  stream << "transition = " << (int)_transition << endl;
  stream << "hxmit_power = " << _horizXmitPower << endl;
  stream << "vxmit_power = " << _vertXmitPower << endl;
//  stream << endl;
}


void EolBeamMsg::printSummary(ostream &stream) const
{
  stream << "beam: elev = " << _elevation << ", az = " << _azimuth
	 << ", time = " << DateTime::str(_time) << endl;
}
