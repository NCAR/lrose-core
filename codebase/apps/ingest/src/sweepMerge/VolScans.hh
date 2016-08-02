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
// VolScans - container class for dbz and vel files that
//            that need to be combined
//
// $Id: VolScans.hh,v 1.7 2016/03/07 01:23:11 dixon Exp $
//
//////////////////////////////////////////////////////////
#ifndef _VOL_SCANS_INC_
#define _VOL_SCANS_INC_

#include <string>
#include <map>

//
// Forward class declarations
//
class DbzFile;
class VelFile;
class NcFile;
class FileInfo;

class VolScans 
{
public:

   //
   // Constructor
   //   vNum = volume number
   //   azName = name of azimuth variable in input files
   //   elevName = name of elevation variable in input files
   //   tmName   = name of time variable in input files
   //   reflName = name of reflectivity variable in input files
   //
   //   Note that this class does not own the memory associated with
   //   azName, elevName, tmName or reflName
   //
   VolScans( int vNum, string* azName, string* elevName, string* tmName,
             string* reflName );

   //
   // Destructor
   //
   ~VolScans();
   
   //
   // Add a dbz (or reflectivity only file) to the list of
   // files for this volume
   //   fileInfo = object which contains information about
   //              this file, including the file path
   //   azTol    = azimuth tolerance
   //
   int addDbzFile( FileInfo& fileInfo, double azTol );

   //
   // Add a vel (or velocity, spectrum width only file) to the list
   // of files for this volume
   //   fileInfo = object which contains information about
   //              this file, including the file path
   //   elevTol  = elevation tolerance
   //   timeTol  = time tolerance
   //
   int addVelFile( FileInfo& fileInfo, double elevTol, double timeTol );

   //
   // Fill in the reflectivity data for each velocity file using
   // the reflectivity data in the corresponding reflectivity file
   //
   int createNewScan();
   
private:

   //
   // Names of variables in the input files
   //   This class does NOT own the memory associated with
   //   these pointers - not copying the memory allows us
   //   to avoid having many copies of the same thing, since
   //   there could be many VolScan objects in the application.
   //
   string *azimuthName;
   string *elevationName;
   string *timeName;
   string *dbzName;
   
   //
   // Current volume number
   //
   int volNum;
   
   //
   // Maps of dbz files and vel files.  The key in the
   // maps is the elevation.  This allows them to be
   // paired up.  Only the elevations we are interested
   // in combining will be in these maps.
   //
   map< float, DbzFile* > dbzFiles;
   map< float, VelFile* > velFiles;

};

#endif
   
