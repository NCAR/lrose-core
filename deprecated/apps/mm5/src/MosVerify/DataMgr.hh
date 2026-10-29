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
//  Data management
//
//  $Id: DataMgr.hh,v 1.8 2016/03/07 01:33:51 dixon Exp $
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
class DataGroup;
class MetarData;
class MetarPoint;
class MM5Data;
class MM5Point;
class Params;

class DataMgr {
 public:

   enum VariableType { WSPD, WDIR, TEMP, PRS, RH, CEIL, VIS };
   
   DataMgr();
  ~DataMgr();

   int  init( Params& params, time_t sTime, time_t eTime );
   void processData();
   
   //
   // Constants
   //
   static const int    TMP_STR_LEN;
   static const int    HOURS_TO_SEC;

   static const string WDIR_NAME;
   static const string WSPD_NAME;
   static const string TEMP_NAME;
   static const string PRS_NAME;
   static const string RH_NAME;
   static const string CEIL_NAME;
   static const string VIS_NAME;
   
private:
   
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
   // ForecastTime information
   //
   vector< int >        forecastTimes;

   //
   // Data servers
   //
   DataServer           truthServer;
   DataServer           forecastServer;
   DataServer           modelServer;

   //
   // Data list objects
   //
   MetarData           *truthData;
   MetarData           *forecastData;
   MM5Data             *modelData;

   //
   // Statistics
   //
   map< VariableType, DataGroup*, less<VariableType> > dataGroups;

   int  timeTolerance;
   void clear();
   void compareForecast( char* stationId, int leadTime = 0 );
   int  blend( map< time_t, MetarPoint*, less<time_t> >& truthVariables,
               map< time_t, MetarPoint*, less<time_t> >& forecastVariables,
               map< time_t, MM5Point*, less<time_t> >& modelVariables );
   void setGroups( MetarPoint* truthPoint, MetarPoint* forecastPoint,
                   MM5Point* modelPoint );

   //
   // Output
   //
   string                outputDir;
   Path                  outputPath;
   void                  writeData( const string& stationId, 
                                    int leadTime = 0 );
   void                  setOutputPath( const string& stationId,
                                        int leadTime = 0 );
   
};

#endif


