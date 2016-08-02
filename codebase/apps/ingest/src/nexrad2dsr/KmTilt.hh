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
//  Class for managing a tilt of 1km data indexed by azimuth
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: KmTilt.hh,v 1.6 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _KM_TILT_INC_
#define _KM_TILT_INC_

#include "Status.hh"
#include "KmData.hh"
using namespace std;


class KmTilt
{
public:
   KmTilt(){ clearState(); }
  ~KmTilt(){};

   //
   // State management
   //
   void    clearState();
   void    startOfDbz( size_t numGates ){ clearState(); dataLen = numGates; }
   void    startOfVel(){ completeLut(); }

   //
   // Saving the data at a specified azimuth
   // Although you can fetch dbz and snr separately, the two must be
   // saved together at a single azimuth.
   //
   void    setData( float azimuth, ui08* dbzData, ui08* snrData );

   //
   // Access methods
   //
   enum type_t { DBZ_DATA = 2,
                 SNR_DATA
               };

   ui08*   getDbz( float azimuth )
                 { return( getData( DBZ_DATA, azimuth )); }

   ui08*   getSnr( float azimuth )
                 { return( getData( SNR_DATA, azimuth )); }

   size_t  numGates(){ return( dataLen ); }

   float   getStoredAzimuth( float azimuth )
                 { return( kmData[getDataIndex( azimuth )].getAzimuth()); }

   //
   // Do we have any 1km data available?
   //
   bool    isAvailable(){ return( dataAvailable ); }

private:

   //
   // Static constants -- initialize them here so we can use them right away
   //
   static const int     UNAVAILABLE   = -1;
   static const size_t  NUM_INDICIES  = 3600 + 1;
   static const size_t  NUM_AZIMUTHS  = 480;

   //
   // List of data indicies stored by synthetic azimuth index
   //
   short           dataLut[NUM_INDICIES];

   //
   // Current data index for storage into the KmData arrays
   // This is the index that gets saved in the dataLut 
   // at the synthetic azimuth index
   //
   size_t          dataIndex;

   //
   // The lookup table is cleared out at the start of reflectivity tilts
   // and indicies are completed at the start of velocity tilts
   //
   void            clearLut();
   void            completeLut();

   //
   // 1km data at each azimuth
   //
   KmData          kmData[NUM_AZIMUTHS];
   size_t          dataLen;
   bool            dataAvailable;

   ui08*           getData( type_t type, float azimuth );

   //
   // Returns synthetic azimuth index (0-3600)
   //
   size_t          getAzIndex( float azimuth );

   //
   // Returns index into KmData arrays or UNAVAILABLE if azimuth is not found
   //
   int             getDataIndex( float azimuth );
};

#endif
