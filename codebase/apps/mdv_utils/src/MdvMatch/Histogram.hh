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
//  Histogram class
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2000
//
//  $Id: Histogram.hh,v 1.4 2016/03/04 02:22:11 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _HISTOGRAM_INC_
#define _HISTOGRAM_INC_

#include <vector>
#include <utility>
using namespace std;

class Histogram
{
public:
   Histogram();
  ~Histogram(){};

   int     init( float minValue, float maxValue, float interval );
   void    build( const float* data, int dataLen,
                  float missingData, float badData );
   void    print();

   float   getPercentMatched(){ return( (float)numValues / numValid ); }
   size_t  getMinOutliers(){ return( minOutliers ); }
   size_t  getMaxOutliers(){ return( maxOutliers ); }

   //
   // CDF is returned as a vector where:
   //    the vector's index [0-100] represents the y-axis as probability*100
   //    the vector element's value represents the x-axis, i.e., expected value
   //
   const vector< float > &  getCdf(){ return( cdf ); }

private:

   size_t           numValues;
   size_t           numValid;
   size_t           minOutliers;
   size_t           maxOutliers;

   float            minValue;
   float            maxValue;
   float            interval;

   vector< float >  xaxis;
   vector< size_t > histogram;
   vector< float >  pdf;
   vector< float >  cdf;

   void    clear();
};

#endif
