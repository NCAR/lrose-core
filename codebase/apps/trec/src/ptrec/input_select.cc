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
 * input_select.c
 * 
 * Selection of input files based on the syntax:  HHMMSS.suffix
 *
 * Terri L. Betancourt
 * 
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *                
 *************************************************************************/
#include "trec.h"

int input_select( const struct dirent *dirInfo )
{
   long hour, minute, second;
   const char *charPtr;

   //
   // Throw out hidden files
   //
   if ( dirInfo->d_name[0] == '.' )
      return 0;

   //
   // Check for the correct file suffix
   //
   charPtr = strchr( dirInfo->d_name, '.' );
   if ( charPtr == NULL ) 
      return 0;
   charPtr++;
   if ( strncmp( charPtr, Glob->params.input_file_suffix, 
                 strlen(Glob->params.input_file_suffix) )) 
      return 0;
   charPtr += strlen(Glob->params.input_file_suffix);
   if ( !strcmp( charPtr, ".Z" )  ||  !strcmp( charPtr, ".gz" ) ) {
      printf( "Please, uncompress files before running PTREC in archive mode.\n" );
      tidy_and_exit( -1 ); 
   }
      
   if ( *charPtr != '\0' )
      return 0;

   //
   // Check for the expected format HHMMSS in the filename
   //
   if ( sscanf( dirInfo->d_name, "%2ld%2ld%2ld", &hour, &minute, &second) != 3 )
      return 0;
   if ( hour > 23 || minute > 59 || second > 59 ) 
      return 0;

   //
   // ok, this one looks good
   //
   return 1;
}
