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
////////////////////////////////////////////////////////////////////////////////
//
//  Class containing scan type information
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: ScanType.hh,v 1.5 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _SCAN_TYPE_INC_
#define _SCAN_TYPE_INC_

#include <string>
#include <vector>

#include "Params.hh"
#include "Status.hh"
#include <cassert>
using namespace std;


class ScanType
{
public:
   ScanType( int scanId, string scanName, float pulseWidth, 
             size_t numElev, float* elevList );
  ~ScanType();

   //
   // Access methods to scan type information
   //
   const int       getId() const { return( id ); }
   const string&   getName() const { return( name ); }
   float           getPulseWidth() const { return( pulseWidth ); }

   //
   // Nexrad elevation numbers are not 0-based, so we have to adjust the index
   //
   float           getElev( size_t elevNum ) const 
                          { assert( elevNum <= elevations.size() );
                            return( elevations[elevNum-1] ); }

private:

   int             id;
   string          name;
   float           pulseWidth;
   vector<float>   elevations;
};

#endif
