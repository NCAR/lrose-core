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
// Projection calculations from Mike Dixon's libs/mdv/mdv_proj.c
// 
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// February 1999
//
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>
#include <toolsa/toolsa_macros.h>
#include <euclid/Projection.hh>
#include <euclid/ProjRUC2Lambert.hh>
using namespace std;

//
// static constants
//
const double ProjRUC2Lambert::LAT_ORIGIN1 = 25.0;
const double ProjRUC2Lambert::LAT_ORIGIN2 = 25.0;
const double ProjRUC2Lambert::LON_ORIGIN  = -95.0;
const double ProjRUC2Lambert::LAT_LLC     = 16.281000;
const double ProjRUC2Lambert::LON_LLC     = -126.138000;
const double ProjRUC2Lambert::X_ORIGIN    = 83.000000;
const double ProjRUC2Lambert::Y_ORIGIN    = 15.500000;

ProjRUC2Lambert::ProjRUC2Lambert( Projection *parent )
         :ProjType( parent )
{
}

void
ProjRUC2Lambert::updateOrigin()
{
}

void
ProjRUC2Lambert::latlon2xy( double lat, double lon, double *xKm, double *yKm )
{
  cerr << "ERROR: Attempting to use ProjRUC2Lambert::latlon2xy(...)." << endl;
}

void
ProjRUC2Lambert::xy2latlon( double xKm, double yKm, double *lat, double *lon )
{
  cerr << "ERROR: Attempting to use ProjRUC2Lambert::xy2latlon(...)." << endl;
}

