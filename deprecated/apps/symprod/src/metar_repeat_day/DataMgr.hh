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
////////////////////////////////////////////////
// Data Manager
//
// $Id: DataMgr.hh,v 1.4 2016/03/06 23:31:57 dixon Exp $
//
////////////////////////////////////////////////

#include <string>
#include <vector>
#include <dirent.h>
#include <toolsa/utim.h>

#include "metar_repeat_day_tdrp.h"

class DataMgr {
 public:
   
   DataMgr();
   ~DataMgr();
   
   int init(metar_repeat_day_tdrp_struct& params);
   int processFiles();

   //
   // Static members
   //
   static const int MAX_LINE;
   static const int MAX_FILE_PATH;

 private:
   
   UTIMstruct      currentTime;
   int             dayLast;

   char           *inputDir;
   char           *outputDir;

   int             nFiles;
   int             fileIndex;
   struct dirent **fileList;

   int             outputHour, outputMin;

   vector< pair< string*, string* >* > ids;

   int             fileTime( const char* fileName, 
			     int* hour, int* sec );
   int             translateFile( const char* fileName, 
                                  int hour, int min );
   
};
