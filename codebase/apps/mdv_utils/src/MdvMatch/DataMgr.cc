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
//  Working class for MdvMatch application
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2000
//
//  $Id: DataMgr.cc,v 1.4 2016/03/04 02:22:11 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iomanip>
#include <Mdv/MdvxField.hh>

#include "Driver.hh"
#include "DataMgr.hh"
#include "Params.hh"
using namespace std;

int
DataMgr::init( Params &params )
{
   if ( histogramA.init( params.dynamic_range_A.min, 
                         params.dynamic_range_A.max, 
                         params.dynamic_range_A.interval ) != 0 ) {
      return( -1 );
   }

   if ( histogramB.init( params.dynamic_range_B.min, 
                         params.dynamic_range_B.max, 
                         params.dynamic_range_B.interval ) != 0 ) {
      return( -1 );
   }

   return( 0 );
}

int
DataMgr::probabilityMatch( const MdvxField& fieldA, const MdvxField& fieldB )
{

   float  probability;

   //
   // Build histograms for both input datasets
   //
   Mdvx::field_header_t fieldHdrA = fieldA.getFieldHeader();

   histogramA.build( (float *)fieldA.getVol(), 
                     fieldA.getVolLen() / sizeof(float),
                     fieldHdrA.missing_data_value,
                     fieldHdrA.bad_data_value );

   Mdvx::field_header_t fieldHdrB = fieldB.getFieldHeader();

   histogramB.build( (float *)fieldB.getVol(), 
                     fieldB.getVolLen() / sizeof(float),
                     fieldHdrB.missing_data_value,
                     fieldHdrB.bad_data_value );

   if ( DEBUG_ENABLED ) {
      POSTMSG( DEBUG, "Histogram for input dataset A" );
      histogramA.print();

      POSTMSG( DEBUG, "Histogram for input dataset B" );
      histogramB.print();
   }

   //
   // Print out the matching statistics
   //
   const vector< float > & cdfA = histogramA.getCdf();
   const vector< float > & cdfB = histogramB.getCdf();
   assert( cdfA.size() == cdfB.size() );

   cerr.setf( ios::right, ios::adjustfield );
   cerr.setf( ios::showpoint );
   cerr.precision( 4 );

   cerr << setw(20) << "           " 
        << setw(20) << "Expected Value"
        << setw(20) << "Expected Value"
        << endl
        << setw(20) << "Probability"
        << setw(20) << "  Dataset A   "
        << setw(20) << "  Dataset B   "
        << endl << endl;

   for(size_t i = 0; i < cdfA.size(); i++ ) {
      probability = i / 100.;
      cerr << setw(20) << probability
           << setw(20) << cdfA[i]
           << setw(20) << cdfB[i]
           << endl;
   }

   //
   // Print other various statistics
   //
   cerr << endl
        << setw(20) << "#Outliers < min:"
        << setw(20) << histogramA.getMinOutliers()
        << setw(20) << histogramB.getMinOutliers()
        << endl
        << setw(20) << "#Outliers > max:"
        << setw(20) << histogramA.getMaxOutliers()
        << setw(20) << histogramB.getMaxOutliers()
        << endl
        << setw(20) << "Percent Matched:"
        << setw(20) << histogramA.getPercentMatched()
        << setw(20) << histogramB.getPercentMatched()
        << endl << endl;

   return( 0 );
}
