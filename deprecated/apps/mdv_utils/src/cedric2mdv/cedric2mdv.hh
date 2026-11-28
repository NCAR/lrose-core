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
// cedric2mdv.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////
//
// cedric2mdv converts Cedric-format files to MDV.
//
////////////////////////////////////////////////////////////////

#ifndef cedric2mdv_hh
#define cedric2mdv_hh

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"
class Cedric;
using namespace std;

////////////////////////
// This class

class cedric2mdv {
  
public:

  // constructor

  cedric2mdv (int argc, char **argv);

  // destructor
  
  ~cedric2mdv();

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
  DsInputPath *_input;
  static const fl32 _missingFl32;
  
  double _latSign;
  double _lonSign;

  int _processFile(const char *input_path);
  void _initMasterHeader(DsMdvx &mdvx, const Cedric &ced);
  void _addField(DsMdvx &mdvx,
                 const Cedric &ced,
                 const std::string &name,
                 const std::string &units,
                 const fl32 *data);
  string _guessUnits(const std::string &fieldName);
  int _writeOutput(DsMdvx &mdvx);

};

#endif

