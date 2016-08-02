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
/////////////////////////////////////////////////
// Metar
//
/////////////////////////////////////////////////
  
#include <string>
#include <map>
#include <rapformats/station_reports.h>
#include <rapformats/metar.h>
#include <toolsa/utim.h>
#include "Params.hh"
using namespace std;

//
// Forward class declarations
//
class MetarLocation;

class Metar {
 public:

   Metar(const Params &params,
	 int file_year, int file_month, int file_day, int file_hour, int file_min,
	 int block_hour, int block_min);
   ~Metar();
  
   int setMetar(const char* message, char* fileName,
                map< string, MetarLocation*, less<string> >& locations);

   int getHashId() { return (hashId); }
   const string &getRawText() { return (rawText); }
   
   const station_report_t &getStationReport(){ return stationReport; }

   void printStationReport();

 private:

   static const int SECS_IN_24_HRS = 86400;
   static const int NOBSTRUCT = 10;
   static const int NOBSCURE = 2;

  const Params &_params;
   Decoded_METAR    dcdMetar;
   station_report_t stationReport;
   int hashId;

   string rawText;
   string stationId;
  
   float lat;
   float lon;
   float alt;

   int fileYear;
   int fileMonth;
   int fileDay;
   int fileHour;
   int fileMin;
   int blockHour;
   int blockMin;

   int fillStationReport();


};

   

