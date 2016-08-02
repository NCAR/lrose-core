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
//   $Id: MitLtg.cc,v 1.10 2016/03/03 18:45:40 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MitLtg: class controlling access to a lightning strike object in
 *         MIT/Lincoln Labs format.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cstdlib>

#include <dataport/bigend.h>
#include <rapformats/MitLtg.hh>
using namespace std;


// Define global constants

const int MitLtg::MIT_ARSI_MSG_LEN = 30;
const int MitLtg::MIT_LLP_MSG_LEN = 20;


/**********************************************************************
 * Constructors
 */

MitLtg::MitLtg() :
  _strikeTime(DateTime::NEVER),
  _millisecs(0),
  _lat(0.0),
  _lon(0.0),
  _strength(0.0),
  _type(0),
  _statusMask(0),
  _numStrokes(1)
{
}

MitLtg::MitLtg(const KAVLTG_strike_t &kav_strike) :
  _strikeTime(kav_strike.time),
  _millisecs(0),
  _lat((double)kav_strike.latitude / 1000.0),
  _lon((double)kav_strike.longitude / 1000.0),
  _strength((double)kav_strike.amplitude / 1000.0),
  _type(0),
  _statusMask(0),
  _numStrokes(1)
{
}


/**********************************************************************
 * Destructor
 */

MitLtg::~MitLtg(void)
{
}
  

/**********************************************************************
 * assembleMitArsiMsg() - Assemble the MIT ARSI ltg message into an
 *                        internal buffer.  This buffer can then be sent
 *                        to a client using the getMsgBuffer() method.
 */

void MitLtg::assembleMitArsiMsg()
{
  static const string method_name = "MitLtg::assembleMitArsiMsg()";
  
  // Set the message values in native format

  mit_ltg_arsi_t ltg_msg;
  
  _strikeTime.getAll(&ltg_msg.time.year,
		     &ltg_msg.time.month,
		     &ltg_msg.time.day,
		     &ltg_msg.time.hour,
		     &ltg_msg.time.minute,
		     &ltg_msg.time.second);
  
  ltg_msg.millisecs = _millisecs;
  ltg_msg.lat = (long)(_lat * 10000.0 + 0.5);
  ltg_msg.lon = (long)(_lon * 10000.0 + 0.5);
  ltg_msg.strength = (long)(_strength * 1000.0 + 0.5);
  ltg_msg.type = _type;
  ltg_msg.status = _statusMask;
  
  // Swap the numbers to big-endian format

  BE_from_array_16(&ltg_msg.time, sizeof(ltg_msg.time));

  ltg_msg.millisecs = BE_from_si16(ltg_msg.millisecs);
  ltg_msg.lat =       BE_from_si32(ltg_msg.lat);
  ltg_msg.lon =       BE_from_si32(ltg_msg.lon);
  ltg_msg.strength =  BE_from_si32(ltg_msg.strength);
  ltg_msg.type =      BE_from_si16(ltg_msg.type);
  ltg_msg.status =    BE_from_si16(ltg_msg.status);
  
  // Put the message information in the message buffer.  Note that
  // the structure fields have to be added to the buffer individually
  // because the fields do not start on word boundaries so there is
  // extra space between some of the fields in memory.

  _msgBuffer.load(&ltg_msg.time.month, sizeof(ltg_msg.time.month));
  _msgBuffer.add(&ltg_msg.time.day, sizeof(ltg_msg.time.day));
  _msgBuffer.add(&ltg_msg.time.year, sizeof(ltg_msg.time.year));
  _msgBuffer.add(&ltg_msg.time.hour, sizeof(ltg_msg.time.hour));
  _msgBuffer.add(&ltg_msg.time.minute, sizeof(ltg_msg.time.minute));
  _msgBuffer.add(&ltg_msg.time.second, sizeof(ltg_msg.time.second));
  
  _msgBuffer.add(&ltg_msg.millisecs, sizeof(ltg_msg.millisecs));
  _msgBuffer.add(&ltg_msg.lat, sizeof(ltg_msg.lat));
  _msgBuffer.add(&ltg_msg.lon, sizeof(ltg_msg.lon));
  _msgBuffer.add(&ltg_msg.strength, sizeof(ltg_msg.strength));
  _msgBuffer.add(&ltg_msg.type, sizeof(ltg_msg.type));
  _msgBuffer.add(&ltg_msg.status, sizeof(ltg_msg.status));

  // Check for message errors

  if (_msgBuffer.getBufLen() != MIT_ARSI_MSG_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Message buffer is incorrect size" << endl;
    cerr << "Buffer contains " << _msgBuffer.getBufLen() << 
      " bytes, should contain " << MIT_ARSI_MSG_LEN << " bytes" << endl;
    cerr << "**** Exiting ****" << endl;
    
    exit(0);
  }
}


/**********************************************************************
 * assembleMitLlpMsg() - Assemble the MIT LLP ltg message into an
 *                       internal buffer.  This buffer can then be sent
 *                       to a client using the getMsgBuffer() method.
 */

void MitLtg::assembleMitLlpMsg()
{
  static const string method_name = "MitLtg::assembleMitLlpMsg()";
  
  // Set the message values in native format

  mit_ltg_llp_t ltg_msg;
  
  _strikeTime.getAll(&ltg_msg.time.year,
		     &ltg_msg.time.month,
		     &ltg_msg.time.day,
		     &ltg_msg.time.hour,
		     &ltg_msg.time.minute,
		     &ltg_msg.time.second);
  
  ltg_msg.lat = (long)(_lat * 100.0 + 0.5);
  ltg_msg.lon = (long)(_lon * 100.0 + 0.5);
  ltg_msg.strength = (long)(_strength * 10.0 + 0.5);
  ltg_msg.mult = _numStrokes;
  
  // Swap the numbers to big-endian format

  BE_from_array_16(&ltg_msg.time, sizeof(ltg_msg.time));

  ltg_msg.lat =       BE_from_si16(ltg_msg.lat);
  ltg_msg.lon =       BE_from_si16(ltg_msg.lon);
  ltg_msg.strength =  BE_from_si16(ltg_msg.strength);
  ltg_msg.mult =      BE_from_si16(ltg_msg.mult);
  
  // Put the message information in the message buffer.  Note that
  // the structure fields have to be added to the buffer individually
  // because the fields do not start on word boundaries so there is
  // extra space between some of the fields in memory.

  _msgBuffer.load(&ltg_msg.time.month, sizeof(ltg_msg.time.month));
  _msgBuffer.add(&ltg_msg.time.day, sizeof(ltg_msg.time.day));
  _msgBuffer.add(&ltg_msg.time.year, sizeof(ltg_msg.time.year));
  _msgBuffer.add(&ltg_msg.time.hour, sizeof(ltg_msg.time.hour));
  _msgBuffer.add(&ltg_msg.time.minute, sizeof(ltg_msg.time.minute));
  _msgBuffer.add(&ltg_msg.time.second, sizeof(ltg_msg.time.second));
  
  _msgBuffer.add(&ltg_msg.lat, sizeof(ltg_msg.lat));
  _msgBuffer.add(&ltg_msg.lon, sizeof(ltg_msg.lon));
  _msgBuffer.add(&ltg_msg.strength, sizeof(ltg_msg.strength));
  _msgBuffer.add(&ltg_msg.mult, sizeof(ltg_msg.mult));

  // Check for message errors

  if (_msgBuffer.getBufLen() != MIT_LLP_MSG_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Message buffer is incorrect size" << endl;
    cerr << "Buffer contains " << _msgBuffer.getBufLen() << 
      " bytes, should contain " << MIT_LLP_MSG_LEN << " bytes" << endl;
    cerr << "**** Exiting ****" << endl;
    
    exit(0);
  }
}


/**********************************************************************
 * disassembleMitArsiMsg() - Disassemble the MIT ARSI ltg message received
 *                           from an MIT server into the object information.
 *
 * Returns true if successful, false otherwise.
 */

bool MitLtg::disassembleMitArsiMsg(const void *msg_buffer,
				   const int msg_len)
{
  static const string method_name = "MitLtg::disassembleMitArsiMsg()";
  
  // Check for a message error

  if (msg_len != MIT_ARSI_MSG_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Expected " << MIT_ARSI_MSG_LEN << " bytes, got " <<
      msg_len << " bytes" << endl;
    cerr << "*** Skipping ltg message ***" << endl;
    
    return false;
  }
  
  // Copy the message information into a local buffer.  Note that
  // we have to copy each structure field individually because some
  // of the fields don't start on word boundaries so there is unused
  // space in the middle of the structure.

  mit_ltg_arsi_t ltg_msg;
  char *buf_ptr = (char *)msg_buffer;
  
  memcpy(&ltg_msg.time.month, buf_ptr, sizeof(ltg_msg.time.month));
  buf_ptr += sizeof(ltg_msg.time.month);
  
  memcpy(&ltg_msg.time.day, buf_ptr, sizeof(ltg_msg.time.day));
  buf_ptr += sizeof(ltg_msg.time.day);
  
  memcpy(&ltg_msg.time.year, buf_ptr, sizeof(ltg_msg.time.year));
  buf_ptr += sizeof(ltg_msg.time.year);
  
  memcpy(&ltg_msg.time.hour, buf_ptr, sizeof(ltg_msg.time.hour));
  buf_ptr += sizeof(ltg_msg.time.hour);
  
  memcpy(&ltg_msg.time.minute, buf_ptr, sizeof(ltg_msg.time.minute));
  buf_ptr += sizeof(ltg_msg.time.minute);
  
  memcpy(&ltg_msg.time.second, buf_ptr, sizeof(ltg_msg.time.second));
  buf_ptr += sizeof(ltg_msg.time.second);
  
  memcpy(&ltg_msg.millisecs, buf_ptr, sizeof(ltg_msg.millisecs));
  buf_ptr += sizeof(ltg_msg.millisecs);
  
  memcpy(&ltg_msg.lat, buf_ptr, sizeof(ltg_msg.lat));
  buf_ptr += sizeof(ltg_msg.lat);
  
  memcpy(&ltg_msg.lon, buf_ptr, sizeof(ltg_msg.lon));
  buf_ptr += sizeof(ltg_msg.lon);
  
  memcpy(&ltg_msg.strength, buf_ptr, sizeof(ltg_msg.strength));
  buf_ptr += sizeof(ltg_msg.strength);
  
  memcpy(&ltg_msg.type, buf_ptr, sizeof(ltg_msg.type));
  buf_ptr += sizeof(ltg_msg.type);
  
  memcpy(&ltg_msg.status, buf_ptr, sizeof(ltg_msg.status));
  buf_ptr += sizeof(ltg_msg.status);
  
  // Swap the bytes in the message
  
  BE_to_array_16(&ltg_msg.time, sizeof(ltg_msg.time));

  ltg_msg.millisecs = BE_to_si16(ltg_msg.millisecs);
  ltg_msg.lat =       BE_to_si32(ltg_msg.lat);
  ltg_msg.lon =       BE_to_si32(ltg_msg.lon);
  ltg_msg.strength =  BE_to_si32(ltg_msg.strength);
  ltg_msg.type =      BE_to_si16(ltg_msg.type);
  ltg_msg.status =    BE_to_si16(ltg_msg.status);
  
  // Update the local object members with the new ltg values

  _strikeTime.set(ltg_msg.time.year,
		  ltg_msg.time.month,
		  ltg_msg.time.day,
		  ltg_msg.time.hour,
		  ltg_msg.time.minute,
		  ltg_msg.time.second);
  
  _millisecs = ltg_msg.millisecs;
  _lat = (double)ltg_msg.lat / 10000.0;
  _lon = (double)ltg_msg.lon / 10000.0;
  _strength = (double)ltg_msg.strength / 1000.0;
  _type = ltg_msg.type;
  _statusMask = ltg_msg.status;

  _numStrokes = 1;
  
  return true;
}


/**********************************************************************
 * disassembleMitLlpMsg() - Disassemble the MIT LLP ltg message received
 *                          from an MIT server into the object information.
 *
 * Returns true if successful, false otherwise.
 */

bool MitLtg::disassembleMitLlpMsg(const void *msg_buffer,
				  const int msg_len)
{
  static const string method_name = "MitLtg::disassembleMitLlpMsg()";
  
  // Check for a message error

  if (msg_len != MIT_LLP_MSG_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Expected " << MIT_LLP_MSG_LEN << " bytes, got " <<
      msg_len << " bytes" << endl;
    cerr << "*** Skipping ltg message ***" << endl;
    
    return false;
  }
  
  // Copy the message information into a local buffer.  Note that
  // we have to copy each structure field individually because some
  // of the fields don't start on word boundaries so there is unused
  // space in the middle of the structure.

  mit_ltg_llp_t ltg_msg;
  char *buf_ptr = (char *)msg_buffer;
  
  memcpy(&ltg_msg.time.month, buf_ptr, sizeof(ltg_msg.time.month));
  buf_ptr += sizeof(ltg_msg.time.month);
  
  memcpy(&ltg_msg.time.day, buf_ptr, sizeof(ltg_msg.time.day));
  buf_ptr += sizeof(ltg_msg.time.day);
  
  memcpy(&ltg_msg.time.year, buf_ptr, sizeof(ltg_msg.time.year));
  buf_ptr += sizeof(ltg_msg.time.year);
  
  memcpy(&ltg_msg.time.hour, buf_ptr, sizeof(ltg_msg.time.hour));
  buf_ptr += sizeof(ltg_msg.time.hour);
  
  memcpy(&ltg_msg.time.minute, buf_ptr, sizeof(ltg_msg.time.minute));
  buf_ptr += sizeof(ltg_msg.time.minute);
  
  memcpy(&ltg_msg.time.second, buf_ptr, sizeof(ltg_msg.time.second));
  buf_ptr += sizeof(ltg_msg.time.second);
  
  memcpy(&ltg_msg.lat, buf_ptr, sizeof(ltg_msg.lat));
  buf_ptr += sizeof(ltg_msg.lat);
  
  memcpy(&ltg_msg.lon, buf_ptr, sizeof(ltg_msg.lon));
  buf_ptr += sizeof(ltg_msg.lon);
  
  memcpy(&ltg_msg.strength, buf_ptr, sizeof(ltg_msg.strength));
  buf_ptr += sizeof(ltg_msg.strength);
  
  memcpy(&ltg_msg.mult, buf_ptr, sizeof(ltg_msg.mult));
  buf_ptr += sizeof(ltg_msg.mult);
  
  // Swap the bytes in the message
  
  BE_to_array_16(&ltg_msg.time, sizeof(ltg_msg.time));

  ltg_msg.lat =       BE_to_si16(ltg_msg.lat);
  ltg_msg.lon =       BE_to_si16(ltg_msg.lon);
  ltg_msg.strength =  BE_to_si16(ltg_msg.strength);
  ltg_msg.mult =      BE_to_si16(ltg_msg.mult);
  
  // Update the local object members with the new ltg values

  _strikeTime.set(ltg_msg.time.year,
		  ltg_msg.time.month,
		  ltg_msg.time.day,
		  ltg_msg.time.hour,
		  ltg_msg.time.minute,
		  ltg_msg.time.second);
  
  _lat = (double)ltg_msg.lat / 100.0;
  _lon = (double)ltg_msg.lon / 100.0;
  _strength = (double)ltg_msg.strength / 10.0;
  _numStrokes = ltg_msg.mult;
  
  _millisecs = 0;
  _type = 0;
  _statusMask = 0;

  return true;
}


/**********************************************************************
 * print() - Print the strike information to the indicated stream.
 */

void MitLtg::print(ostream &out,
		   const string &spacer) const
{
  out << spacer << "time: " << DateTime::str(_strikeTime.utime()) << " (" << _millisecs <<
      " ms)" << endl;
  out << spacer << "lat: " << _lat << endl;
  out << spacer << "lon: " << _lon << endl;
  out << spacer << "strength: " << _strength << " amps" << endl;
  out << spacer << "type: " << _type << endl;
  out << spacer << "status mask: " << _statusMask << endl;
  out << spacer << "mult: " << _numStrokes << endl;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
