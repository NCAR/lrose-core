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
// Ingester.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
// Copied from Grib2Mdv
//
///////////////////////////////////////////////////////////////

#ifndef _INGESTER_
#define _INGESTER_

#include <ctime>
#include <string>
#include <vector>
#include <list>
#include <toolsa/MemBuf.hh>
#include "GribMgr.hh"
#include "InputField.hh"
#include "Params.hh"

using namespace std;

class SawsUm2Mdv;

class Ingester {
public:


  Ingester(SawsUm2Mdv &parent,
	   const string &prog_name,
	   const Params &params);

  virtual ~Ingester();
 
  /// read a single file

  int readFile(const string& filePath);
  
  /// Load the next GRIB record from the file into _gribBuf.
  /// Generally you call this routine repeatedly.
  /// _gribBuf is loaded with one entire grib record.
  /// This routine may set _eofFound to true, so stop reading then.
  /// \param file the open file being processed
  /// \return true if a record was successfully loaded

  bool loadNextGrib(FILE *file);

  // get methods
  
  int getForecastTime() const { return(_gribMgr.getForecastTime()); }
  time_t getGenerateTime() const { return(_gribMgr.getGenerateTime()); }
  fl32 getMissingVal() const { return(_missingVal); }
  bool getEofFound() const { return _eofFound; }
  const GribMgr& getGribMgr() const { return(_gribMgr); }
  const vector<InputField*> &getInputFields() const { return(_fields); }

private:

  SawsUm2Mdv &_parent;
  const string &_progName;
  const Params &_params;
  bool _eofFound;
  MemBuf _gribBuf;
  GribMgr _gribMgr;

  // vector of GRIB field information structs

  vector<InputField*> _fields;

  // Missing value

  fl32 _missingVal;

  // private functions
  
  bool _outputFieldExists(const int field_id, int& uc);
  InputField* _getField(const int field_id);
  void _clearInputFields();

};

#endif
