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
// This is an abstract base class for all projection types
// To add a new projection you should do the following...
//   - inherit from this class, providing the required null methods 
//   - add an entry to the enumeration Projection::ProjId in Projection.hh
//   - include the new projection header in Projection.cc
//   - add an entry to the switch statement in Projection::setProjectionType()
//         in Projection.cc
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA   
// February 1999
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _PROJ_TYPE_INC_
#define _PROJ_TYPE_INC_

//
// Foward class declaration
//
class Projection;

class ProjType {

public:

   ProjType( Projection *parent ){ projection = parent; }
   virtual ~ProjType(){};

   //
   // Required methods from all projection type subclasses
   //
   virtual void updateOrigin() = 0;
   virtual void latlon2xy( double lat, double lon, double *x, double *y ) = 0;
   virtual void xy2latlon( double x, double y, double *lat, double *lon ) = 0;

protected:

   Projection *projection;
};

#endif
