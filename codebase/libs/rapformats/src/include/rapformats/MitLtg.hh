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
 *   $Id: MitLtg.hh,v 1.7 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.7 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MitLtg: class controlling access to a lightning strike object in
 *         MIT/Lincoln Labs format.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MitLtg_HH
#define MitLtg_HH



#include <string>
#include <sys/time.h>

#include <dataport/port_types.h>
#include <rapformats/kavltg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>
using namespace std;


class MitLtg
{
 public:

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /**********************************************************************
   * Constructors
   */

  MitLtg();
  MitLtg(const KAVLTG_strike_t &kav_strike);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~MitLtg(void);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * setStrike() - Set the strike values based on the given information.
   */

  inline void setStrike(const KAVLTG_strike_t &kav_strike)
  {
    _strikeTime.set(kav_strike.time);
    _millisecs = 0;
    _lat = (double)kav_strike.latitude * 1000.0;
    _lon = (double)kav_strike.longitude * 1000.0;
    _strength = (double)kav_strike.amplitude * 1000.0;
    _type = 0;
    _statusMask = 0;
  }
  

  /**********************************************************************
   * getTime() - Get the time of the strike.
   */

  inline time_t getUnixTime(void) const
  {
    return _strikeTime.utime();
  }
  

  /**********************************************************************
   * getLocation() - Get the location of the strike.
   */

  inline void getLocation(double &lat, double &lon) const
  {
    lat = _lat;
    lon = _lon;
  }
  
  inline void getLocation(float &lat, float &lon) const
  {
    lat = _lat;
    lon = _lon;
  }
  

  /**********************************************************************
   * getStrength() - Get the strength of the strike in amps.
   */

  inline double getStrength(void) const
  {
    return _strength;
  }
  

  /////////////////
  // I/O methods //
  /////////////////

  /**********************************************************************
   * assembleMitArsiMsg() - Assemble the MIT ARSI ltg message into an
   *                        internal buffer.  This buffer can then be sent
   *                        to a client using the getMsgBuffer() method.
   */

  void assembleMitArsiMsg();
  

  /**********************************************************************
   * assembleMitLlpMsg() - Assemble the MIT LLP ltg message into an
   *                       internal buffer.  This buffer can then be sent
   *                       to a client using the getMsgBuffer() method.
   */

  void assembleMitLlpMsg();
  

  /**********************************************************************
   * getMsgBuffer() - Get a pointer to the latest assembled message
   *                  buffer.
   */

  const void *getMsgBuffer() const
  {
    return _msgBuffer.getBufPtr();
  }
  

  /**********************************************************************
   * getMsgBufferLen() - Get the length (number of bytes) of the latest
   *                     assembled message buffer.
   */

  int getMsgBufferLen() const
  {
    return _msgBuffer.getBufLen();
  }
  

  /**********************************************************************
   * disassembleMitArsiMsg() - Disassemble the MIT ARSI ltg message received
   *                           from an MIT server into the object information.
   *
   * Returns true if successful, false otherwise.
   */

  bool disassembleMitArsiMsg(const void *msg_buffer,
			     const int msg_len);
  

  /**********************************************************************
   * disassembleMitLlpMsg() - Disassemble the MIT LLP ltg message received
   *                          from an MIT server into the object information.
   *
   * Returns true if successful, false otherwise.
   */

  bool disassembleMitLlpMsg(const void *msg_buffer,
			    const int msg_len);
  

  /**********************************************************************
   * print() - Print the strike information to the indicated stream.
   */

  void print(ostream &out,
	     const string &spacer = "") const;


 private:

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    si16 month;
    si16 day;
    si16 year;
    si16 hour;
    si16 minute;
    si16 second;
  } mit_time6_t;
  
  typedef struct
  {
    mit_time6_t time;
    si16 millisecs;
    si32 lat;             // latitude * 100000
    si32 lon;             // longitude * 10000
    si32 strength;        // strength & polarity * 1000
    si16 type;
    si16 status;
  } mit_ltg_arsi_t;
  
  typedef struct
  {
    mit_time6_t time;
    si16 lat;             // latitude * 100
    si16 lon;             // longitude * 100
    si16 strength;        // strength & polarity * 10
    si16 mult;            // number of strokes in the strike
  } mit_ltg_llp_t;
  
  
  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int MIT_ARSI_MSG_LEN;   // Msg len in bytes
  static const int MIT_LLP_MSG_LEN;   // Msg len in bytes
  

  /////////////////////
  // Private members //
  /////////////////////

  // lightning strike information

  DateTime _strikeTime;
  long _millisecs;          // Add to strike_time to get precise time.
  
  double _lat;
  double _lon;
  
  double _strength;        // Sign indicates polarity.  Value in amps.
  
  int _type;               // Can't enum the type yet because I don't know the values.
  
  int _statusMask;         // Bits to be defined.

  int _numStrokes;
  
  
  // Message buffer for sending/receiving strike data to/from other processes

  MemBuf _msgBuffer;
  
};


#endif
