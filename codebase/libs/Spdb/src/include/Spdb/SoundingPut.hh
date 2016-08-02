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
//
/**
 * @file SoundingPut.hh
 * @brief Writes sounding data to spdb
 * @class SoundingPut
 * @brief Writes sounding data to spdb
 */
//  
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
// $Id: SoundingPut.hh,v 1.13 2016/03/03 18:13:05 dixon Exp $
//
///////////////////////////////////////////////
#ifndef _SOUNDING_PUT_HH_
#define _SOUNDING_PUT_HH_

#include <Spdb/Sounding.hh>
using namespace std;

class SoundingPut : public Sounding
{
public:

   SoundingPut(){};
  ~SoundingPut(){};

   void init( const string &url, source_t sourceId, const char *sourceFmt);

   void init( vector< string* >& urls, source_t sourceId,
              const char *sourceFmt, 
              int siteId = 0, const char *siteName = NULL,
              double lat = 0.0, double lon = 0.0, 
              double altOfSite = 0.0, double missingVal = DBL_MAX );

  void init( const string& urlStr, 
             source_t sourceIdx, const char *sourceFmtx,
             int siteIdx, const char *siteNamex,
             double lat, double lon, double altOfSite,
             double missingVal );

   //
   // Set sounding data from arrays of doubles
   //
   int set( time_t launch, 
            int numPts, 
            double *height, 
            double *u, 
            double *v,
	    double *w = NULL, 
            double *prs = NULL,
            double *relHum = NULL, 
            double *temperature = NULL,
            double *divergence = NULL );

   //
   // Set sounding data from vectors of doubles
   //
   int set( time_t launch,
            vector<double> *height,
            vector<double> *u, 
            vector<double> *v,
            vector<double> *w = NULL, 
            vector<double> *prs = NULL,
            vector<double> *relHum = NULL,
            vector<double> *temperature = NULL,
            vector<double> *divergence = NULL );

   //
   // Set descriptive info
   // SiteId and SiteName are arbitrary and set by the client
   // SiteId is used as the spdb data_type
   //
   void     setSiteId( int id ){ siteId = id; }
   void     setSiteName( const char* name );
   void     setSiteName( const string& name ){ siteName = name; }

   //
   // SourceId is limited to the types defined above by source_t
   // SourceName is derived from sourceId
   //
   void     setSourceId( source_t id ){ sourceId = id; }

   //
   // Set Location
   //
   void     setLocation( double lat, double lon, double alt ) 
                       { siteLat = lat; siteLon = lon, siteAlt = alt; }

   //
   // Write sounding to spdb
   // Returns 0 upon success, -1 on failure
   //
   int      writeSounding( time_t validTime, time_t expireTime, 
                           int leadSecs = 0,
                           Spdb::put_mode_t putMode = Spdb::putModeOver,
						   int data_type2 = 0 );

private:

   void setProduct( SNDG_spdb_product_t *productPtr );
};

#endif
