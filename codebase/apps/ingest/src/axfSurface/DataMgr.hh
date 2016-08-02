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
////////////////////////////////////////////////////////////////////////////////
//
//  Ingest of axf surface observations file and output of spdb data
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  August 1999
//
//  $Id: DataMgr.hh,v 1.7 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <map>
#include <vector>
#include <fstream>
#include <utility>
#include <euclid/geometry.h>
#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>
using namespace std;

//
// Forward class declarations
//
class Params;
class StationReport;
class StationDB;


class DataMgr
{
public:
   DataMgr();
  ~DataMgr();

   int              init( Params &params );
   int              ingest();

private:

   Path             inputFile;
   ifstream         inputStream;

   int              processFile();
   int              readData();
   int              writeObservations();
   void             clearObservations();
   void             clearStations();

   //
   // Fields to be read from the Axf Surface Observation file
   //
   pair< size_t, size_t > STATION_NAME;
   pair< size_t, size_t > STATION_DESCRIP;
   pair< size_t, size_t > STATION_ID;
   pair< size_t, size_t > STATION_LAT;
   pair< size_t, size_t > STATION_LON;
   pair< size_t, size_t > STATION_ALT;
   pair< size_t, size_t > TIME_STAMP;
   pair< size_t, size_t > WIND_DIR;
   pair< size_t, size_t > WIND_SPEED;
   pair< size_t, size_t > WIND_GUST;
   pair< size_t, size_t > TEMPERATURE;
   pair< size_t, size_t > DEW_POINT;
   pair< size_t, size_t > REL_HUMIDITY;
   pair< size_t, size_t > PRESSURE;
   pair< size_t, size_t > VISIBILITY;

   static const double MISSING_VALUE;
   static const char*  INDEX_FILENAME;

   //
   // Data region limits for ingest
   //
   bool     doClipping;
   Point_d *clippingArea;
   int      numVerticies;

   //
   // StationReport database
   //
   StationDB                             *database;
   vector< StationReport* >               observations;
   map< int, StationReport*, less<int> >  stations;
   int                                    numDestinations;

   //
   // Time management
   //
   DateTime         dataTime;
   DateTime         statTime;
   bool             useIndexFile;
   bool             setTimeNow;
   LdataInfo       *ldataIndex;

};

#endif
