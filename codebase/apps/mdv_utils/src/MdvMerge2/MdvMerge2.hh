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
// MdvMerge2 object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
///////////////////////////////////////////////////////////////

#ifndef MdvMerge2_HH
#define MdvMerge2_HH

#include <string>
#include <vector>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>

#include "Args.hh"
#include "Params.hh"
#include "Trigger.hh"
#include "InputFile.hh"

using namespace std;

/////////////////////////
// Forward declarations

class MdvMerge2 {
  
public:

  // constructor

  MdvMerge2 (int argc, char **argv);

  // destructor
  
  ~MdvMerge2();

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
  MdvxProj _outProj;
  Trigger *_trigger;
  DsTrigger *_dataTrigger;
  vector<InputFile *> _inputs;

  // example master and field headers

  Mdvx::master_header_t _exampleMhdr;
  vector<Mdvx::field_header_t> _exampleFhdrs;

  // gridded fields

  int _nz, _nxy, _nxyz;
  vector<void *> _merged;
  vector<ui08 *> _count;
  vector<time_t *> _latestTime;
  fl32 *_closestRange;
  int *_closestFlag;
  
  // Methods

  int _runForecast();
  int _runObs();
  int _initGrids();
  int _createTrigger();
  void _initMerge();
  int _processData(time_t fileTime, int leadTime);

};

#endif
