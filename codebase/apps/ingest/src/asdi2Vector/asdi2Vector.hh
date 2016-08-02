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


#ifndef ASDI_VECTOR_H
#define ASDI_VECTOR_H


#include <cstdio>
#include <toolsa/Socket.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/acPosVector.hh>
#include <vector>

#include "Params.hh"


class asdi2Vector {
 
public:
 
  // constructor. Does nothing.
 
  asdi2Vector(Params *tdrpParams);
 
  // destructor. Does nothing.
 
  ~asdi2Vector();    
    
  // public method.
 
  void ProcFile(char *FilePath, Params *P);



protected:
 
private:

  int _openReadSource(Params *P, char *FileName);
  void _closeReadSource();
  int _readFromSource(char *buffer, int numbytes);

  void _processAsdiMsg(char *asdiMsg);

  void _extractString(char *instring, char *outstring);
  void _getTime(date_time_t *T, 
		int A_day, int A_hour, 
		int A_min, int A_sec);

  void _save2ascii(char *asdiMsg);

  //
  // Data.
  //  
  bool _readingFromFile;
  FILE *_fp;
  Socket _S;

  Params *_params;

  int _filenameYear;
  int _filenameMonth;

  int _count;
  const static int _internalStringLen = 64;

  int _spdbBufferCount;
  DsSpdb _spdb;

  const static int _asciiBufferLen = 8192;
  char _asciiBuffer[_asciiBufferLen];


  vector <acPosVector> _acVectors;

  time_t _lastCleanoutTime;

};  

#endif
   
