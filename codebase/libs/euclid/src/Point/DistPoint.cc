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

#include <euclid/DistPoint.hh>
#include <euclid/Polyline.hh>
using namespace std;

DistPoint::DistPoint( vector< float >& vals, 
                      float x, float y, float z,
                      Polyline *poly )
{
   xDist     = x;
   yDist     = y;
   zDist     = z;
   refPoly   = poly;

   vector< float >::iterator it;
   for( it = vals.begin(); it != vals.end(); it++ ) {
      values.push_back( *it );
   }
   
}

DistPoint::DistPoint( float x, float y, float z,
                      Polyline *poly )
{
   xDist     = x;
   yDist     = y;
   zDist     = z;
   refPoly   = poly;
}

DistPoint::DistPoint( const DistPoint& pt ) 
{
   copy( pt );
}

DistPoint::DistPoint()
{
   xDist     = 0.0;
   yDist     = 0.0;
   zDist     = 0.0;
   refPoly   = NULL;
}

DistPoint::~DistPoint() 
{
   values.erase( values.begin(), values.end() );
}

void
DistPoint::copy( const DistPoint& pt )
{
   xDist     = pt.xDist;
   yDist     = pt.yDist;
   zDist     = pt.zDist;
   refPoly   = pt.refPoly;

   int nVals = pt.values.size();
   for( int i = 0; i < nVals; i++ ) {
      values.push_back( (float) pt.values[i] );
   }
   
}



