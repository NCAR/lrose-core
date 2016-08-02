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
#ifndef _MDV_FIELD_INC_
#define _MDV_FIELD_INC_

#include <sys/time.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_user.h>
#include <Mdv/mdv/mdv_macros.h>
#include <didss/DsURL.hh>
#include <toolsa/DateTime.hh>
using namespace std;

//
// forward class declarations
//
class Grid;

class MdvField
{
public:
   MdvField();
   MdvField( const string &name, const Grid &g );
   MdvField( const MdvField &source );
  ~MdvField();

   enum CompositeType { MDVP_COMPOSITE_NONE = -1, MDVP_COMPOSITE_MAX = 1 };
   
   enum PlaneHeightType { 

      MDVP_VERT_TYPE_SURFACE         = 1,
      MDVP_VERT_TYPE_SIGMA_P         = 2,
      MDVP_VERT_TYPE_PRESSURE        = 3,
      MDVP_VERT_TYPE_Z               = 4,
      MDVP_VERT_TYPE_SIGMA_Z         = 5,
      MDVP_VERT_TYPE_ETA             = 6,
      MDVP_VERT_TYPE_THETA           = 7,
      MDVP_VERT_TYPE_MIXED           = 8,
      MDVP_VERT_TYPE_ELEV            = 9,
      MDVP_VERT_TYPE_COMPOSITE       = 10,
      MDVP_VERT_TYPE_CROSS_SEC       = 11,
      MDVP_VERT_TYPE_SATELLITE_IMAGE = 12,
      MDVP_VERT_TYPE_VARIABLE_ELEV   = 13
   };

   //
   // Copying 
   //
   MdvField& operator= ( const MdvField &source );
   void copy( const MdvField &source );

   //
   // Get the header info and muck with it yo'self
   //
   MDV_field_header_t&    getInfo(){ return info; }

   // Sets the field header struct.
   //   Be sure to call updateGeometry() after putting in the appropriate grid!
   //  
   void setInfo( const MDV_field_header_t & header );

   //
   // Setting info
   //
   void    setGeometry( size_t nx, size_t ny, size_t nz,
                        float dx, float dy, float dz,
                        float minx, float miny, float minz,
                        double lat, double lon, double rotation );
   void    updateGeometry();

   void    setDescription( const char *name, const char *desc=NULL,
                           const char *units=NULL, const char *transform=NULL );

   void    setOffset( int offset )
                    { info.field_data_offset = (si32)offset; }
   void                   setSourceFieldNum(int num) { sourceFieldNum = num; }
   inline void            setSourceURL(const DsURL * URL);


   // Special methods for Using MdvField to specify a server request.
   // 
   void    setBoundingBox(double newMinLat, double newMinLon,
                          double newMaxLat, double newMaxLon);
   void    getBoundingBox(double & outMinLat, double & outMinLon,
                          double & outMaxLat, double & outMaxLon);
   bool    isBoundingBoxRequest() { return _isBoundingBoxRequest; }
   void    clearBoundingBox();

   void    setSearchZRequest(bool isSearchZ) { _isSearchZRequest = isSearchZ; }
   bool    isSearchZRequest()                { return _isSearchZRequest; }

   void    setPlaneNum(const size_t & n)     { planeNum = n; }
   size_t  getPlaneNum()                     { return planeNum; }

   // Need to send whole field header to specify query geometry?
   bool hasSpecialQueryGeometry() const;
   bool areGeometriesEqual(const MdvField & other) const;

   // int     setFromMsg( const DsURL & url, int fieldNum, const DateTime & time,
   //                     const MDV_field_header_t & header, 
   //                     CompositeType compositeType,
   //                     int dataLen, const unsigned char * data,
   //                     const MDV_vlevel_header_t * vHeader);

   //
   // Field identification
   //
   const char*     getName(){ return info.field_name; }
   void            setName( const char* newName )
                          { setDescription( newName ); }
   void            setName( const string& newName )
                          { setDescription( newName.c_str() ); }

   //
   // Field data timestamp
   //
   time_t           getTime() const        { return dataTime.utime(); }
   const DateTime&  getDateTime() const    { return dataTime; }

   void             setTime( time_t when ) { dataTime.set(when); }
   void             setDateTime( const DateTime & when ) { dataTime = when; }

   //
   // Fetching info
   //
   Grid*           getGrid() const { return grid; }
   int             getSourceFieldNum() const { return sourceFieldNum; }
   const DsURL *   getSourceURL() const      { return sourceURL; }
   const char*     getName() const           { return info.field_name; }
   const char*     getNameLong() const       { return info.field_name_long; }
   const char*     getUnits() const          { return info.units; }
   const char*     getTransform() const      { return info.transform; }
   int             getDataSize() const;

   //
   // Field data scaling
   //
   unsigned char*  getScaledData();
   void *          getUnscaledData();
   void            forceIntegralScaling(){ integralScaling = true;}

private:
   int                    sourceFieldNum;
   DsURL                 *sourceURL;
   MDV_field_header_t     info;
   Grid                  *grid;
   unsigned char         *data;
   DateTime               dataTime;
   bool                   integralScaling;

   // Bounding box requests -- ignore the grid's x-y geometry.
   bool                   _isBoundingBoxRequest;
   double                 minLat;
   double                 minLon;
   double                 maxLat;
   double                 maxLon;

   // Allow the server to be flexible about what vertical levels are returned.
   bool                   _isSearchZRequest;

   // Request a single plane with the specified index.
   size_t                 planeNum;

   void                   initHdr();
   void                   setData( const Grid &g );
   void                   calcVolumeSize();
};

inline void MdvField::setSourceURL(const DsURL * URL)
{
   if (sourceURL != NULL) {
       delete sourceURL;
       sourceURL = NULL;
   }

   if (URL != NULL) {
       sourceURL = new DsURL(*URL);
   }
}

#endif
