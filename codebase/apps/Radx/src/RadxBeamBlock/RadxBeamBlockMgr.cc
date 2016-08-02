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

/**
 * @file RadxBeamBlockMgr.cc
 */
#include <toolsa/umisc.h>
#include "RadxBeamBlockMgr.hh"
#include "RadxBeamBlock.hh"

//------------------------------------------------------------------
RadxBeamBlockMgr::RadxBeamBlockMgr(int argc, char **argv)
{
  _alg = NULL;
  _params = NULL;
  isOK = false;
  _progName = "RadxBeamBlock";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    return;
  }

  // get TDRP params
  _paramsPath = (char *) "unknown";
  Params p;
  if (p.loadFromArgs(argc, argv, _args.override.list,
		     &_paramsPath))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
  }

  _params = new Parms(p, _progName);

  // use params to create the algorithm
  _alg = new RadxBeamBlock(*_params);
  isOK = true;
}

//------------------------------------------------------------------
RadxBeamBlockMgr::~RadxBeamBlockMgr(void)
{
  if (_alg != NULL)
  {
    delete _alg;
  }
  if (_params != NULL)
  {
    delete _params;
  }
}

//------------------------------------------------------------------
int RadxBeamBlockMgr::Run(void)
{
  if (!isOK)
  {
    return 1;
  }
  if (_alg->Run() == 1)
  {
    return 1;
  }
  return _alg->Write();
}

