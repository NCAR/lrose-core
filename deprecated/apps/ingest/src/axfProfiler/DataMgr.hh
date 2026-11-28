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
//  Ingest of axf profiler data and output of spdb data
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2000
//
//  $Id: DataMgr.hh,v 1.6 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <vector>
#include <iostream>
#include <utility>
#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <Spdb/SoundingPut.hh>
using namespace std;

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

private:

   Path             inputFile;
   ifstream         inputStream;
   SoundingPut      sounding;
   size_t           numPoints;

   int              processFile();
   int              readData();
   void             clearData();

   //
   // Fields to be read from the Axf Sonde file
   //
   pair< size_t, size_t > HEIGHT;
   pair< size_t, size_t > WIND_DIR;
   pair< size_t, size_t > WIND_SPEED;
   pair< size_t, size_t > WIND_W;
   pair< size_t, size_t > QUAL_HORIZ;
   pair< size_t, size_t > QUAL_VERT;
   pair< size_t, size_t > TIME_STAMP;
   pair< size_t, size_t > STATION_ID;
   pair< size_t, size_t > STATION_NAME;

   //
   // BOM has specified varying missing values
   // depending on the data field from the profiler
   //
   static const double WIND_DIR_MISSING;
   static const double WIND_SPEED_MISSING;
   static const double WIND_W_MISSING;

   //
   // We use only one missing value in the spdb database
   //
   static const double SPDB_MISSING_VALUE;

   static const char*  INDEX_FILENAME;

   //
   // Fields to be written to the sounding database
   //
   vector<double>            height;
   vector<double>            uwind;
   vector<double>            vwind;
   vector<double>            wwind;
   vector<double>            pressure;

   int writeSounding();

   //
   // Time management
   //
   DateTime         dataTime;
   DateTime         statTime;
   time_t           expireSecs;
   bool             setTimeNow;
   bool             useIndexFile;
   LdataInfo       *ldataIndex;

};

#endif
