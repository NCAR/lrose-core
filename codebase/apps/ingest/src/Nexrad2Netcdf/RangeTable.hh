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
// $Id: RangeTable.hh,v 1.3 2016/03/07 01:23:03 dixon Exp $
//
/////////////////////////////////////////////////////////
#ifndef _RANGE_TABLE_
#define _RANGE_TABLE_

#include <map>

#include "Nexrad2Netcdf.hh"

using namespace std;

class RangeTable 
{
public:

   //
   // Constructor
   //
   RangeTable();
   
   //
   // Destructor
   //
   ~RangeTable();

   //
   // Set up the table
   //   filePath = path on disk for file that contains table
   //
   //   See Status.hh for meaning of return value
   //
   status_t setup( char* filePath );
   
   //
   // Get the range for the given elevation
   //   elevation = elevation in degrees
   //
   float getRange( float elevation );

   //
   // Constants
   //   MAX_LINE_LEN = maximum length of line in file
   //
   static const int MAX_LINE_LEN;
   
private:

   //
   // Maximum elevation provided in table
   //
   float maxElevation;

   //
   // Table with elevation as the key and range
   // as the element
   //
   map< float, float, less<float> > rangeLookup;
};

#endif
   

   
   
   
