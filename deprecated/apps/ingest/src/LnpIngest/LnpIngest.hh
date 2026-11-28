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
 *   $Date: 2016/03/07 01:23:02 $
 *   $Id: LnpIngest.hh,v 1.2 2016/03/07 01:23:02 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * LnpIngest: LnpIngest program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef LnpIngest_HH
#define LnpIngest_HH

#include <string>
#include <sys/time.h>

#include <toolsa/MemBuf.hh>

#include "Args.hh"
#include "LtgWriter.hh"
#include "Params.hh"

using namespace std;


class LnpIngest
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~LnpIngest(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static LnpIngest *Inst(int argc, char **argv);
  static LnpIngest *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  // Define message constants

  static const unsigned char MSG_BOP;
  static const unsigned char MSG_EOP;
  static const unsigned char MSG_DLE;

  static const unsigned char MSG_STROKE_FLAG;
  
  // Date conversion constants

  static const int SECS_PER_DAY;
  static const int SECS_PER_HOUR;
  static const int SECS_PER_MINUTE;
  

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static LnpIngest *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  int _lnpSockFd;
  
  MemBuf _availBytes;
  
  time_t _refTime;
  
  LtgWriter _writer;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  LnpIngest(int argc, char **argv);
  

  /*********************************************************************
   * _closeSocket() - Close the LNP socket
   */

  void _closeSocket();
  

  /*********************************************************************
   * _extractBits() - Extract the specified bits from the given buffer.
   *
   * Returns the integer value of the extracted bits.
   */

  inline static ui32 _extractBits(const ui32 buffer, int first_bit, int num_bits)
  {
    ui32 mask = 0xffffffff >> first_bit;
    mask = mask | (0x0 << (32 - num_bits));
    
    return (buffer & mask) >> (32 - num_bits - first_bit);
  }
  

  /*********************************************************************
   * _extractStrokeData() - Extract the stroke data values from the
   *                        given fields.
   */

  inline void _extractStrokeData(const ui32 k_longitude_c,
				 const ui32 dmq_latitude,
				 double &latitude,
				 double &longitude,
				 double &amplitude,
				 bool &cloud_to_ground_flag,
				 int &multiplicity,
				 double &seconds_fraction,
				 bool &qa_flag)
  {
    ui32 packed_half_kiloamperes = _extractBits(k_longitude_c, 0, 8);
    if (packed_half_kiloamperes <= 128)
      amplitude = (double)packed_half_kiloamperes * 2.0;
    else
      amplitude = (double)(packed_half_kiloamperes - 128) * -2.0;

    ui32 packed_longitude = _extractBits(k_longitude_c, 8, 24);
    longitude =
      ((double)packed_longitude / (double)(0x1 << 24) * 360.0) - 180.0;
    
    ui32 packed_cloud = _extractBits(k_longitude_c, 31, 1);
    if (packed_cloud)
      cloud_to_ground_flag = false;
    else
      cloud_to_ground_flag = true;
    
    ui32 packed_deciseconds = _extractBits(dmq_latitude, 0, 4);
    seconds_fraction = (double)packed_deciseconds * 0.1;
    
    ui32 packed_multiplicity = _extractBits(dmq_latitude, 4, 4);
    multiplicity = packed_multiplicity;
    
    ui32 packed_quality = _extractBits(dmq_latitude, 8, 1);
    if (packed_quality)
      qa_flag = false;
    else
      qa_flag = true;
    
    ui32 packed_latitude = _extractBits(dmq_latitude, 9, 23);
    latitude = ((double)packed_latitude / (double)(0x1 << 24) * 360.0) - 90.0;
  }
  

  /*********************************************************************
   * _extractTdate() - Extract the tdate values from the given field.
   *
   * Returns unix time for the tdate.
   */

  inline int _extractTdate(const ui32 tdate)
  {
    int day_count = _extractBits(tdate, 0, 15);
    int hour = _extractBits(tdate, 15, 5);
    int minute = _extractBits(tdate, 20, 6);
    int second = _extractBits(tdate, 26, 6);
  
    return _refTime + (day_count * SECS_PER_DAY) + (hour * SECS_PER_HOUR)
      + (minute * SECS_PER_MINUTE) + second;
  }
  

  /*********************************************************************
   * _fillMsg() - Fill the given message buffer with the next message
   *              available.
   *
   * Returns true on success, false on failure.
   */

  bool _fillMsg(MemBuf &msg);
  

  /*********************************************************************
   * _openSocket() - Open the LNP socket
   *
   * Returns true on success, false on failure.
   */

  bool _openSocket();
  

  /*********************************************************************
   * _processMsg() - Process the next message available on the socket
   *
   * Returns true on success, false on failure.
   */

  bool _processMsg();
  

  /*********************************************************************
   * _processStrokeMsg() - Process the given stroke message
   *
   * Returns true on success, false on failure.
   */

  bool _processStrokeMsg(const MemBuf &msg);
  

  /*********************************************************************
   * _printMsg() - Print the given message
   */

  inline static void _printMsg(MemBuf &msg)
  {
    unsigned char *msg_ptr = (unsigned char *)msg.getPtr();
    int msg_len = msg.getLen();
    
    for (int i = 0; i < msg_len; ++i)
    {
      if (msg_ptr[i] == MSG_BOP)
	fprintf(stderr, "BOP ");
      else if (msg_ptr[i] == MSG_EOP)
	fprintf(stderr, "EOP ");
      else if (msg_ptr[i] == MSG_DLE)
	fprintf(stderr, "DLE ");
      else
	fprintf(stderr, "%x ", msg_ptr[i]);
    }
    cerr << endl;
  }
  
  
  /*********************************************************************
   * _removeDLE() - Remove the DLE flags from the given message.
   */

  inline static void _removeDLE(MemBuf &msg)
  {
    MemBuf temp_msg;
    
    unsigned char *msg_ptr = (unsigned char *)msg.getPtr();
    int msg_len = msg.getLen();
    
    int begin_index = 0;
    
    for (int i = 0; i < msg_len; ++i)
    {
      if (msg_ptr[i] == MSG_DLE)
      {
	temp_msg.add(&(msg_ptr[begin_index]), i - begin_index);
	begin_index = ++i;
      }
    } /* endfor - i */
    
    temp_msg.add(&(msg_ptr[begin_index]), msg_len - begin_index);

    msg = temp_msg;
  }
  
  
};


#endif
