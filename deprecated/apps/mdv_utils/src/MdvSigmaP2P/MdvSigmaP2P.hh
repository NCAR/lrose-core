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
// $Id: MdvSigmaP2P.hh,v 1.2 2016/03/04 02:22:12 dixon Exp $
//
// MdvSigmaP2P object
//
// Yan Chen, RAL, NCAR
//
// Sept. 2008
//
///////////////////////////////////////////////////////////////
//
// MdvSigmaP2P converts Mdv files in Sigma P levels to pressure levels.
//
///////////////////////////////////////////////////////////////

#ifndef MdvSigmaP2P_HH
#define MdvSigmaP2P_HH

#include <string>
#include <set>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class MdvSigmaP2P {
  
public:

  // constructor

  MdvSigmaP2P (int argc, char **argv);

  // destructor
  
  ~MdvSigmaP2P();

  // run 

  int Run();

  // data members

  bool isOK;

private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  date_time_t _startTime;
  date_time_t _endTime;

  set<string> _fieldSet;

  int _performConvert();
  int _convertField(DsMdvx&, MdvxField*, fl32*, bool);
};

#endif

