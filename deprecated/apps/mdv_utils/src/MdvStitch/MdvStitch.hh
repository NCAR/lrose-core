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
// MdvStitch object
//
// Jason Craig, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////

#ifndef MdvStitch_HH
#define MdvStitch_HH

#include <string>
#include <vector>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

#include "Args.hh"
#include "Params.hh"
#include "Trigger.hh"
#include "InputFile.hh"

using namespace std;

/////////////////////////
// Forward declarations

class MdvStitch {
  
public:

  // constructor

  MdvStitch (int argc, char **argv);

  // destructor
  
  ~MdvStitch();

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
  vector<InputFile *> _inputs;

  // example master and field headers

  Mdvx::master_header_t _exampleMhdr;
  vector<Mdvx::field_header_t> _exampleFhdrs;

  // gridded fields

  int _nxyz;
  int _nxy;
  vector<void *> _merged;
  vector<ui08 *> _count;
  
  // Methods
  
  void _initOutputProj();
  void _createTrigger();

};

#endif
