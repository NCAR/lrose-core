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

/////////////////////////////////////////////////////////////
// basicIO.hh
//
// Reads bytes for super resolution nexrad data. Will do it
// from file, or from socket, depending on which constructor
// is called.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef BASIC_IO_H
#define BASIC_IO_H

#include <toolsa/Socket.hh>

#include "Params.hh"

using namespace std;

class basicIO {
  
public:

  // Constructor, from filename, file IO assumed.
  basicIO ( char *fileName, Params *TDRP_params );

  // Constructor, from hostname and port, socket IO assumed.
  basicIO ( char *hostName, int portNum, Params *TDRP_params );

  // Get status.
  bool isOk();

  // Get read mode.
  int getReadMode();

  // See if we are at the end of a file.
  bool atEnd();

  // Read some bytes. User must allocate space. Returns
  // the number of bytes actually read. If the number of
  // bytes read is different from what was asked for then
  // isOk() will return false.
  int readBytes(int numBytesToRead, unsigned char *buffer);

  // Destructor.
  ~basicIO ();

  const static int _readingFromFile = 0;
  const static int _readingFromSocket = 1;

protected:
  
private:

  int _readMode; // Set to one of the constants above
  Params *_params;
  FILE *_fp;
  bool _ok;
  long _fileSize;

  Socket _socket;

};

#endif





