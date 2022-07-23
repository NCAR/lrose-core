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
/////////////////////////////////////////////////////////////////////////////////
//
//  Ingest of class file and output of spdb data
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 1998
//
//  $Id: DataMgr.hh,v 1.7 2016/03/07 01:23:11 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <map>
#include <toolsa/file_io.h>
#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <rapformats/Sndg.hh>
#include <Spdb/StationLoc.hh>
#include <Spdb/DsSpdb.hh>

//
// Forward class declarations
//
class Params;


class DataMgr
{
public:
   DataMgr();
  ~DataMgr();

   int              init( Params &params );
   int              ingest();

   char*            getFileSuffix(){ return fileSuffix; }

private:

  Sndg::header_t header;

  Sndg sndg;

   //
   // Data ingest
   //

   Path             inputFile;
   char            *fileSuffix;
   string           indexFileName;
   FILE            *fp;
   char             sondeId[64];
   double           lat;
   double           lon;
  DateTime         start_time;
  DateTime         end_time;
  bool             useStartEndTimes;

  //
  // Station Id locator info.
  //
  string            station_loc_url;
  float             max_station_dist_km;

   int              processFile();
   int              readHeader();
   char*            getHeaderLine( const char* label, char fieldText[] );
   int              findColumns();
   int              findFirstData();
   int              readData();

   //
   // Sounding data
   //
   static const char*  INDEX_FILENAME;
   static const char*  DELIMETER;
   static const double MISSING_VALUE;
   static const int    MAX_POINTS = 8192;

   int                 numPoints;

   //
   // Fields to be read out of the CLASS file
   //
   static const unsigned int NFIELDS_IN;  
  
   static const char*  TIME_LABEL; 
   static const char*  PRESSURE_LABEL;
   static const char*  TEMPERATURE_LABEL;
   static const char*  DEWPT_LABEL;
   static const char*  REL_HUM_LABEL;
   static const char*  U_WIND_LABEL;
   static const char*  V_WIND_LABEL;
   static const char*  WIND_SPEED_LABEL;
   static const char*  WIND_DIR_LABEL;
   static const char*  ASCENSION_RATE_LABEL;
   static const char*  LONGITUDE_LABEL;
   static const char*  LATITUDE_LABEL;
   static const char*  HEIGHT_LABEL;
   static const char*  PRESSURE_QC;
   static const char*  TEMP_QC;
   static const char*  HUMIDITY_QC;
   static const char*  U_WIND_QC;
   static const char*  V_WIND_QC;
   static const char*  ASCENSION_RATE_QC;

   double  time[MAX_POINTS];
   double  pressure[MAX_POINTS];
   double  temperature[MAX_POINTS];
   double  dewpt[MAX_POINTS];
   double  relHum[MAX_POINTS];
   double  uwind[MAX_POINTS];
   double  vwind[MAX_POINTS];
   double  windSpeed[MAX_POINTS];
   double  windDir[MAX_POINTS];
   double  ascensionRate[MAX_POINTS];
   double  longitude[MAX_POINTS];
   double  latitude[MAX_POINTS];
   double  height[MAX_POINTS];
   double  pressureQC[MAX_POINTS];
   double  tempQC[MAX_POINTS];
   double  humidityQC[MAX_POINTS];
   double  uwindQC[MAX_POINTS];
   double  vwindQC[MAX_POINTS];
   double  ascensionRateQC[MAX_POINTS];

   map<int, double*, less<int> > columnData;

   int writeSounding();
   string url;

   //
   // Time management
   //
   DateTime         dataTime;
   time_t           expireSecs;
   LdataInfo       *ldataIndex;
  
   //
   // File indexing
   //
   bool             useIndexFile;
   string           previousFileName;

};

#endif



