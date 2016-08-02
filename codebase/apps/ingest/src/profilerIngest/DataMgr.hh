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
//  Ingest of class file and output of spdb data
//
//  $Id: DataMgr.hh,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <map>
#include <toolsa/file_io.h>
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

   char*            getFileSuffix(){ return fileSuffix; }

private:

   //
   // Data ingest
   //
   Path             inputFile;
   char            *fileSuffix;
   FILE            *fp;
   SoundingPut      sounding;
   char             sondeId[64];
   double           lat;
   double           lon;

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
   static const int    MAX_POINTS = 1024;

   int                 numPoints;

   //
   // Fields to be read out of the CLASS file
   //
   static const unsigned int NFIELDS_IN;

   static const char*        HEIGHT_LABEL;
   static const char*        U_WIND_LABEL;
   static const char*        V_WIND_LABEL;
   static const char*        PRESSURE_LABEL;
   static const char*        REL_HUM_LABEL;
   static const char*        TEMPERATURE_LABEL;

   double                    height[MAX_POINTS];
   double                    uwind[MAX_POINTS];
   double                    vwind[MAX_POINTS];
   double                    pressure[MAX_POINTS];
   double                    relHum[MAX_POINTS];
   double                    temperature[MAX_POINTS];

   map<int, double*, less<int> > columnData;

   int writeSounding();

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
