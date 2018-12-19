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
#ifndef _FILE_INFO_INC_
#define _FILE_INFO_INC_

#include <string>

using namespace std;

class FileInfo 
{
public:

   //
   // Constructor
   //   inputDir  = input directory path
   //   fname     = file name
   //   volNum    = volume number
   //   swpNum    = sweep number
   //   elev      = elevation
   //   backupDir = path to backup directory
   //
   FileInfo( const char* inputDir, char* fname, int volNum, int swpNum, 
             float elev, string& backupDir );

   //
   // Destructor
   //
   ~FileInfo();

   //
   // Backup the file to the backup directory
   //
   int doBackup();

   //
   // Return information from this object
   //
   string* getFilePath(){ return fullPath; }
   int     getVolumeNum(){ return volumeNum; }
   int     getSweepNum(){ return sweepNum; }
   float   getElevation(){ return elevation; }

   //
   // Constants
   //
   static const int CMD_STR_LEN;
   static const int PATH_NAME_LEN;
   
   
private:

   //
   // Current file name without the directory
   //  This class owns the memory associated with this pointer
   //
   string *fileName;

   //
   // Current file name with the full path
   //  This class owns the memory associated with this pointer
   //
   string *fullPath;

   //
   // Full path for backup name
   //  This class owns the memory associated with this pointer
   //
   string *backupPath;
   
   //
   // Information from the file name
   //   volumeNum = volume number
   //   sweepNum  = sweep number
   //   elevation = elevation
   //
   int   volumeNum;
   int   sweepNum;
   float elevation;
};

#endif
