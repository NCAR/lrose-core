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
// CmdVerify.hh
//
// CmdVerify object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2015
//
///////////////////////////////////////////////////////////////
//
// Reads CMD data from CfRadial files, containing (a) weather only,
// (b) clutter only, (c) merged. Verifies the performance of CMD against
// the known truthiness. Also writes data out to ASCII file in column
// format, for analysis by other apps.
//
////////////////////////////////////////////////////////////////

#ifndef CmdVerify_H
#define CmdVerify_H

#include "Args.hh"
#include "Params.hh"
#include <string>
class RadxVol;
class RadxFile;
class RadxRay;
class RadxField;
class DoradeRadxFile;
using namespace std;

class CmdVerify {
  
public:

  // constructor
  
  CmdVerify (int argc, char **argv);

  // destructor
  
  ~CmdVerify();

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
  
  int _nWarnCensorPrint;
  bool _labelsPrinted;

  int _runFilelist();
  int _runArchive();
  void _setupRead(RadxFile &file);
  int _processFile(const string &primaryPath);
  int _addCombinedFields(RadxVol &vol);
  int _addCombinedField(RadxVol &vol,
                        const Params::combined_field_t &comb);
  void _censorFields(RadxVol &vol);
  void _censorRay(RadxRay *ray);

  void _printVolume(RadxVol &vol);
  void _printRay(RadxRay &ray);

  bool _checkAllFieldsMissing(vector<RadxField *> &fields,
                              size_t gateNum);
  bool _checkAnyFieldsMissing(vector<RadxField *> &fields,
                              size_t gateNum);

};

#endif
