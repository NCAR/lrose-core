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
///////////////////////////////////////////////////////////////////////////////
// $Id: LatLon.cc,v 1.5 2016/03/06 23:53:41 dixon Exp $
//
// Keeps latitude and longitude in deg, min, and sec
///////////////////////////////////////////////////////////////////////////////

#include "LatLon.hh"
#include "Test2Edge.hh"
using namespace std;

LatLon::LatLon( int deg, int min, int sec ) 
{
   set( deg, min, sec );
}

LatLon::LatLon( double val ) 
{
   set( val );
}

void 
LatLon::set( double val ) 
{
   value = val;
   
   degrees = (int) value;
   minutes = (int) ((value-degrees)*60.0);
   seconds = (int) ((value-degrees-minutes/60.0)*360.0);
}

void
LatLon::set( int deg, int min, int sec )
{
   degrees = deg;
   minutes = min;
   seconds = sec;

   value   = deg + ((double) minutes)/60.0 + ((double) seconds)/360.0;
}

   
   
