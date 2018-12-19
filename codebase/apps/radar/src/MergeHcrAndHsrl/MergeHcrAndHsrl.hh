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
// MergeHcrAndHsrl.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2018
//
///////////////////////////////////////////////////////////////
//
// Merges field data from HCR and HSRL instruments.
// Writes combined data into CfRadial files.
//
////////////////////////////////////////////////////////////////

#ifndef MergeHcrAndHsrl_H
#define MergeHcrAndHsrl_H

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxVol.hh>
class RadxFile;
class RadxRay;
using namespace std;

class MergeHcrAndHsrl {
  
public:

  // constructor
  
  MergeHcrAndHsrl (int argc, char **argv);

  // destructor
  
  ~MergeHcrAndHsrl();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  string _hsrlPath;
  RadxVol _hsrlVol;
  RadxTime _hsrlVolStartTime;
  RadxTime _hsrlVolEndTime;
  RadxTime _searchFailureLowerLimit;
  RadxTime _searchFailureUpperLimit;
  size_t _hsrlRayIndex;
  
  typedef enum {
    FIND_CLOSEST,
    FIND_FIRST_BEFORE,
    FIND_FIRST_AFTER
  } hsrl_search_t;
  
  vector<string> _hsrlFieldNames;
  vector<string> _hsrlFieldLongNames;
  vector<string> _hsrlFieldUnits;

  int _checkParams();
  int _runFilelist();
  int _runArchive();
  int _runRealtime();

  int _processFile(const string &hcrPath);

  void _setupHcrRead(RadxFile &file);

  RadxRay *_findHsrlRay(RadxRay *hcrRay);
  void _setupHsrlRead(RadxFile &file);
  int _readHsrlVol(RadxTime &searchTime);

  void _mergeRay(RadxRay *hcrRay,
                 RadxRay *hsrlRay);
  
  void _addEmptyHsrlFieldsToRay(RadxRay *hcrRay);
  
  int _addFields(RadxVol &vol);
  
  int _addField(RadxVol &vol,
                const Params::field_t &fld);

  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol);

};

#endif
