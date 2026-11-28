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
//////////////////////////////////////////////////////
// DataMgr - handles data i/o and processing
/////////////////////////////////////////////////////
#ifndef _DATA_MGR_INC
#define _DATA_MGR_INC

#include <string>
#include <vector>
#include <map>
#include "Params.hh"

//
// Forward class declarations
//
class FileInfo;
class VolScans;

class DataMgr 
{
public:

   //
   // Constructor
   //
   DataMgr();

   //
   // Destructor
   //
   ~DataMgr();
   
   //
   // Initialize
   //   params = tdrp parameter object
   //
   int init( Params& params );

   //
   // Process the files
   //
   int run();

   //
   // Constants
   //
   static const int SUFFIX_LEN;
   static const int RADAR_ID_LEN;
   
private:
  
   //
   // Tolerances
   //
   double azimuthTol;
   double elevationTol;
   double timeTol;

   //
   // Variable names in the files
   //  Copies made from parameter file
   //
   string *azimuthName;
   string *elevationName;
   string *timeName;
   string *dbzName;

   //
   // Path for backups
   //  Copy made from parameter file
   //
   string *backupPath;

   //
   // Elevations that we are going to process
   //
   vector< double > elevationList;

   //
   // Map of all the files that we are going to process
   // listed by sweep number
   //
   map< int, FileInfo*, less<int> > files;

   //
   // Map of all the volumes that we are going to process
   // listed by volume number - the VolScan objects contain
   // the information from the file list
   //
   map< int, VolScans*, less<int> > volumeScans;

   //
   // Creates the list of files above by scanning the directory
   //   inputDir = input directory
   //
   int createFileList( const char* inputDir );
   
   //
   // Pair up the files in a given volume with a given elevation.
   // One will be the dbz only scan and the other will be the
   // velocity and spectrum width only scan
   //
   int findPairs();

   //
   // Combines the file pairs above to fill in the dbz data in
   // the velocity file
   //
   int fillScan();
};

#endif
   
