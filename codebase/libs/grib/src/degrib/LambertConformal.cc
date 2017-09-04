// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1992 - 2010 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2010/10/7 23:12:29 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////
// LambertConformal - Lambert Conformal Projection
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
// 
///////////////////////////////////////////////////
#include <grib/LambertConformal.hh>
using namespace std;

LambertConformal::LambertConformal( ui08 *gdsPtr )
                 :GribSection()
{
   //
   // Initialize flags
   //
   directionIncsGiven = false;
   earthSpherical     = true;
   earthOblate        = false;
   uvRelToGrid        = false;
   northPoleOnProj    = true;
   southPoleOnProj    = false;
   oneProjCenter      = true;
   bipolarProj        = false;
   scanInPlusIDir     = true;
   scanInPlusJDir     = false;
   ijOrder            = true;
   
   //
   // The number of bytes used for this section
   // should always be the same
   //
   nBytes = 34;
   
   //
   // Numbers of rows and columns respectively
   //
   nx = upkUnsigned2( gdsPtr[6], gdsPtr[7] );
   ny = upkUnsigned2( gdsPtr[8], gdsPtr[9] );
   
   //
   // Latitude and Longitude of first grid point
   //
   lat1 = upkSigned3( gdsPtr[10], gdsPtr[11], gdsPtr[12] );
   lon1 = upkSigned3( gdsPtr[13], gdsPtr[14], gdsPtr[15] );

   // 
   // Flags
   //
   if( gdsPtr[16] & 128 )
      directionIncsGiven = true;

   if( gdsPtr[16] & 64 ) {
      earthSpherical = false;
      earthOblate    = true;
   }

   if( gdsPtr[16] & 8 ) 
      uvRelToGrid = true;
   
   //
   // The east longitude of the meridian parallel to the
   // y axis (or columns of grid) - See Grib documentation
   //
   gridOrientation = upkSigned3( gdsPtr[17], gdsPtr[18], gdsPtr[19]);

   //
   // x and y direction grid lengths - in meters
   //
   dx = upkSigned3( gdsPtr[20], gdsPtr[21], gdsPtr[22] );
   dy = upkSigned3( gdsPtr[23], gdsPtr[24], gdsPtr[25] );

   // 
   // Flags
   //
   if( gdsPtr[26] & 128 ) {
      northPoleOnProj = false;
      southPoleOnProj = true;
   }
   
   if( gdsPtr[26] & 64 ) {
      oneProjCenter = false;
      bipolarProj   = true;
   } 
   
   if( gdsPtr[27] & 128 ) 
      scanInPlusIDir = false;
   
   if( gdsPtr[27] & 64 )
      scanInPlusJDir = true;

   if( gdsPtr[27] & 32 )
      ijOrder = false;

   //
   // "The first latitude from the pole at which the secant cone
   // cuts the spherical earth", The WMO Format for the Storage
   // And the Exchange of Weather Product Messages in Gridded
   // Binary Form as Used by NCEP Central Operations, Clifford
   // H. Dey, March 10, 1998, Sec. 2, pp. 10.
   // 
   latin1 = upkSigned3( gdsPtr[28], gdsPtr[29], gdsPtr[30] );
   latin2 = upkSigned3( gdsPtr[31], gdsPtr[32], gdsPtr[33] );
   
   //
   // Latitude and Longitude of southern pole
   //
   latSouthernPole = upkSigned3( gdsPtr[34], gdsPtr[35], gdsPtr[36] );
   lonSouthernPole = upkSigned3( gdsPtr[37], gdsPtr[38], gdsPtr[39] );
   
}

   
   
      
   
   
   
   
   
   
   
   
   
   

