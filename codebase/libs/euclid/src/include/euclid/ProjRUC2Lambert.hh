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
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA   
// February 1999
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _PROJ_RUC2LAMBERT_INC_
#define _PROJ_RUC2LAMBERT_INC_

#include <euclid/ProjType.hh>

//
// Forward class declaration
//
class Projection;

class ProjRUC2Lambert : public ProjType {

public:

   ProjRUC2Lambert( Projection *context );
  ~ProjRUC2Lambert(){};

   //
   // Required projection methods
   //
   void     updateOrigin();
   void     latlon2xy( double lat, double lon, double *xKm, double *yKm );
   void     xy2latlon( double xKm, double yKm, double *lat, double *lon );

private:

   static const double LAT_ORIGIN1;
   static const double LAT_ORIGIN2;
   static const double LON_ORIGIN;
   static const double LAT_LLC;
   static const double LON_LLC;
   static const double X_ORIGIN;
   static const double Y_ORIGIN;
};

#endif
