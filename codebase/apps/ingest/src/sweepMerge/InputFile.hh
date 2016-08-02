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
// InputFile - base class which handles processing of
//             various types of input files
//
// $Id: InputFile.hh,v 1.5 2016/03/07 01:23:11 dixon Exp $
//
///////////////////////////////////////////////////////
#ifndef _INPUT_FILE_INC_
#define _INPUT_FILE_INC_

#include <string>
#include <netcdf.hh>

//
// Forward class declarations
//
class FileInfo;

class InputFile 
{
public:

   //
   // Constructor:
   //   fileInfo = container class with the following necessary
   //              information - volume number, sweep number, elevation, and
   //              current path name
   //
   InputFile( FileInfo& fileInformation );

   //
   // Destructor
   //
   virtual ~InputFile();

   //
   // Set up the object
   //   azimuthName   = name of azimuth variable in the file
   //   elevationName = name of the elevation variable in the file
   //   timeName      = name of the time variable in the file
   //   dbzName       = name of the dbz variable in the file
   //
   //   returns FAILURE on failure and SUCCESS on success
   //
   virtual int init( string* azimuthName, string* elevationName, 
                     string* timeName, string* reflName ) = 0;

   //
   // Return info about this input file
   //
   virtual int     getNumCells(){ return numCells; }
   virtual int     getNumRays(){ return numRays; }

   virtual float*  getElevationData(){ return elevationData; }
   virtual float*  getAzimuthData(){ return azimuthData; }
   virtual short*  getDbzData(){ return dbzData; }
   virtual double* getTimeData(){ return timeData; }
   virtual string* getPath(){ return currentPath; }

   //
   // Constants
   //
   static const string MISSING_VAL_NAME;
   static const double DELTA_AZIMUTH;

protected:

   //
   // Input file information
   //   This class does NOT own the memory associated
   //   with this object - keeps a reference only
   //
   FileInfo &fileInfo;

   //
   // Netcdf file object for input file
   //   This class owns the memory associated with this
   //   object
   //
   NcFile *ncInput;

   //
   // Information about this file
   //   volNum    = volume number
   //   sweepNum  = sweep number
   //   elevation = elevation
   //   numRays   = number of rays in file
   //   numCells  = number of cells in each ray in file
   //
   int   volNum;
   int   sweepNum;
   float elevation;
   int   numRays;
   int   numCells;

   //
   // Data retrieved from file
   //   dbzData       = reflectivity data
   //   elevationData = elevation data
   //   azimuthData   = azimuth data
   //   timeData      = time data
   //
   //   This class owns the memory associated with these
   //   arrays
   //   
   short  *dbzData;
   float  *elevationData;
   float  *azimuthData;
   double *timeData;

   //
   // Missing value for dbz data
   //
   short dbzMissingVal;

   //
   // Path for current input file
   //   This class does NOT own the memory associated
   //   with this pointer.  The memory is owned by
   //   the fileInfo object referenced above.  The
   //   pointer here is just for convenience in
   //   the methods in the class and subclasses.
   //
   string *currentPath;

   //
   // Name for dbz data in file
   //   This class owns the memory associated with this
   //   pointer
   //
   string *dbzName;
   
   //
   // Process the file - get the information we need out of it
   //
   //   azimuthName   = name of azimuth variable in the file
   //   elevationName = name of the elevation variable in the file
   //   timeName      = name of the time variable in the file
   //   reflName      = name of the reflectivity variable in the file
   //
   //   returns FAILURE on failure and SUCCESS on success
   //
   int processFile( string* azimuthName, string* elevationName, 
                    string* timeName, string* reflName );

private:

};

#endif
