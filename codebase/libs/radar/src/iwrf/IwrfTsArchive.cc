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
///////////////////////////////////////////////////////////////
// IwrfTsArchive.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Retrieves time series data from a file archive.
//
// March 2021
//
///////////////////////////////////////////////////////////////
//
// IwrfTsArchive holds an arry of IwrfTsPulse objects, along with
// the associated IwrfTsInfo objects.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <radar/IwrfTsArchive.hh>
using namespace std;

////////////////////////////////////////////////////
// Base class

IwrfTsArchive::IwrfTsArchive(IwrfDebug_t debug) :
        _debug(debug)
        
{
}

//////////////////////////////////////////////////////////////////
// destructor

IwrfTsArchive::~IwrfTsArchive()

{

}

//////////////////////////////////////////////////////////////////
// reset queue to start

void IwrfTsArchive::resetToStart()

{
}

//////////////////////////////////////////////////////////////////
// reset queue to end

void IwrfTsArchive::resetToEnd()

{
}

//////////////////////////////////////////////////////////////////
// get a beam of pulses, given the time, el and az
// returns empty vector on failure
// data in the beam must be used before any other operations
// are performed on the archive

vector<IwrfTsArchive::PulseEntry *>
  IwrfTsArchive::getBeam(DateTime &searchTime,
                         double searchElev,
                         double searchAz)
{
  
  vector<PulseEntry *> beamPulses;
  
  return beamPulses;
  
}

///////////////////////////////////////////////////////////////
// PulseEntry inner class
///////////////////////////////////////////////////////////////

// Constructor

IwrfTsArchive::PulseEntry::PulseEntry(IwrfTsPulse *pulse) :
        _pulse(pulse)

{

  _burst = NULL;
  
}  

// destructor

IwrfTsArchive::PulseEntry::~PulseEntry() 
{
  delete _pulse;
  if (_burst != NULL) {
    delete _burst;
  }
}

// compute clump geom

// void IwrfTsArchive::PulseEntry::computeGeom() 
// {

// }

