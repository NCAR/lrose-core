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
//////////////////////////////////////////////////////////
// RangeTable - class that reads in a file which contains
//              a table of elevations associated with a 
//              maximum range; this class will then provide
//              the maximum range given an elevation value
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
//
// $Id: RangeTable.cc,v 1.8 2016/03/07 01:23:10 dixon Exp $
//
/////////////////////////////////////////////////////////
#include <stdio.h>
#include "RangeTable.hh"
#include "Driver.hh"

//
// Constants
//
const int RangeTable::MAX_LINE_LEN = 40;

RangeTable::RangeTable() 
{
   maxElevation = 0.0;
}

RangeTable::~RangeTable() 
{
   rangeLookup.erase( rangeLookup.begin(), rangeLookup.end() );
}

Status::info_t RangeTable::setup( char* filePath ) 
{
   //
   // Open the config file
   //
   FILE *fp = fopen( filePath, "r" );
  
   if ( !fp ) {
      POSTMSG( ERROR, "Could not open %s\n", filePath );
      return( Status::FAILURE );
   }
  
   //
   // Read through the file
   //   Lines with 2 characters indicate the end of the file
   //   Ignore lines beginning with #
   //
   char *tmpStr = new char[MAX_LINE_LEN];
  
   while ( NULL != fgets( tmpStr, MAX_LINE_LEN, fp ) && strlen(tmpStr) > 2 ) {

      char* elevStr  = strtok( tmpStr, " " );
      char* rangeStr = strtok( NULL, " " );

      if( elevStr[0] == '#' ) {
         continue;
      }

      float elevation = 0.0;
      float range     = 0.0;
      
      if( sscanf( elevStr, "%f", &elevation ) != 1 ) {
         POSTMSG( ERROR, "Could not read file %s\n", filePath );
         fclose( fp );
         return( Status::FAILURE );
      }
      
      if( sscanf( rangeStr, "%f", &range ) != 1 ) {
         POSTMSG( ERROR, "Could not read file %s\n", filePath );
         fclose( fp );
         return( Status::FAILURE );
      }

      rangeLookup[elevation] = range;

      if( elevation > maxElevation ) {
         maxElevation = elevation;
      }
   }

   delete[] tmpStr;

   return( Status::ALL_OK );
}

float RangeTable::getRange( float elevation ) 
{
   if ( elevation < 0 ) {
      return( 0 );
   }

   //
   // Round the elevation to the nearest .1 degree
   //
   float nearestElev = ((float) ((int) ( elevation * 10 + 0.5 ))) / 10.0;

   //
   // If we've exceeded the elevations listed in the table, just
   // use the last one
   //
   if( nearestElev > maxElevation ) {
      nearestElev = maxElevation;
   }
   
   //
   // Find the range 
   //  NOTE: We are assuming that the table contains every 0.1
   //  degrees of elevation from 0 to the max!!!
   //
   return( rangeLookup[nearestElev] );
   
}

   

      
