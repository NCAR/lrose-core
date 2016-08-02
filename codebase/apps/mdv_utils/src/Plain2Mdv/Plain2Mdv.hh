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
// Plain2Mdv.hh
//
// Plain2Mdv object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// Plain2Mdv converts a plain binary array file to MDV.
// The data in the file must be a single field in an ordered binary
// array.
//
////////////////////////////////////////////////////////////////

#ifndef Plain2Mdv_hh
#define Plain2Mdv_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class Plain2Mdv {
  
public:

  // constructor

  Plain2Mdv (int argc, char **argv);

  // destructor
  
  ~Plain2Mdv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const fl32 _missingFloat;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  DsInputPath *_input;

  int _processFile(const char *input_path);
  int _computeDataTime(const char *input_path, time_t &data_time);
  void _loadFieldData(ui08 *inBuf,
                      int nx, int ny, int nz, int nxy, int nxyz,
                      fl32 scale, fl32 bias, fl32 miss,
		      fl32 minVal, fl32 maxVal,
                      fl32 *data);
  void _initMasterHeader(DsMdvx &mdvx, time_t dataTime);
  void _addField(DsMdvx &mdvx, int fieldNum, fl32 *data);
  int _writeOutput(DsMdvx &mdvx);

};

#endif

