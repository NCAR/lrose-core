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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: EdgeStatusMsg.cc,v 1.5 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeStatusMsg : An EDGE status message.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cerrno>
#include <cstring>

#include "EdgeMsgSupport.hh"
#include "EdgeStatusMsg.hh"
using namespace std;


// Define constants

const int EdgeStatusMsg::STATUS_MSG_SIZE = 500;


/*********************************************************************
 * Constructor
 */

EdgeStatusMsg::EdgeStatusMsg() :
  EdgeMsg(),
  _versionNum(1),
  _prf1(0),
  _prf2(0),
  _range(0),
  _uiSamples(0),
  _timeSeriesRange(0.0),
  _processingMode(0),
  _gw1(0),
  _gw2(0),
  _gwPartition(0),
  _rangeAvg(0),
  _gates(0),
  _momentEnable(0),
  _softwareSim(0),
  _uiScanType(0),
  _targetAzimuth(0),
  _targetElevation(0),
  _speed(0),
  _antennaSpeed(0),
  _elevationSpeed(0),
  _startAngle(0),
  _stopAngle(0),
  _dTime(0),
  _siteName(""),
  _radarType(""),
  _jobName(""),
  _lonDeg(0),
  _lonMin(0),
  _lonSec(0),
  _latDeg(0),
  _latMin(0),
  _latSec(0),
  _azimuth(0.0),
  _elevation(0.0),
  _scdFlag(0),
  _sigprocFlag(0),
  _interfaceType(0),
  _radarPower(0),
  _servo(0),
  _radiate(0),
  _flags(0),
  _tcfZ(0),
  _tcfU(0),
  _tcfV(0),
  _tcfW(0),
  _clutterFilter(0),
  _sqi(0),
  _pw(0),
  _fold(0),
  _radarWavelength(0.0)
{
  // Set the constant values in the header

  _msgHeader.setMomentType(EdgeMsgSupport::EDGE_STATUS_MOMENT);
}


/*********************************************************************
 * Destructor
 */

EdgeStatusMsg::~EdgeStatusMsg() 
{
  // Do nothing
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

bool EdgeStatusMsg::writeMsgToBuffer(void *msg_buffer)
{
  // Initialize the buffer

  memset(msg_buffer, 0, _msgHeader.getHeaderSize() + STATUS_MSG_SIZE);

  // Get a pointer into the buffer for writing

  char *buffer_ptr = (char *)msg_buffer;
  
  // First write the message header to the buffer, updating the
  // message lengths appropriately.

  _msgHeader.setUncompressedLen(STATUS_MSG_SIZE);
  _msgHeader.setCompressedLen(STATUS_MSG_SIZE);
  
  _msgHeader.addHeaderToBuffer(msg_buffer);
  
  // Now add the message information to the buffer.  We will write
  // each record separately updating the buffer pointer as we go.

  buffer_ptr += _msgHeader.getHeaderSize();
  
  int record_size;
  
  record_size = _addRecord1ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord2ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord3ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord4ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord5ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord6ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord7ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord8ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  record_size = _addRecord9ToBuffer(buffer_ptr);
  buffer_ptr += record_size;
  
  return true;
}



/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addRecord1ToBuffer() - Write the record 1 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord1ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "V%04d %4d %4d %8d %4d %f %d",
			      _versionNum,
			      _prf1, _prf2, _range, _uiSamples,
			      _timeSeriesRange, _processingMode);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord2ToBuffer() - Write the record 2 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord2ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%f %f %8d %3d %4d",
			      (float)_gw1,
			      (float)_gw2,
			      _gwPartition, _rangeAvg, _gates);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord3ToBuffer() - Write the record 3 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord3ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%02x %1d",
			      _momentEnable, _softwareSim);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord4ToBuffer() - Write the record 4 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord4ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%1d %04x %04x %04x %04x %04x %04x %04x",
			      _uiScanType,
			      EdgeMsgSupport::degToBinary(_targetAzimuth),
			      EdgeMsgSupport::degToBinary(_targetElevation),
			      _speed, _antennaSpeed, _elevationSpeed,
			      _startAngle, _stopAngle);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord5ToBuffer() - Write the record 5 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord5ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%08x \"%s\" \"%s\" \"%s\"",
			      _dTime, _siteName.c_str(),
			      _radarType.c_str(), _jobName.c_str());
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord6ToBuffer() - Write the record 6 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord6ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%4d %3d %3d %4d %3d %3d %5d",
			      _lonDeg, _lonMin, _lonSec,
			      _latDeg, _latMin, _latSec,
			      _antennaHeight);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord7ToBuffer() - Write the record 7 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord7ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%04x %04x %04x",
			      EdgeMsgSupport::degToBinary(_azimuth),
			      EdgeMsgSupport::degToBinary(_elevation),
			      _scdFlag);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord8ToBuffer() - Write the record 8 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord8ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%1d %1d %2d %2d %2d",
			      _sigprocFlag, _interfaceType,
			      _radarPower, _servo, _radiate);
  
  return chars_printed + 1;
}


/*********************************************************************
 * _addRecord9ToBuffer() - Write the record 9 information to the status
 *                         buffer.
 *
 * Returns the number of bytes written to the buffer.
 */

int EdgeStatusMsg::_addRecord9ToBuffer(char *msg_buffer)
{
  int chars_printed = sprintf(msg_buffer,
			      "%04x %04x %04x %04x %04x %04x %3d %1d %1d %7G",
			      _flags, _tcfZ, _tcfU, _tcfV, _tcfW,
			      _clutterFilter, _sqi, _pw, _fold,
			      _radarWavelength);
  
  return chars_printed + 1;
}
