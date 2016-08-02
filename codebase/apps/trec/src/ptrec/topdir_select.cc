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
/****************************************************************************
 * topdir_select.c
 * 
 * Selection of top directory entries based on the syntax:  YYYYMMDD
 *
 * Terri L. Betancourt
 * 
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *                
 *************************************************************************/
#include "trec.h"

int topdir_select( const struct dirent *dirInfo )
{
   long year, month, day;

   //
   // Throw out hidden files
   //
   if ( dirInfo->d_name[0] == '.' )
      return 0;

   //
   // Check for the expected format YYYYMMDD in the directory entry
   //
   if ( sscanf( dirInfo->d_name, "%4ld%2ld%2ld", &year, &month, &day) != 3 )
      return 0;
   if ( month > 12 || day > 31 ) 
      return 0;

   //
   // ok, this one looks good
   //
   return 1;
}
