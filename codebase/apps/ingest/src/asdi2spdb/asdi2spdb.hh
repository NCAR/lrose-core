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


#include <cstdio>
#include <toolsa/Socket.hh>
#include <Spdb/DsSpdb.hh>

#include "Params.hh"
#include "routeDecode.hh"

class asdi2spdb {
 
public:
 
#define MACH1 661
  // constructor. Does nothing.
 
  asdi2spdb(Params *tdrpParams);
 
  // destructor. Does nothing.
 
  ~asdi2spdb();    
    
  // public method.
 
  void ProcFile(char *FilePath, Params *P);

  // Flush out the buffers to file and SPDB
  void flush();


protected:
 
private:

  int _openReadSource(Params *P, char *FileName);
  void _closeReadSource();
  int _readFromSource(char *buffer, int numbytes);

  void _processAsdiMsg(char *asdiMsg);
  void _contriveDataType(char *fltID, si32 *dataType, si32 *dataType2);

  void _extractString(char *instring, char *outstring);
  void _getTime(date_time_t *T, 
		int A_day, int A_hour, 
		int A_min, int A_sec);

  bool _saveRouteToSPDB(const si32 data_type, const time_t valid_time,
			const time_t expire_time, const int chunk_len,
			const void *chunk_data, const si32 data_type2);
  void _save2ascii(char *asdiMsg);

  void _parseTZ(char *tzMsg, date_time_t T);
  void _parseFZ(char *fzMsg, date_time_t T);
  void _parseAF(char *afMsg, date_time_t T);
  void _parseUZ(char *uzMsg, date_time_t T);
  void _parseAZ(char *azMsg, date_time_t T);
  void _parseDZ(char *dzMsg, date_time_t T);
  void _parseRZ(char *rzMsg, date_time_t T);
  void _parseTO(char *toMsg, date_time_t T);

  //
  // Data.
  //  
  bool _readingFromFile;
  char * _inputFile;
  int _inputFileYear;
  int _inputFileMonth;
  FILE *_fp;
  Socket _S;

  Params *_params;

  routeDecode *_route;
  int _count;
  const static int _internalStringLen = 64;

  int _spdbBufferCount;
  DsSpdb _spdb;
  int _spdbRouteBufferCount;
  DsSpdb _spdbRoute;

  const static int _asciiBufferLen = 8192;
  char _asciiBuffer[_asciiBufferLen];



};  

#endif
   
