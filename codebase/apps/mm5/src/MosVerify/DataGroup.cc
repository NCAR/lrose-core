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
///////////////////////////////////////////////////////////////////
//
// DataGroup - container and methods for two arrays of the
//            same variable, each from different sources
//
// $Id: DataGroup.cc,v 1.3 2016/03/07 01:33:51 dixon Exp $
//
///////////////////////////////////////////////////////////////////
#include <toolsa/DateTime.hh>

#include "DataGroup.hh"
#include "MosVerify.hh"
using namespace std;

//
// Constants
//
const double DataGroup::MISSING_VAL = -999.0;

DataGroup::DataGroup( const string& baseName )
{
   name          = baseName;
}

DataGroup::~DataGroup() 
{
   clear();
}

void
DataGroup::clear() 
{
   x.erase( x.begin(), x.end() );
   y.erase( y.begin(), y.end() );
   z.erase( z.begin(), z.end() );
}


void
DataGroup::setPoint( double yVal, double xVal, double zVal ) 
{
   y.push_back( yVal );
   x.push_back( xVal );
   z.push_back( zVal );
}

int
DataGroup::writeVars( const string& outputFile )
{
   //
   // If we don't have any data, do nothing
   //
   if( x.size() < 1 || y.size() < 1 || z.size() < 1 ) {
      return( SUCCESS );
   }
   
   //
   // Open the file
   //
   FILE *fp = fopen( outputFile.c_str(), "a" );
   if ( fp == NULL ) {
      POSTMSG( ERROR, "Could not open file %s", outputFile.c_str() );
      return( FAILURE );
   }

   //
   // Write the data
   //
   for( int i = 0; i < (int) y.size(); i++ ) {
      fprintf( fp, "%11.7g\t%11.7g\t%11.7g\n", y[i], x[i], z[i] );
   }

   fclose( fp );
   
   return( SUCCESS );
}


