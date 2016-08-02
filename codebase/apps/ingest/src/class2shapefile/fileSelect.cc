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
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 2003
//
// $Id: fileSelect.cc,v 1.4 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include <toolsa/Path.hh>
#include <toolsa/port.h>

#include "Driver.hh"
using namespace std;

int fileSelect( const struct dirent *dirInfo )
{
   //
   // Degenerate case
   //
   if( dirInfo->d_name == NULL )
      return 0;

   //
   // Throw out hidden files
   //
   if ( dirInfo->d_name[0] == '.' )
      return 0;

   //
   // Check for the expected format YYYYMMDDHHMMSS in the basename
   //
   for( int i=0; i < 14; i++ ) {
      if ( !isdigit( dirInfo->d_name[i] )) {
         return 0;
      }
   }

   //
   // Check for the optional suffix
   //
/*-------
   if ( strstr( dirInfo->d_name, 
                driver->getDataMgr().getFileSuffix()) == NULL ) {
        return 0;
   }
--------*/

   string suffix = driver->getDataMgr().getFileSuffix();
   if ( !ISEMPTY( suffix )) {
      Path fileName = dirInfo->d_name;
      if ( fileName.getExt() != suffix ) {
         return 0;
      }
   }

   //
   // ok, this one looks good
   //
   return 1;
}
