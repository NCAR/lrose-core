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
// Helpers.cc: helper objects for moments computations
//
// RAP, NCAR, Boulder CO
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


#include "Helpers.hh"
#include <cstdio>
#include <iostream>

using namespace std;

// Global variables - instance

Helpers *Helpers::_instance = NULL;

//////////////////////////////////////////////////////////////////////////
// Constructor - private, called by inst()

Helpers::Helpers()
{
  _nAlloc = 0;
  _nFree = 0;
  _prevWasFree = false;
}

//////////////////////////////////////////////////////////////////////////
// Destructor

Helpers::~Helpers()
{

}

//////////////////////////////////////////////////////////////////////////
// Inst() - Retrieve the singleton instance of this class.

Helpers &Helpers::inst()
{

  if (_instance == (Helpers *)NULL) {
    _instance = new Helpers();
  }

  return *_instance;
}

//////////////////////////////////////////////////////////////////////////
// add to stats

void Helpers::addAlloc()        
{

  _nAlloc++;
  doPrint(_prevWasFree);
  _prevWasFree = false;

}

void Helpers::addFree()        
{

  _nFree++;
  doPrint(false);
  _prevWasFree = true;

}

//////////////////////////////////////////////////////////////////////////
// do the print if needed

void Helpers::doPrint(bool force)        
{

  if (force || ((_nAlloc + _nFree) % 200 == 0)) {
    if (force) {
      cerr << "=================================" << endl;
    }
    cerr << "====>>>> Ray allocation: ";
    cerr << " nAlloc: " << _nAlloc;
    cerr << " nFree: " << _nFree;
    cerr << " delta: " << _nAlloc - _nFree;
    cerr << endl;
  }

}

