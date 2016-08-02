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
// RadxMergeFields.hh
//
// RadxMergeFields object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2010
//
///////////////////////////////////////////////////////////////
//
// Merges fields from multiple CfRadial files into a single file
//
////////////////////////////////////////////////////////////////

#ifndef RadxMergeFields_H
#define RadxMergeFields_H

#include "Args.hh"
#include "Params.hh"
#include <string>
class RadxVol;
class RadxFile;
class RadxRay;
class DoradeRadxFile;
using namespace std;

class RadxMergeFields {
  
public:

  // constructor
  
  RadxMergeFields (int argc, char **argv);

  // destructor
  
  ~RadxMergeFields();

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

  string _primaryDir;
  vector<string> _secondaryDirs;

  class OutputGroup {
  public:
    string dir;
    double fileTimeOffset;
    double fileTimeTolerance;
    double rayElevTolerance;
    double rayAzTolerance;
    double rayTimeTolerance;
    vector<Params::output_field_t> fields;
  };
  
  OutputGroup _primaryGroup;
  vector<OutputGroup> _secondaryGroups;
  OutputGroup _activeGroup;

  int _checkParams();
  int _runFilelist();
  int _runArchive();
  int _runRealtime();
  void _setupPrimaryRead(RadxFile &file);
  void _setupSecondaryRead(RadxFile &file,
                           const OutputGroup &group);
  void _setupWrite(RadxFile &file);
  int _writeVol(RadxVol &vol);
  int _processFile(const string &primaryPath);
  int _checkGeom(const RadxVol &primaryVol,
                 const RadxVol &secondaryVol);
  int _mergeVol(RadxVol &primaryVol,
                const RadxVol &secondaryVol,
                const OutputGroup &group);
  void _mergeRay(RadxRay &primaryRay,
                 const RadxRay &secondaryRay,
                 const OutputGroup &group);

  int _addCombinedFields(RadxVol &vol);
  int _addCombinedField(RadxVol &vol,
                        const Params::combined_field_t &comb);

};

#endif
