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


#ifndef ASDI_H
#define ASDI_H


#include <stdio.h>
#include <toolsa/Socket.hh>

#include "Params.hh"
#include "Gridder.hh"


class asdi2mdv {
 
public:
 
  // constructor. Does nothing.
 
  asdi2mdv(Params *tdrpParams);
 
  // destructor. Does nothing.
 
  ~asdi2mdv();    
    
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


  Gridder *_gridder;
  //
  // Data.
  //  
  bool _readingFromFile;
  FILE *_fp;
  Socket _S;

  Params *_params;

  int _count;
  const static int _internalStringLen = 64;

};  

#endif
   
