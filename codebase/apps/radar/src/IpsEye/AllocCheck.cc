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
//////////////////////////////////////////////////////////////////////////
// AllocCheck.cc: helper objects for moments computations
//
// EOL, NCAR, Boulder CO
//
// April 2010
//
// Mike Dixon
//
//////////////////////////////////////////////////////////////////////////
//
// This is implemented as a singleton.
//
//////////////////////////////////////////////////////////////////////////


#include "AllocCheck.hh"
#include <cstdio>
#include <iostream>

using namespace std;

// Global variables - instance

AllocCheck *AllocCheck::_instance = NULL;

//////////////////////////////////////////////////////////////////////////
// Constructor - private, called by inst()

AllocCheck::AllocCheck()
{
  _params = NULL;
  _nAlloc = 0;
  _nFree = 0;
  _prevWasFree = false;
}

//////////////////////////////////////////////////////////////////////////
// Destructor

AllocCheck::~AllocCheck()
{

}

//////////////////////////////////////////////////////////////////////////
// Inst() - Retrieve the singleton instance of this class.

AllocCheck &AllocCheck::inst()
{

  if (_instance == (AllocCheck *)NULL) {
    _instance = new AllocCheck();
  }

  return *_instance;
}

//////////////////////////////////////////////////////////////////////////
// add to stats

void AllocCheck::addAlloc()        
{

  _nAlloc++;
  doPrint(_prevWasFree);
  _prevWasFree = false;

}

void AllocCheck::addFree()        
{

  _nFree++;
  doPrint(false);
  _prevWasFree = true;

}

//////////////////////////////////////////////////////////////////////////
// do the print if needed

void AllocCheck::doPrint(bool force)        
{
  
  if (_params != NULL &&
      _params->check_ray_alloc == FALSE) {
    return;
  }

  if (force || ((_nAlloc + _nFree) % 500 == 0)) {
    if (force) {
      cerr << "=================================" << endl;
    }
    cerr << "====>>>> Ray allocation check: ";
    cerr << " nAlloc: " << _nAlloc;
    cerr << " nFree: " << _nFree;
    cerr << " delta: " << _nAlloc - _nFree;
    cerr << endl;
  }

}

