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
/////////////////////////////////////////////////////////////////////////
//
//  Server and ingest data management
//
//  $Id: DataMgr.hh,v 1.32 2016/03/07 01:33:50 dixon Exp $
//
////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <map>
#include <vector>
#include <rapformats/GenPt.hh>
#include <toolsa/Path.hh>

#include "DataServer.hh"
using namespace std;

//
// Forward class declarations
//
class MetarData;
class MetarPoint;
class MM5Data;
class MM5Point;
class MultReg;
class Params;
class Regression;
class SimpleReg;

class DataMgr {
 public:

   enum VariableType { U, V, WSPD, TEMP, PRS, RH, CEIL, VIS };
   
   DataMgr();
  ~DataMgr();
   int  init( Params& params, time_t sTime, time_t eTime );
   int  processData();
   
   //
   // Constants
   //
   static const int    TMP_STR_LEN;
   static const int    HOURS_TO_SEC;
   
private:

   //
   // Forecast times
   //
   vector< int >        forecastTimes;
   
   //
   // Station ids
   //
   vector< char* >      stationIds;

   //
   // Data times
   //
   time_t               startTime;
   time_t               endTime;

   //
   // Thresholds
   //
   double               clwThresh;
   double               iceThresh;
   double               ceilingThresh;
   double               visThresh;

   //
   // Data servers
   //
   DataServer           mm5Server;
   DataServer           metarServer;
   DataServer           rrServer;

   //
   // Data list objects
   //
   MM5Data             *mm5Data;
   MetarData           *metarData;

   //
   // Blend the data
   //
   time_t   timeTolerance;
   int      blend( map< time_t, MM5Point*, less<time_t> >& modelVariable,
                   map< time_t, MetarPoint*, less<time_t> >& metarVariable );

   //
   // Statistics
   //
   map< VariableType, Regression*, less<VariableType> > regressions;

   void  clear();
   int   setRegressions( MM5Point* mm5Point, MetarPoint* metarPoint );
  void  setSimple( double mm5Val, double metarVal, SimpleReg& reg, time_t metarTime, time_t modelTime, int modelLeadSecs);
   int   setMultiple( vector< double >& mm5Vals, double metarVal, 
                      MultReg& reg, time_t metarTime, time_t modeTime, int modelLeadSecs);

   //
   // Output
   //
   int                   maxValidTime;
   string                outputUrl;
   DsSpdb                spdbMgr;
   GenPt                 outputPt;
   bool                  writeInfo;
   int                   writeData( const string& stationId, 
                                    double lat, double lon,
                                    time_t dataTime, int leadTime );
   
   Path                  regOutputPath;
   string                regOutputDir;

   Path                  coeffOutputPath;
   string                coeffOutputDir;

   Path                  corrOutputPath;
   string                corrOutputDir;

   Path                  varOutputPath;
   string                varOutputDir;

   void                  writeTextData( int forecastTime, char* stationId );
   void                  setOutputPath( string& outputDir, char* stationId, 
                                        int forecastTime, Path& outputPath );
   
};

#endif


