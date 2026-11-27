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
#include <math.h>
#include <zlib.h>

#include <toolsa/MsgLog.hh>

#include "DataMgr.hh"
#include "EdgeMsg.hh"
#include "Edge2Dsr.hh"

using namespace std;

const int EdgeMsg::HEADER_SIZE    = 40;
const int EdgeMsg::MAX_STRING_LEN = 18;
const int EdgeMsg::NUM_FIELDS     = 4;

const int EdgeMsg::REFLECTIVITY   = 8;
const int EdgeMsg::UNCORR_REFL    = 4;
const int EdgeMsg::VELOCITY       = 2;
const int EdgeMsg::SPECTRUM_WIDTH = 1;
const int EdgeMsg::EDGE_PARAMS    = 32;

const double EdgeMsg::INPUT_DBZ_SCALE  = 0.5;
const double EdgeMsg::INPUT_DBZ_BIAS   = -32.0;

EdgeMsg::EdgeMsg() :
  _prevStatusRollingCount(-1),
  _prevCorrReflRollingCount(-1),
  _prevUncorrReflRollingCount(-1),
  _prevVelRollingCount(-1),
  _prevSwRollingCount(-1),
  _useLocalSystemTime(false)
{
   firstBeam             = true;
   inputGateSpacing      = 0.0;
   prevGateSpacing       = 0.0;
   msgPtr                = NULL;
   nLookupGates          = 0;
   gateLookup            = NULL;
   momentsData           = NULL;
}

EdgeMsg::~EdgeMsg() 
{
   if( gateLookup )
      delete[] gateLookup;
   if( momentsData )
      delete[] momentsData;
}

void
EdgeMsg::init( Params& params ) 
{
   //
   // Set gate information - Gate spacing is stored
   // in meters here
   //
   nGatesOut         = params.n_gates_out;
   outputGateSpacing = params.gate_spacing * 1000.0;

   //
   // Set wavelength
   //
   wavelength        = params.wavelength;

   //
   // Set use local time flag
   //
   _useLocalSystemTime = params.use_local_system_time;
   
   //
   // Set scales and biases
   //
   outputDbzScale = params.dbz_scaling_info.scale;
   outputDbzBias  = params.dbz_scaling_info.bias;
   outputVelScale = params.vel_scaling_info.scale;
   outputVelBias  = params.vel_scaling_info.bias;
   outputSwScale  = params.sw_scaling_info.scale;
   outputSwBias   = params.sw_scaling_info.bias;

   //
   // Dbz lookup table is constant, so set it now
   //
   double dbz;
   dbzLookup[0] = 0;
   for( int i = 1; i < 256; i++ ) {
      dbz = (double) i * INPUT_DBZ_SCALE + INPUT_DBZ_BIAS;
      dbzLookup[i] = (ui08) ((dbz - outputDbzBias)/outputDbzScale + 0.5);
   }

   dataLen     = nGatesOut * NUM_FIELDS;
   momentsData = new ui08[dataLen];

   //
   // Set up array for gate lookup table - this
   // table will be used for resampling data into the output
   // gate spacing.  It will be no larger than nGatesOut, but
   // could be smaller
   //
   gateLookup = new int[nGatesOut];

}

int
EdgeMsg::readHdr( char* buffer ) 
{
   int binaryAz, binaryEl, checkSum;
   
   //
   // Read the header.
   //
   // There are now 2 versions of the header.  The header was updated
   // when the TCP/IP input was added.
   //
   int nParts;

   unsigned int rolling_count;
   
   if ((nParts = sscanf(buffer,
			"%04x %04x %08x %4d %4d %04x %d %02x",
			&binaryAz, &binaryEl, &checkSum, &uncompressedLen,
			&compressedLen, &moment, &compression,
			&rolling_count)) == 8)
   {
     // Check for errors with the rolling count

     switch (moment)
     {
     case REFLECTIVITY :
       _checkRollingCount(_prevCorrReflRollingCount, rolling_count,
			  "corrected reflectivity");
       break;
       
     case UNCORR_REFL :
       _checkRollingCount(_prevUncorrReflRollingCount, rolling_count,
			  "uncorrected reflectivity");
       break;
       
     case VELOCITY :
       _checkRollingCount(_prevVelRollingCount, rolling_count,
			  "velocity");
       break;
       
     case EDGE_PARAMS :
       _checkRollingCount(_prevStatusRollingCount, rolling_count,
			  "status");
       break;
       
     case SPECTRUM_WIDTH :
       _checkRollingCount(_prevSwRollingCount, rolling_count,
			  "spectrum width");
       break;
       
     default:
       POSTMSG(ERROR,
	       "Unknown moments type in header: %d",
	       moment);
       break;
     } /* endswitch - moment */
     
   }
   else if ((nParts = sscanf(buffer,
			     "%04x %04x %08x %4d %4d %04x %1d",
			     &binaryAz, &binaryEl, &checkSum, &uncompressedLen,
			     &compressedLen, &moment, &compression)) == 7)
   {
     // Do nothing -- continue on with processing old style header
   }
   else
   {
     POSTMSG( ERROR,
	      "Couldn't read the header: %s\n"
	      "Expected: binaryAz binaryEl checkSum uncompressedLen compressedLen moment compression   -- OR --\n"
	      "          binaryAz binaryEl checkSum uncompressedLen compressedLen moment compression rolling_count",
	      buffer );
     return( FAILURE );
   }

   //
   // Convert azimuth and elevation
   //
   azimuth   = binaryAz/65536.0 * 360.0;
   elevation = binaryEl/65536.0 * 360.0;

   //
   // Check the elevation and azimuth
   //
   if( elevation > 90.0 || elevation < 0.0 ||
       azimuth > 360.0 || azimuth < 0.0 ) {
      POSTMSG( ERROR, "elevation = %f, azimuth = %f", elevation, azimuth );
      return( FAILURE );
   }

   //
   // Set the message type
   //
   msgType = UNKNOWN;
   newBeam = false;

   switch (moment)
   {
   case REFLECTIVITY :
   case UNCORR_REFL :
   case VELOCITY :
   case SPECTRUM_WIDTH :
     msgType = BEAM;
     if (!firstBeam && azimuth != prevAzimuth)
     {
       newBeam = true;
     }
     
     prevAzimuth = azimuth;
     break;
     
   case EDGE_PARAMS :
      msgType = STATUS;
      break;
      
   default:
     POSTMSG(ERROR,
	     "Unknown moment type in header: %d",
	     moment);
     return FAILURE;
     break;
     
   } /* endswitch - moment */

   return( SUCCESS );
}


int
EdgeMsg::readMsg( char* buffer )
{
   //
   // Set message pointer
   //
   msgPtr  = buffer;

   //
   // Uncompress the message
   //
   if( uncompressMsg() != SUCCESS ) {
      POSTMSG(ERROR, "Error uncompressing message: %s", buffer);
      return( FAILURE );
   }
   
   switch( msgType ) {
       case BEAM:
	  if( reformatBeam() != SUCCESS ) {
	    POSTMSG(ERROR, "Error reformatting beam");
	     return( FAILURE );
	  }
          firstBeam = false;
	  break;
	  
       case STATUS:
	  if( getParams() != SUCCESS ) {
	    POSTMSG(ERROR, "Error getting parameters from status message");
	     return( FAILURE );
	  }
          findNyquist();
	  break;

       case UNKNOWN:
	  break;
   }
   
   return( SUCCESS );
}


int EdgeMsg::uncompressMsg() 
{
  POSTMSG(DEBUG, "Uncompressing message");

  switch (compression)
  {
  case 0:
    // No compression
    if( compressedLen != uncompressedLen )
    {
      POSTMSG(ERROR,
	      "Msg not compressed, but compressedLen != uncompressedLen");
      return FAILURE;
    }
    return SUCCESS;
    break;
	  
  case 1:
    POSTMSG(ERROR, "LZW compression not supported");
    return FAILURE;
    break;
	  
  case 2:
    POSTMSG(ERROR, "RUN_LEN compression not supported");
    return FAILURE;
    break;
	  
  case 3:
  {
    // zlib compression

    char *uncompressed_data = new char[uncompressedLen];
    uLongf out_len = uncompressedLen;
    
    int iret;
    
    if ((iret = uncompress((Bytef *)uncompressed_data, &out_len,
			   (const Bytef *)(msgPtr + HEADER_SIZE),
			   compressedLen)) != Z_OK)
    {
      switch (iret)
      {
      case Z_MEM_ERROR :
	POSTMSG(ERROR, "zlib uncompression failed:  Not enough memory");
	break;
	
      case Z_BUF_ERROR :
	POSTMSG(ERROR,
		"zlib uncompression failed: Not enough room in output buffer");
	break;
	
      case Z_DATA_ERROR :
	POSTMSG(ERROR,
		"zlib uncompression failed: Input data corrupted");
	break;
	
      default:
	POSTMSG(ERROR,
		"zlib uncompression failed: Unknown error code returned from uncompress() call");
	break;
      }
      
      delete [] uncompressed_data;
      return FAILURE;
    }
    
    if ((uLongf)uncompressedLen != out_len)
    {
      POSTMSG(ERROR, "zlib uncompression failed: Expected %d uncompressed bytes, got %d uncompressed bytes",
	      uncompressedLen, out_len);
      return FAILURE;
    }

    // Make sure there is enough space in the original message buffer 
    // for the uncompressed message.

    if ((int)(HEADER_SIZE + out_len) > DataMgr::MAX_BUF_SIZE)
    {
      POSTMSG(ERROR, "zlib uncompression failed: Original buffer too small for uncompressed message\n"
	      "Message buffer has %d bytes, uncompressed message is %d bytes",
	      DataMgr::MAX_BUF_SIZE - HEADER_SIZE,out_len);
      return FAILURE;
    }

    // Copy the uncompressed data over the original data.  This must be
    // done because msgPtr points to a static buffer allocated inside the
    // DataMgr object.

    memcpy(msgPtr + HEADER_SIZE, uncompressed_data, out_len);
    delete [] uncompressed_data;

    return SUCCESS;

    break;
  }
  
  default:
    POSTMSG(ERROR, "Compression type %d unknown", compression);
    return FAILURE;
    break;
  }
   
  return SUCCESS;
}

int
EdgeMsg::getParams()
{
   char *bufferPtr = msgPtr + HEADER_SIZE;

   // Declare new fields in record 1

   int version_num;
   float time_series_range;
   int processing_mode;
   
   //
   // Record 1
   //
   unsigned int prf1, prf2, range, uiSamples;
   if (sscanf(bufferPtr, "%4d %4d %8d %3d",
	      &prf1, &prf2, &range, &uiSamples) == 4)
   {
     // Do nothing -- continue on with processing old style record
   }
   else if (sscanf(bufferPtr, "V%4d %4d %4d %8d %3d %f %d",
		   &version_num, &prf1, &prf2, &range, &uiSamples,
		   &time_series_range, &processing_mode) == 7)
   {
     // Do nothing -- continue on with processing new style record
   }
   else
   {
      POSTMSG( ERROR, "Could not read record 1: %s\n"
	       "Expected: prf1 prf2 range uiSamples  -- OR --\n"
	       "          version_num prf1 prf2 range uiSamples time_series_range processing_mode", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 2
   //
   float gw1, gw2;
   unsigned int gwPartition, rangeAvg, gates;
   if( sscanf( bufferPtr, "%f %f %8d %3d %4d",
               &gw1, &gw2, &gwPartition,
               &rangeAvg, &gates ) != 5 ) {
      POSTMSG( ERROR, "Could not read record 2: %s\n"
	       "Expected: gw1 gw2 gwPartition rangeAvg gates", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;

   //
   // Record 3
   //
   unsigned int momentEnable, softwareSim;
   if( sscanf( bufferPtr, "%02x %1d",
               &momentEnable, &softwareSim ) != 2 ) {
      POSTMSG( ERROR, "Could not read record 3: %s\n"
	       "Expected: momentEnable softwareSim", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 4
   //
   unsigned int uiScanType, binTargAz, binTargEl;
   unsigned int speed, antennaSpeed, elevSpeed;
   unsigned int startAngle, stopAngle;
   if( sscanf( bufferPtr, "%1d %04x %04x %04x %04x %04x %04x %04x",
               &uiScanType, &binTargAz, &binTargEl, &speed, &antennaSpeed,
               &elevSpeed, &startAngle, &stopAngle ) != 8 ) {
      POSTMSG( ERROR, "Could not read record 4: %s\n"
	       "Expected: uiScanType binTargAz binTargEl speed antennaSpeed elevSpeed startAngle stopAngle", bufferPtr );
      return( FAILURE );
   }
   bufferPtr      += strlen( bufferPtr ) + 1;
   
   //
   // Record 5 - Note that we cannot use the size function on
   // the strings since there may be a null terminator before
   // the end of this string (since we are using substrings)
   //
   char siteName[MAX_STRING_LEN];
   char radarType[MAX_STRING_LEN];
   char jobName[MAX_STRING_LEN];
   unsigned int dTime;
   if( sscanf( bufferPtr, "%08x \"%[^\"]\" \"%[^\"]\" \"%[^\"]\"",
	       &dTime, siteName, radarType, jobName ) != 4 ) {
      POSTMSG( ERROR, "Could not read record 5: %s\n"
	       "Expected: dTime siteName radarType jobName", bufferPtr );
      return( FAILURE );
   }

   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 6
   //
   int lonDeg, lonMin, lonSec;
   int latDeg, latMin, latSec;
   unsigned int antennaHeight;
   if( sscanf( bufferPtr, "%4d %3d %3d %4d %3d %3d %5d",
               &lonDeg, &lonMin, &lonSec, &latDeg, &latMin, &latSec,
               &antennaHeight ) != 7 )  {
      POSTMSG( ERROR, "Could not read record 6: %s\n"
	       "Expected: lonDeg lonMin lonSec latDet latMin latSec antennaHeight", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 7
   //
   unsigned int binaryAz, binaryEl, scdFlag;
   if( sscanf( bufferPtr, "%04x %04x %04x",
	       &binaryAz, &binaryEl, &scdFlag ) != 3 ) {
      POSTMSG( ERROR, "Could not read record 7: %s\n"
	       "Expected: binaryAz binaryEl scdFlag", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 8
   //
   unsigned int sigprocFlag, interfaceType, radarPower;
   unsigned int servo, radiate;
   if( sscanf( bufferPtr, "%1d %1d %2d %2d %2d",
	       &sigprocFlag, &interfaceType, &radarPower, 
               &servo, &radiate ) != 5 ) {
      POSTMSG( ERROR, "Could not read record 8: %s\n"
	       "Expected: sigprocFlag interfaceType radarPower servo radiate", bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;
   
   //
   // Record 9.
   //
   // My documentation for this record is different from
   // the original code, so both are included here.  -- NR
   //
   unsigned int flags, tcfZ, tcfU, tcfV, tcfW;
   unsigned int clutterFilter, sqi, pw, fold;
   float radar_wavelength;
   if( sscanf( bufferPtr, "%04x %04x %04x %04x %04x %1d %3d %1d %1d",
	       &flags, &tcfZ, &tcfU, &tcfV, &tcfW, &clutterFilter, &sqi,
	       &pw, &fold ) == 9 )
   {
     // Do nothing -- continue with processing of original format record
   }
   else if (sscanf(bufferPtr, "%04x %04x %04x %04x %04x %04x %3d %1d %1d %7G",
		   &flags, &tcfZ, &tcfU, &tcfV, &tcfW, &clutterFilter, &sqi,
		   &pw, &fold, &radar_wavelength) == 10)
   {
     // Do nothing -- continue with processing record matching current docs
   }
   else
   {
      POSTMSG( ERROR, "Could not read record 9: %s\n"
	       "Expected: flags tcfZ tcfU tcfV tcfW clutterFilter sqi pw fold  -- OR --\n"
	       "          flags tcfZ tcfU tcfV tcfW clutterFilter sqi pw fold radar_wavelength",
	       bufferPtr );
      return( FAILURE );
   }
   bufferPtr += strlen( bufferPtr ) + 1;

   //
   // Don't need any information from records 10 - 14
   //

   //
   // Check azimuth, elevation and scan type
   //
   if( uiScanType != 0 ) {
      POSTMSG( WARNING, "Unrecognized scan type %d\n", uiScanType );
      return( FAILURE );
   }

   double paramsAz = binaryAz/65536.0 * 360.0;
   if( paramsAz != azimuth ) {
      POSTMSG( WARNING, "Azimuth does not match header" );
      POSTMSG( WARNING, "Header azimuth = %f, Params azimuth = %f", 
	       azimuth, paramsAz );
   }

   double paramsEl = binaryEl/65536.0 * 360.0;
   if( paramsEl != elevation ) {
      POSTMSG( WARNING, "Elevation does not match header" );
      POSTMSG( WARNING, "Header elevation = %f, Params elevation = %f",
	       elevation, paramsEl );
   }

   //
   // Set parameters
   //
   prf               = (double) prf1;
   samples           = (int) uiSamples;
   inputGateSpacing  = gw1;
   scanType          = (int) uiScanType;
   if (_useLocalSystemTime)
     dataTime        = time(0);
   else
     dataTime        = (time_t) dTime;
   targetElevation   = binTargEl/65536.0 * 360.0;
   lat = _calcLatLon(latDeg, latMin, latSec);
   lon = _calcLatLon(lonDeg, lonMin, lonSec);
   alt               = (double) antennaHeight;
   pulseWidth        = (double) pw * 100.0;
   
   return( SUCCESS );
   
}

void
EdgeMsg::findNyquist() 
{
   static bool firstCall = true;
   
   if( !firstCall && prevPrf == prf ) 
      return;
   
   POSTMSG(DEBUG,
	   "Calculating Nyquist vel and vel/sw lookup tables");
   
   nyquistVel = ((wavelength/100.0) * prf) / 4.0;

   double inputVelScale = (2 * nyquistVel) / 254;
   double inputVelBias  = -nyquistVel;
   double inputSwScale  = 2 * nyquistVel / 255;

   double vel, sw;
   velLookup[0] = 0;
   swLookup[0]  = 0;
   for( int i = 1; i < 256; i++ ) {
      vel = (double) (i-1) * inputVelScale + inputVelBias;
      velLookup[i] = 
	((ui08) ((vel - outputVelBias)/outputVelScale + 0.5)) % 256;
      
      sw = (double) i * inputSwScale;
      swLookup[i] = (ui08) ((sw - outputSwBias)/outputSwScale + 0.5 );
   }
   
   prevPrf   = prf;
   firstCall = false;

}

int
EdgeMsg::reformatBeam() 
{
   if( newBeam ) {
      memset( (void *) momentsData, (int) 0, dataLen );
   }
   
   nGatesIn = uncompressedLen;

   switch( moment ) {
       
       case REFLECTIVITY:
          resampleData( dbzLookup, 0 );
          break;
	 
       case VELOCITY:
          resampleData( velLookup, 1 );
          break;

       case SPECTRUM_WIDTH:
          resampleData( swLookup, 2 );
          break;

       case UNCORR_REFL:
          resampleData( dbzLookup, 3 );
          break;
	 
   }

   return( SUCCESS );
}

void 
EdgeMsg::resampleData( ui08* scaleTable, int fieldOffset ) 
{
   static bool firstCall = true;
   
   //
   // If the gate spacing has changed from the
   // last beam, or if this is the first beam,
   // setup the resampling lookup table
   //
   if( inputGateSpacing != prevGateSpacing || firstCall ) {

     POSTMSG(DEBUG,
	     "Input gate spacing changed -- recalculating gate lookup");
     
      memset( (void *) gateLookup, (int) 0, nGatesOut*sizeof(int) );

      int destIdex   = 0;
      int sourceIdex = 0;
      nLookupGates   = 0;

      while( destIdex < nGatesOut && sourceIdex < nGatesIn ) {
	if( (destIdex+1)*outputGateSpacing <= 
            (sourceIdex+1)*inputGateSpacing ) {
          gateLookup[destIdex] = sourceIdex;
          destIdex++;
          nLookupGates++;
        } else {
          sourceIdex++;
        }
      }
      
      prevGateSpacing = inputGateSpacing;
   }
      
   //
   // Set the output data field
   //
   int   scaleIdex, gateIdex;
   ui08 *dest   = momentsData + fieldOffset;
   ui08 *source = (ui08 *) (msgPtr + HEADER_SIZE);

   for( int i = 0; i < nLookupGates; i++ ) {
      gateIdex  = gateLookup[i];
      scaleIdex = (int) (source[gateIdex]);
      *dest = scaleTable[scaleIdex];
      dest += NUM_FIELDS;
   }

   firstCall = false;
}


void EdgeMsg::_checkRollingCount(int &prev_rolling_count,
				 const int rolling_count,
				 const string msg_type) 
{
  if (prev_rolling_count >= 0)
  {
    int expected_count = prev_rolling_count + 1;
    expected_count = expected_count % 256;
     
    if (rolling_count != expected_count &&
	rolling_count != prev_rolling_count)
    {
      POSTMSG(ERROR,
	      "Error in rolling count for %s type message:\n"
	      "rolling count = %d, expected rolling count = %d",
	      msg_type.c_str(), rolling_count, expected_count);
    }
  }
     
  prev_rolling_count = rolling_count;
}
