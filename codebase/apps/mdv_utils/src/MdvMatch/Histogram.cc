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
//  $Id: Histogram.cc,v 1.4 2016/03/04 02:22:11 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <float.h>
#include <math.h>
#include <iomanip>

#include "Histogram.hh"

//
// This class could be moved into a library for re-use 
// by removing the following include file and
// replacing the POSTMSG call below with a different error handling mechanism
// 
#include "Driver.hh"
using namespace std;

Histogram::Histogram()
{
   numValues   = 0;
   numValid    = 0;
   minOutliers = 0;
   maxOutliers = 0;
}

int
Histogram::init( float dynamicMin, float dynamicMax, float dynamicInterval )
{
   int      i;
   float    ticValue;

   minValue = dynamicMin;
   maxValue = dynamicMax;
   interval = dynamicInterval;

   //
   // Make sure the interval is an even integral of the dynamic range
   //
   size_t numBins  = (size_t)(((maxValue - minValue) / interval) + 0.5);
   double dblBins = (double)((maxValue - minValue) / interval);
   if ( numBins - fabs( dblBins ) > 0.001 ) {
      POSTMSG( ERROR, "Interval %.2f is not an intergral of the dynamic range "
                      "[%.2f, %.2f].", interval, minValue, maxValue );
      return( -1 );
   }

   //
   // Initialize the x-axis
   //
   ticValue = minValue;
   size_t numTics = numBins + 1;

   for( i=0; i < (int) numTics; i++ ) {
      xaxis.push_back( ticValue );
      ticValue += interval;
   }

   //
   // Initialize the histogram and pdf
   //
   for( i=0; i < (int) numBins; i++ ) {
      histogram.push_back( 0 );
      pdf.push_back( 0 );
   }

   //
   // Initialize the cdf
   //
   for( i=0; i <= 100; i++ ) {
      cdf.push_back( 0 );
   }

   return( 0 );
}

void
Histogram::clear()
{
   int i;

   numValues   = 0;
   numValid    = 0;
   minOutliers = 0;
   maxOutliers = 0;

   assert( histogram.size() == pdf.size() &&
           histogram.size() == xaxis.size()-1 );

   for( i = 0; i < (int) histogram.size(); i++ ) {
      histogram[i] = 0;
      pdf[i] = 0;
   } 
   for( i = 0; i < 100; i++ ) {
      cdf[i] = 0;
   } 
}

void
Histogram::build( const float* data, int dataLen,
                  float missingData, float badData )
{
   int     i, j;
   float   cumProb;
   float   x, x0, x1, xdiff, xincrement;
   size_t  y, y0, y1, ydiff;
   size_t  numInvalid = 0;
   float   dataValue;

   //
   // Clear out any old data
   //
   clear();
   numValues = dataLen;

   //
   // Examine each data value to see which bin it belongs in
   // for the histogram
   //
   for( i=0; i < dataLen; i++ ) {
      dataValue = data[i];
      if ( dataValue == missingData || dataValue == badData ) {
         //
         // Don't include missing or bad data in the histogram
         //
         numInvalid++;
         continue;
      }
      if ( dataValue < minValue ) {
         //
         // Don't include values below the minValue of the dynamic range
         //
         minOutliers++;
         continue;
      }
      if ( dataValue > maxValue ) {
         //
         // Don't include values above the maxValue of the dynamic range
         //
         maxOutliers++;
         continue;
      }
      for( j = 0; j < (int) histogram.size(); j++ ) {
         if ( dataValue <= xaxis[j+1] ) {
            histogram[j]++;
            numValid++;
            break;
         }
      }
   }
   assert( dataLen == (int) (numValid + numInvalid + minOutliers + maxOutliers) );

   //
   // Construct the probablility density function (PDF) from the histogram
   //
   for( i=0; i < (int) histogram.size(); i++ ) {
      pdf[i] = histogram[i] / (float)numValid;
   }

   //
   // Construct the cumulative distribution function (CDF) 
   // in increments of 1% from the PDF
   //
   x0      = xaxis[0];
   y0      = 0;
   cumProb = 0;

   for( i=0; i < (int) pdf.size(); i++ ) {
      x1 = xaxis[i+1];
      y1 = (size_t)(rint( (double)(cumProb + pdf[i])*100 ));

      ydiff = y1 - y0;
      xdiff = x1 - x0;
      if ( ydiff > 0 ) 
         xincrement = xdiff / ydiff;
      else
         xincrement = xdiff;

      //
      // Stop at the leading edge of a horizontal cdf segment
      // unless we're at the front of the cdf, i.e. cumProb is zero
      //
      if ( ydiff == 0  &&  cumProb > 0 ) {
         continue;
      }

      for( x=x0, y=y0; y <= y1; x+=xincrement,y++ ) {
         cdf[y] = x;
      }

      x0 = x1;
      y0 = y1;
      cumProb += pdf[i];
   }
}

void
Histogram::print()
{
   int    i;
   float  probability;

   cerr.setf( ios::right, ios::adjustfield );
   cerr.setf( ios::showpoint );
   cerr.precision( 4 );

   //
   // Print the histogram and PDF
   //
   cerr << setw(20) << "LowerLimit"
        << setw(20) << "UpperLimit"
        << setw(10) << "Count"
        << setw(10) << "PDF"
        << endl;

   for( i = 0; i < (int) histogram.size(); i++ ) {
      cerr << setw(20) << xaxis[i]
           << setw(20) << xaxis[i+1]
           << setw(10) << histogram[i]
           << setw(10) << pdf[i] 
           << endl;
   }
   
   //
   // Print the CDF
   //
   cerr << setw(30) << "Probability"
        << setw(15) << "Expected Value"
        << endl;

   for( i = 0; i < (int) cdf.size(); i++ ) {
      probability = i / 100.;
      cerr << setw(30) << probability
           << setw(15) << cdf[i]
           << endl;
   }
}
