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
/////////////////////////////////////////////////////////
// Ingester
//
////////////////////////////////////////////////////////

#include <string>
#include <map>
#include <vector>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class Metar;
class MetarLocation;

class Ingester {
 public:

  Ingester(char *inputDir, const Params &params);
   ~Ingester();
   
   int    setLocations(char* locationFileName);
   int    setTime(char* fileName);
   void    setTime(time_t fileTime);

  int processFile(char* fileName, 
		  vector< Metar* >& metars,
		  int ExtendFilename = 1);
  
  int storeMetar(char* fileName, 
		 vector< Metar* >& metars,
		 const string &metarMessage);
  
   int    getYear(){ return fileYear; }
   int    getMonth(){ return fileMonth; }
   int    getDay(){ return fileDay; }
   int    getHour(){ return fileHour; }
   int    getMin(){ return fileMin; }
   int    getBlockHour(){ return blockHour; }
   int    getBlockMin(){ return blockMin; }
   
 private:

   static const int FILE_NAME_LENGTH = 256;
   static const int MAX_LINE = 256;
   static const float MISSING_ALT = -999.0;
   static const float FT_TO_M = 0.3048;

   const Params &_params;

   int    fileYear, fileMonth, fileDay, fileHour, fileMin, fileSec;
   int    blockHour, blockMin;

   char   *inputPath;

   map< string, MetarLocation*, less<string> > locations;

   bool startOfSABlock(char *line);
   
};
