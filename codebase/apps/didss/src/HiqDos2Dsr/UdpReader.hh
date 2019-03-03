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
 * UdpReader: Class for objects used to read beam data from a UDP port.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef UdpReader_hh
#define UdpReader_hh

#include <cstdio>
#include <netinet/in.h>

#include "Reader.hh"

using namespace std;


class UdpReader : public Reader
{

public:

  ////////////////////////////////
  // Constructors & Destructors //
  ////////////////////////////////

  /*********************************************************************
   * Constructor
   */

  UdpReader(const int port, const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  ~UdpReader();


  /*********************************************************************
   * init() - Initialize the reader.
   *
   * Returns true on success, false on failure.
   */

  bool init();


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  int _port;
  int _maxPktSize;
  int _udpFd;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _readBytes() - Read the next group of bytes from the source.
   *
   * Returns the number of bytes read from the source.
   */

  virtual int _readBytes(char *buffer, const int buffer_size);
   

};

#endif

   
