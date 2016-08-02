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
// Surfer2Mdv.hh
//
// Surfer2Mdv object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1999
//
///////////////////////////////////////////////////////////////

#ifndef Surfer2Mdv_H
#define Surfer2Mdv_H

#include <Mdv/DsMdvx.hh>
#include <string>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class MemBuf;

////////////////////////
// This class

class Surfer2Mdv {
  
public:

  // constructor

  Surfer2Mdv (int argc, char **argv);

  // destructor
  
  ~Surfer2Mdv();

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

  int _processFile(const string &inputPath);

  int _computeTime(const string &inputPath,
		   time_t &start_time,
		   time_t &mid_time,
		   time_t &end_time);

  int _readInput(const string &inputPath,
		 int &nx, int &ny,
		 double &minx, double &miny,
		 double &dx, double &dy,
		 MemBuf &dataBuf);

  void _convertData(MemBuf &dataBuf);

  int _writeOutput(time_t start_time,
		   time_t mid_time,
		   time_t end_time,
		   int nx, int ny,
		   double minx, double miny,
		   double dx, double dy,
		   MemBuf &dataBuf);

  void _setFieldName(Mdvx::field_header_t &fhdr,
		     const char *name,
		     const char *name_long,
		     const char *units,
		     const char *transform,
		     const int field_code);
  
};

#endif

