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
///////////////////////////////////////////////////////////
// FileInfo - container class for information retrieved
//            from file name
//////////////////////////////////////////////////////////
#include "FileInfo.hh"
#include "SweepMerge.hh"

//
// Constants
//
const int FileInfo::CMD_STR_LEN   = 2000;
const int FileInfo::PATH_NAME_LEN = 500;

FileInfo::FileInfo( const char* inputDir, char* fname, int volNum, 
                    int swpNum, float elev, string& backupDir )
{
   fileName   = new string( fname );
   volumeNum  = volNum;
   sweepNum   = swpNum;
   elevation  = elev;

   char backupStr[PATH_NAME_LEN];
   sprintf( backupStr, "%s/%s", backupDir.c_str(), fileName->c_str() );
   backupPath = new string( backupStr );

   char pathStr[PATH_NAME_LEN];
   sprintf( pathStr, "%s/%s", inputDir, fname );
   fullPath = new string( pathStr );

}

FileInfo::~FileInfo() 
{
   delete fileName;
   delete fullPath;
}

int FileInfo::doBackup() 
{
   char cmdStr[CMD_STR_LEN];

   //
   // Copy the file to the backup area
   //
   sprintf( cmdStr, "cp %s %s", fullPath->c_str(), backupPath->c_str() );

   if( system( cmdStr ) != 0 ) {
      POSTMSG( ERROR, "Could not copy file %s to backup area", 
               fileName->c_str() );
      return( FAILURE );
   }

   return( SUCCESS );
}


