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


#ifndef LL_ASOS_H
#define LL_ASOS_H


#include <cstdio>
#include <toolsa/Socket.hh>

#include <string>
#include <map>
#include <Spdb/DsSpdb.hh>

#include <vector>

#include "StationLoc.hh"

#include "Params.hh"
using namespace std;


class llAsosIngest {
 
public:
 
  // constructor. Does nothing.
 
  llAsosIngest(Params *P);
 
  // destructor. Does nothing.
 
  ~llAsosIngest();    
    
  // public method.
 
  void ProcFile(char *FilePath, Params *P);



protected:
 
private:
 

  int _openReadSource(Params *P, char *FileName);
  void _closeReadSource();
  int _readFromSource(unsigned char *buffer, int numbytes);
  void _printData(unsigned char *bf);
  int _bitSet(unsigned char *bf, int byteNum, int bitNum);
  void _printPrecip(unsigned char byte, int higherBits);
  void _writeSpdb(unsigned char *buffer, Params *P);
  int _loadLocations(const char* station_location_path,
		     int altInFeet);
  int _getPrecip(unsigned char byte, int higherBits);

  map< string, StationLoc, less<string> > _locations;

  typedef struct {
    char ID[5];
    double accum;
  } accumSave_t;

  vector <accumSave_t> _pastAccums;

  //
  // Data.
  //  
  bool _readingFromFile;
  FILE *_fp;
  Socket _S;

  DsSpdb _outSpdb;
  int _numInBuffer;

  int _socketTimeoutSecs;

};  

#endif
   
