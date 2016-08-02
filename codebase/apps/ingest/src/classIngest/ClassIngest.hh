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
// classIngest.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// March 2014.
//
// Code originally by Terri Betancourt.
//
///////////////////////////////////////////////////////////////
//
// classIngest reads automated sounding observations in CLASS
// format, and writes them to an SPDB data base
//
////////////////////////////////////////////////////////////////

#ifndef classIngest_H
#define classIngest_H

#include <string>
#include <map>
#include "Args.hh"
#include "Params.hh"
using namespace std;
class SoundingPut;

////////////////////////
// This class

class ClassIngest {
  
public:

  // constructor

  ClassIngest (int argc, char **argv);

  // destructor
  
  ~ClassIngest();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // consts

  static const char* INDEX_FILENAME;
  static const char* DELIMETER;
  static const double MISSING_VALUE;

  // Fields to be read out of the CLASS file
  
  static const unsigned int NFIELDS_IN;
  static const char* HEIGHT_LABEL;
  static const char* U_WIND_LABEL;
  static const char* V_WIND_LABEL;
  static const char* PRESSURE_LABEL;
  static const char* REL_HUM_LABEL;
  static const char* TEMPERATURE_LABEL;
  
  static const int MAX_POINTS = 16384;
  double height[MAX_POINTS];
  double uwind[MAX_POINTS];
  double vwind[MAX_POINTS];
  double pressure[MAX_POINTS];
  double relHum[MAX_POINTS];
  double temperature[MAX_POINTS];
  
  map<int, double*, less<int> > columnData;

  int _numPoints;
  time_t _dataTime;
  time_t _launchTime;

  int _processFile(const char *file_path);
  int _readHeader(FILE *in, SoundingPut &sounding);
  int _getHeaderText(FILE *in, const char* label, string &text);
  int _findColumns(FILE *in);
  int _findFirstData(FILE *in);
  int _readData(FILE *in);
  int _writeSounding(SoundingPut &sounding);

};

#endif

