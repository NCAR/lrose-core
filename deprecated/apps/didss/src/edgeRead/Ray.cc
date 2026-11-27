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
//////////////////////////////////////////////////////////////////
// Ray class
/////////////////////////////////////////////////////////////////

#include <math.h>
#include <toolsa/MsgLog.hh>

#include "Ray.hh"
#include "EdgeRead.hh"
using namespace std;

Ray::Ray() 
{
   numPts   = 0;
   nAlloc   = 0;
   byteData = NULL;
}

Ray::~Ray() 
{
   if( byteData )
      delete[] byteData;
}

int
Ray::readMsg( char* buffer, int npts )
{
   char *bufferPtr = buffer;
   
   //
   // Data
   //
   numPts = npts;
   if( numPts > nAlloc ) {
      if( byteData )
        delete[] byteData;

      byteData = new ui08[numPts];
      nAlloc   = numPts;
   } else {
      memset( (void *) byteData, (int) 0, nAlloc );
   }

   memcpy( (void *) byteData, (void *) bufferPtr, numPts * sizeof(ui08) );
   fprintf( stderr, "numPts = %d\n", numPts );
   for( int i = 0; i < numPts; i++ ) {
      fprintf( stderr, "%d  ", byteData[i]);
   }
   fprintf( stderr, "end\n" );
   
   return( SUCCESS );
   
}


