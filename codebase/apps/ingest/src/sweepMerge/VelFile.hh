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
/////////////////////////////////////////////////////////////
// VelFile - handles processing of short range scan sweep
//           file without matching dbz data
//
// $Id: VelFile.hh,v 1.5 2016/03/07 01:23:11 dixon Exp $
//
/////////////////////////////////////////////////////////////
#ifndef _VEL_FILE_INC_
#define _VEL_FILE_INC_

#include "InputFile.hh"

//
// Forward class declarations
//
class DbzFile;
class FileInfo;

class VelFile : public InputFile
{
public:

   //
   // Constructor:
   //   fileInfo = container class with the following necessary
   //              information - volume number, sweep number, elevation, and
   //              current path name
   //   elevTol  = elevation tolerance - allowed difference when
   //              comparing to dbz file data
   //   timeTol  = time tolerance - allowed difference when comparing
   //              to dbz file data
   //
   VelFile( FileInfo& fileInfo, double elevTol, double timeTol );

   //
   // Destructor
   //
   ~VelFile(); 

   //
   // Initialize object
   //   azimuthNuame  = name of azimuth variable in file
   //   elevationName = name of elevation variable in file
   //   timeName      = name of time variable in file
   //   dbzName       = name of reflectivity variable in file
   //
   int init( string* azimuthName, string* elevationName, string* timeName,
             string* dbzName );
   
   //
   // Fills in the dbz data for this file using the long range
   // scan
   //   dbzFile = long range scan file
   //
   //   returns FAILURE on failure and SUCCESS on success
   //
   int createNewScan( DbzFile& dbzFile );
   
   //
   // Writes a copy of the original file to the backup path
   // and writes the file with the dbz data filled in to 
   // the output path
   //
   int write();

private:

   //
   // Tolerances 
   //   elevTolerance = elevation tolerance
   //   timeTolerance = time tolerance
   //
   //   If a ray from the reflectivity file exceeds either of these
   //   tolerances, it will not be used to fill in the reflectivity
   //   data in the velocity file
   //
   double elevTolerance;
   double timeTolerance;
   
   //
   // Array that holds the new reflectivity data for the velocity file
   //
   short *newDbz;

   //
   // Array that keeps track of the azimuths used from the reflectivity
   // file to fill out the reflectivity data in the velocity file
   //   This is not used at this time, but it may be used as a
   //   debugging tool, or added to the output file at some point
   //
   float *dbzAzUsed;
   
};

#endif
   



