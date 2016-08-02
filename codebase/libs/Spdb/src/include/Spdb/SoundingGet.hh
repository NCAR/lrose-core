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
 * @file  SoundingGet.hh
 * @brief sounding data from spdb
 * @class SoundingGet
 * @brief Reads sounding data from spdb
 */
//  
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1999
//
///////////////////////////////////////////////
#ifndef _SOUNDING_GET_HH_
#define _SOUNDING_GET_HH_

#include <Spdb/Sounding.hh>
using namespace std;


class SoundingGet : public Sounding
{
public:

   SoundingGet(){};
  ~SoundingGet();

   void init( const char* url, time_t margin = 0,
              double minWindsAlt = -1.0, double maxWindsAlt = -1.0,
              double avgU = 0.0, double avgV = 0.0 );

   //
   // Use defaults or not
   //
   void useDefaults(){ useSounding = false; }
   void useSndgData(){ useSounding = true; }

   // 
   // Read sounding from spdb
   // If siteId is 0, all locations are retained; 
   //    otherwise, only the specified location is retained
   // Returns -1 on failure, otherwise returns number of products retained
   //
   int  readSounding( time_t when, int whichSiteId = 0, int data_type2 = 0 );

   //
   // Load a product into the class
   //
   // NOTE: You must first call readSounding() to retrieve sounding data
   //       from the server.  This method just loads up
   //       the member data arrays to be in sync with the specified product.
   //       For convenience, the first product (index = 0) is always loaded 
   //       after a call to readSounding().
   //
   void  loadProduct( int productIndex );

   inline size_t getNumSoundings() const { return products.size(); }
  

   //
   // Get descriptive info regarding sounding
   //
   int            getSiteId() const { return siteId; }
   const string&  getSiteName() const { return siteName; }
   int            getSourceId() const { return sourceId; }
   double         getLat() const { return siteLat; }
   double         getLon() const { return siteLon; }
   double         getAlt() const { return siteAlt; }
   time_t         getLaunchTime() const { return launchTime; }

   //
   // Get sounding data
   //
   double *getPres() const ;
   double *getAlts() const ;
   double *getU() const ;
   double *getV() const ;
   double *getW() const ;
   double *getRH() const ;
   double *getTemp() const ;
   double *getDivergence() const ;
   double *getWindSpeed() const ;
   double *getWindDir() const ;

   //
   // Get derived data from the sounding
   // 
   int     getNumPoints() const { return( numObs ); }
   void    getUV( double *u, double *v, double speedRange = DBL_MAX ) const;
   void    getUV( double alt, double *uVal, double *vVal ) const;
   void    getDirSpeed( double alt, double *dir, double *speed ) const;
   void    getStats( field_t field, double *min, double *max, double *avg ) const;
   void*   getChunk() const;

private:

   //
   // Spdb management
   //
   string              url;
   time_t              timeMargin;
   int                 nInputChunks;
   Spdb::chunk_ref_t  *inputChunkHdrs;
   void               *inputChunkData;

   //
   // Altitude range for calculating averages
   //
   bool    userAltLimits;
   double  userMinAlt;
   double  userMaxAlt;
   double  currentMinAlt;
   double  currentMaxAlt;

   //
   // Default averages if not using sounding
   //
   bool    useSounding;
   double  avgU;
   double  avgV;

   //
   // Selected sounding products based on selection criteria in readSounding()
   //
   vector < SNDG_spdb_product_t * > products;
   int                            activeProduct;
   void  clearProducts();

   //
   // 
   // Fetch from the spdb database
   // Returns -1 on failure, otherwise the number of chunks read
   //
   int   fetchData( time_t when, int whichSiteId, int data_type2, bool* gotData );

   //
   // Find the index into the altitude array 
   // for a given altitude
   //
   int  getIndex( double alt ) const;

   //
   // Set the min and max altitude, if not already set
   //
   void setAltLimits();

};

#endif
