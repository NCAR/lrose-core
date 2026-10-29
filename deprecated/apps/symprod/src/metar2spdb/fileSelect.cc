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
//
//

#include <toolsa/port.h>
#include "Metar2Spdb.hh"
using namespace std;

int fileSelect( const struct dirent *dirInfo )
{
   int nParts;
   long year, month, day, hour, minute;

   //
   // Throw out bad file names
   //
   if( dirInfo->d_name == NULL)
      return 0;

   //
   // Throw out hidden files
   //
   if ( dirInfo->d_name[0] == '.' )
      return 0;

   //
   // Check for the correct file prefix
   //
   if ( strncmp( dirInfo->d_name, FILE_PRFX, strlen(FILE_PRFX)) )
	return 0;
  
   //
   // Check for the expected format YYYYMMDDHH in the filename
   //
   nParts = sscanf( dirInfo->d_name + strlen(FILE_PRFX) + 1, 
                    "%4ld%2ld%2ld%2ld%2ld", 
		    &year, &month, &day, &hour, &minute );
   if( nParts < 4 )
      return 0;
   if( month > 12 || day > 31 || hour > 23 ) 
      return 0;
   if( nParts == 5 && minute > 59 )
      return 0;

   //
   // ok, this one looks good
   //
   return 1;
}
