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
//////////////////////////////////////////////////////////
// $Id: EdgeMsg.hh,v 1.14 2016/03/06 23:53:42 dixon Exp $
//
// Edge Message class
/////////////////////////////////////////////////////////
#ifndef _EDGE_MSG_
#define _EDGE_MSG_

#include <dataport/port_types.h>
#include "Params.hh"
using namespace std;

class EdgeMsg {

public:

   enum messageType{ STATUS, BEAM, UNKNOWN };

   EdgeMsg();
   ~EdgeMsg();

   void init( Params& params );
   int  readHdr( char* buffer );
   int  readMsg( char* buffer );
   
   messageType getMsgType(){ return msgType; }

   bool   isNewBeam(){ return newBeam; }

   int    getHeaderSize(){ return HEADER_SIZE; }
   
   double getAzimuth() const { return azimuth; }
   double getElevation() const { return elevation; }
   double getTargetElev() const { return targetElevation; }
   double getPrf() const { return prf; }
   int    getSamples() const { return samples; }
   int    getScanType() const { return scanType; }
   time_t getDataTime() const { return dataTime; }
   double getLat() const { return lat; }
   double getLon() const { return lon; }
   double getAlt() const { return alt; }
   double getPulseWidth() const { return pulseWidth; }
   double getNyquist() const { return nyquistVel; }

   ui08  *getDataPtr() const { return momentsData; }
   int    getDataLen() const { return dataLen; }

   static const int HEADER_SIZE;
   static const int MAX_STRING_LEN;
   static const int NUM_FIELDS;

   static const int REFLECTIVITY;
  static const int UNCORR_REFL;
   static const int VELOCITY;
   static const int SPECTRUM_WIDTH;
   static const int EDGE_PARAMS;
   
   static const double INPUT_DBZ_SCALE;
   static const double INPUT_DBZ_BIAS;
   
private:

   //
   // Pointer to message - Note that this class does
   // not own the memory associated with the message
   //
   char        *msgPtr;

   //
   // Read the header
   //
   bool         firstBeam;
   double       azimuth;
   double       elevation;
   int          compression;
   int          uncompressedLen;
   int          compressedLen;
   int          moment;

   int          uncompressMsg();

   //
   // Is it a status message or a beam message
   //
   messageType  msgType;
   

  // 
  // Current rolling count for error checking
  //

  int _prevStatusRollingCount;
  int _prevCorrReflRollingCount;
  int _prevUncorrReflRollingCount;
  int _prevVelRollingCount;
  int _prevSwRollingCount;
 
  // 
  // Flag indicating we should use local system time rather than the
  // data time received in the EDGE message
  //

  bool _useLocalSystemTime;
  
   //
   // Read the parameters
   //
   double       prf;
   double       prevPrf;
   int          samples;
   int          nGatesIn;
   double       inputGateSpacing;
   double       prevGateSpacing;
   int          scanType;
   time_t       dataTime;
   double       targetElevation;
   double       lat;
   double       lon;
   double       alt;
   double       pulseWidth;
   double       nyquistVel;
   double       wavelength;

   int          getParams();
   void         findNyquist();

   //
   // Reformat beam data
   //
   bool         newBeam;
   double       prevAzimuth;

   int          nGatesOut;
   double       outputGateSpacing;
   int          nLookupGates;
   int         *gateLookup;

   double       outputDbzScale, outputDbzBias;
   double       outputVelScale, outputVelBias;
   double       outputSwScale, outputSwBias;

   ui08         dbzLookup[256];
   ui08         velLookup[256];
   ui08         swLookup[256];

   ui08        *momentsData;
   int          dataLen;

   int          reformatBeam();
   void         resampleData( ui08* scaleTable, int fieldOffset );

   //
   // Diagnostics
   //
   int          summaryInterval;

  void _checkRollingCount(int &prev_rolling_count,
			  const int rolling_count,
			  const string msg_type) ;

  // Support methods

  inline static double _calcLatLon(const double deg, const double min,
				   const double sec)
  {
    if (deg < 0)
      return deg - (min/60.0) - (sec/3600.0);
   else
     return deg + (min/60.0) + (sec/3600.0);
  }
  
};

#endif
