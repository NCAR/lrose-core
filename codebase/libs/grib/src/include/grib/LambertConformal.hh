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
///////////////////////////////////////////////////
// LambertConformal - Lambert Conformal Projection
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
// 
//////////////////////////////////////////////////
#ifndef _LAMBERT_CONF_
#define _LAMBERT_CONF_

#include "GribSection.hh"
using namespace std;

class LambertConformal : public GribSection {
public:

   LambertConformal( ui08 *gdsPtr );
   ~LambertConformal(){};

   inline int  getNx(){ return( nx ); }
   inline int  getNy(){ return( ny ); }
   inline bool isOrderIJ(){ return( ijOrder ); }
   inline bool increasingI(){ return( scanInPlusIDir ); }
   inline bool increadingJ(){ return( scanInPlusJDir ); }
   inline int  nRows(){ return( ijOrder ? ny : nx ); }
   
private:

   int nx;
   int ny;
   int lat1, lon1;                             // millidegrees
   
   bool directionIncsGiven;
   bool earthSpherical;
   bool earthOblate;
   bool uvRelToGrid;
   
   int  gridOrientation;                       // millidegrees
   int  dx, dy;                                // meters                
   
   bool northPoleOnProj;
   bool southPoleOnProj;
   bool oneProjCenter;
   bool bipolarProj;

   bool scanInPlusIDir;
   bool scanInPlusJDir;
   bool ijOrder;
   
   int latin1, latin2;                       // millidegrees
   int latSouthernPole, lonSouthernPole;     // millidegrees
 
};

#endif

   
