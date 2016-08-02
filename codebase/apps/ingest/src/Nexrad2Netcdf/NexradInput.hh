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
//////////////////////////////////////////////////////////
// NexradInput - handles decompression and buffering of ldm
//           files
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
// Note: Decompression code copied from NexradBzUncomp.
// See notes in that code to track history.
//
// May 2005
//
// $Id: NexradInput.hh,v 1.4 2016/03/07 01:23:03 dixon Exp $
//
///////////////////////////////////////////////////////////
#ifndef _NEXRAD_INPUT_HH
#define _NEXRAD_INPUT_HH

#include <string>
using namespace std;

class NexradInput {
  
public:

   //
   // Constructor
   //
   NexradInput();

   //
   // Destructor
   //
   ~NexradInput();

   //
   // Initialize
   //
   void init( bool oneFilePerVolume, bool ppiFiles );

   //
   // Run the uncompression routine on the file
   //   filePath   = path to file that we want to uncompress
   //   decompress = do we need to decompress the data?
   //
   //   returns -1 on failure, 0 on success
   //
   int readFile( char* filePath, bool decompress );

   //
   // Get information from file
   //
   bool           cookie(){ return cookieFound; }
   int            getBufferSize(){ return bufferSize; }
   int            getBytesRead(){ return bytesInBuffer; }
   unsigned char* getHeader(){ return header; }
   unsigned char* getBuffer(){ return ldmBuffer; }

   //
   // Constants
   //
   static const int MAX_FILE_LEN;

private:
   
   bool volumeFiles;
   bool ppiFiles;
   bool cookieFound;

   int headerSize;
   int bufferSize;

   int bytesInBuffer;

   unsigned char *header;
   unsigned char *ldmBuffer;
   unsigned char *bufferPtr;

   void clear();

   //
   // Read in the data files containing 100 beams or less
   //   filePath   = path to fiel that we want to read
   //   decompress = do we need to decompress the data?
   //
   //   returns -1 on failure, 0 on success
   //
   int readHundredBeamFile( char* filePath, bool decompress );

   //
   // Read in the volume files. It is currently assumed
   // that these files are not compressed
   //   filePath   = path to file that we want to read
   //   decompress = do we need to decompress the data?
   //                Note that currently we do not support
   //                compressed volume files.
   //
   //   returns -1 on failure, 0 on success
   //
   int readVolumeFile( char* filePath, bool decompress );

};

#endif

