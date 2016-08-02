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


#ifndef MIGFA_H
#define MIGFA_H


#include <cstdio>
#include <toolsa/Socket.hh>


#include "Params.hh"
using namespace std;


class Migfa2Spdb {
 
public:
 
  // constructor. Does nothing.
 
  Migfa2Spdb();
 
  // destructor. Does nothing.
 
  ~Migfa2Spdb();    
    
  // public method.
 
  void ProcFile(char *FilePath, Params *P);



protected:
 
private:
 
  //
  // Sizes of records, after id has been read.
  //
  static const int frameStartRecordSize = 26;
  static const int frameStartRecordID = 1470;
 
  static const int tiltHeaderRecordSize = 100;
  static const int tiltHeaderRecordID = 2001;
 
  static const int tiltTrailerRecordSize = 20;
  static const int tiltTrailerRecordID = 2090;
 
  static const int frameEndRecordSize = 26;
  static const int frameEndRecordID = 1471;         
 
  static const int detectionRecordSize = 52; // Actually depends on # points
  static const int detectionRecordID = 2901;
 
  static const int forecastRecordSize = 32; // Actually depends on # points
  static const int forecastRecordID = 2905;      

  static const int areYouThereID = 90;   

  const static int MaxNumPoints = 1000;
  //
  // Methods.
  //
  int  decode2bytes(unsigned char *b, bool byteSwap );
  unsigned int  decode2bytesUnsigned(unsigned char *b, bool byteSwap );

  long decode4bytes(unsigned char *b, bool byteSwap );
  void decodeTime( unsigned char *b, date_time_t *dataTime, 
		   long leadTime, bool byteSwap );
  void ConvertXY2LatLon(int num,
			double *x,
			double *y,
			double *lat,
			double *lon,
			double radar_lat,
			double radar_lon);

  int openReadSource(Params *P, char *FileName);
  void closeReadSource();
  int readFromSource(unsigned char *buffer, int numbytes);

  //
  // Data.
  //  
  bool _readingFromFile;
  FILE *_fp;
  Socket _S;

};  

#endif
   
