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
/////////////////////////////////////////////////////////////////////////////
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1998
//
//////////////////////////////////////////////////////////////////////////////


#include <toolsa/umisc.h>
#include <dataport/bigend.h>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsRadarParams.hh>
#include <iostream>
using namespace std;

DsRadarBeam::DsRadarBeam()
{

   _beam         = NULL;
   _beamLen      = 0;
   _data         = NULL;
   _dataLen      = 0;
 
   dataTime      = -1;
   nanoSecs      = 0;
   referenceTime = 0;
   byteWidth     = 1;

   volumeNum  = 0;
   tiltNum    = 0;
   scanMode   = -1;
   antennaTransition    = false;
   
   azimuth    = 0;
   elevation  = 0;

   targetElev = 0;
   targetAz   = 0;

   beamIsIndexed = false;
   angularResolution = 1.0;
   nSamples = 0;

   measXmitPowerDbmH = 0;
   measXmitPowerDbmV = 0;

}

DsRadarBeam::DsRadarBeam( const DsRadarBeam &other ) 
{

   _beam    = NULL;
   _beamLen = 0;
   _dataLen = 0;
   _data    = NULL;
   
   copy( other );
   
}

DsRadarBeam::~DsRadarBeam()
{
   if( _beam )
      ufree(_beam);
}

DsRadarBeam&
DsRadarBeam::operator=( const DsRadarBeam &other )
{
  
  // check for copying on itself
  
  if (&other == this) {
    return *this;
  }
  
  copy( other );
  return( *this );
}

void
DsRadarBeam::copy( const DsRadarBeam& other )
{

  // Copy the header members

  copyHeader(other);

  // Copy the data

  loadData( other.data(), other.dataLen(), byteWidth );
   
}

// copy the header members only

void
DsRadarBeam::copyHeader( const DsRadarBeam& other )
{

   dataTime      = other.dataTime;
   nanoSecs      = other.nanoSecs;
   referenceTime = other.referenceTime;
   byteWidth     = other.byteWidth;
   volumeNum     = other.volumeNum;
   tiltNum       = other.tiltNum;
   scanMode      = other.scanMode;
   antennaTransition = other.antennaTransition;
   azimuth       = other.azimuth;
   elevation     = other.elevation;
   targetElev    = other.targetElev;
   targetAz      = other.targetAz;
   beamIsIndexed = other.beamIsIndexed;
   angularResolution = other.angularResolution;
   nSamples = other.nSamples;
   measXmitPowerDbmH = other.measXmitPowerDbmH;
   measXmitPowerDbmV = other.measXmitPowerDbmV;
   
}

// get time as a double
double DsRadarBeam::getDoubleTime() const
{
  double dtime = dataTime;
  dtime += nanoSecs / 1.0e9;
  return dtime;
}

// load up with generic data

void
DsRadarBeam::loadData( const void *in_data, int in_data_len,
		       int byte_width )
{

  _alloc( in_data_len );
  memcpy( _data, in_data, in_data_len);
  byteWidth = byte_width;

}

// load up with byte data

void
DsRadarBeam::loadData( const ui08 *in_data, int in_data_len )
{

  loadData((void *) in_data, in_data_len, 1 );

}

void
DsRadarBeam::decode( int nGatesIn, DsBeamHdr_t *bhdr_msg,
		     int nFields, int nGatesOut)
{

  // grab and swap header

  DsBeamHdr_t bhdr;
  memcpy(&bhdr, bhdr_msg, sizeof(bhdr));
  BE_to_DsBeamHdr(&bhdr);
  bool version1 = false;

  if (bhdr.byte_width == 1 ||
      bhdr.byte_width == 2 ||
      bhdr.byte_width == 4) {
    
  } else {

    // version 0
    // load new struct version from old version

    DsBeamHdr_v1_t bhdr_v1;
    memcpy(&bhdr_v1, bhdr_msg, sizeof(bhdr_v1));
    BE_to_array_32(&bhdr_v1, sizeof(DsBeamHdr_v1_t));
    version1 = true;

    MEM_zero(bhdr);
    bhdr.time = bhdr_v1.time;
    bhdr.vol_num = bhdr_v1.vol_num;
    bhdr.tilt_num = bhdr_v1.tilt_num;
    bhdr.reference_time = bhdr_v1.reference_time;
    bhdr.byte_width = bhdr_v1.byte_width;
    bhdr.azimuth = bhdr_v1.azimuth;
    bhdr.elevation = bhdr_v1.elevation;
    bhdr.target_elev = bhdr_v1.target_elev;

  }
  
  dataTime =  bhdr.time;
  nanoSecs = bhdr.nano_secs;
  referenceTime = bhdr.reference_time;
  byteWidth = bhdr.byte_width;
  volumeNum = bhdr.vol_num;
  tiltNum = bhdr.tilt_num;
  scanMode = bhdr.scan_mode;
  antennaTransition = bhdr.antenna_transition;
  azimuth = bhdr.azimuth;
  elevation = bhdr.elevation;
  targetElev = bhdr.target_elev;
  targetAz = bhdr.target_az;
  beamIsIndexed = bhdr.beam_is_indexed;
  angularResolution = bhdr.angular_resolution;
  nSamples = bhdr.n_samples;
  measXmitPowerDbmH = bhdr.measXmitPowerDbmH;
  measXmitPowerDbmV = bhdr.measXmitPowerDbmV;

  // alloc space for data

  int nBytesIn = nGatesIn * nFields * byteWidth;
  int nBytesOut = nGatesOut * nFields * byteWidth;
  _alloc(nBytesOut);

  // load header

  memcpy(_beam, &bhdr, sizeof(bhdr));

  // load data

  memset( _data, 0, nBytesOut );
  ui08 *msgDataPtr = (ui08*) bhdr_msg + sizeof(DsBeamHdr_t);
  if (version1) {
    msgDataPtr = (ui08*) bhdr_msg + sizeof(DsBeamHdr_v1_t);
  }
  if( nGatesOut <=  nGatesIn ) {
    memcpy( _data, msgDataPtr, (nBytesOut) );
  } else {
    memcpy( _data, msgDataPtr, (nBytesIn) );
  }

  // swap multi-byte data
  
  if (byteWidth == 2) {
    BE_to_array_16(_data, _dataLen);
  } else if (byteWidth == 4) {
    BE_to_array_32(_data, _dataLen);
  }

}

void *
DsRadarBeam::encode()
{

  //
  // Encode the beam header
  //

  DsBeamHdr_t *bhdr = (DsBeamHdr_t *) _beam;
  memset( bhdr, 0, sizeof(DsBeamHdr_t) );

  bhdr->time            = dataTime;
  bhdr->nano_secs       = nanoSecs;
  bhdr->reference_time  = referenceTime;
  bhdr->byte_width      = byteWidth;
  bhdr->vol_num         = volumeNum;
  bhdr->tilt_num        = tiltNum;
  bhdr->scan_mode       = scanMode;
  bhdr->antenna_transition = antennaTransition;
  bhdr->azimuth         = azimuth;
  bhdr->elevation       = elevation;
  bhdr->target_elev     = targetElev;
  bhdr->target_az       = targetAz;
  bhdr->beam_is_indexed = beamIsIndexed;
  bhdr->angular_resolution = angularResolution;
  bhdr->n_samples = nSamples;
  bhdr->measXmitPowerDbmH = measXmitPowerDbmH;
  bhdr->measXmitPowerDbmV = measXmitPowerDbmV;
  
  BE_from_DsBeamHdr(bhdr);
  
  // swap multi-byte data
  
  if (byteWidth == 2) {
    BE_from_array_16(_data, _dataLen);
  } else if (byteWidth == 4) {
    BE_from_array_32(_data, _dataLen);
  }

  // return pointer to the beam

  return _beam;

}

void
DsRadarBeam::print( FILE *out ) const
{
   
   fprintf(out, "BEAM PARAMS\n");
   
   fprintf(out, "  data time:  %s\n", utimstr(dataTime));
   if (nanoSecs != 0) {
     fprintf(out, "  nano secs:  %d\n", nanoSecs);
   }
   if (referenceTime != 0) {
     fprintf(out, "  reference time: %s\n", utimstr(referenceTime));
   }

   fprintf(out, "  byte width:  %d\n", byteWidth);

   fprintf(out, "  volume number:  %d\n", volumeNum);
   fprintf(out, "  tilt number:  %d\n", tiltNum);
   fprintf(out, "  scan mode: %s\n",
           DsRadarParams::scanMode2Str(scanMode).c_str());
   fprintf(out, "  antenna transition? %s\n", antennaTransition? "T" : "F");
   
   fprintf(out, "  azimuth:  %f\n", azimuth);
   fprintf(out, "  elevation:  %f\n", elevation);
   fprintf(out, "  target elevation:  %f\n", targetElev);
   fprintf(out, "  target azimuth:  %f\n", targetAz);
   
   fprintf(out, "  beam is indexed? %s\n", beamIsIndexed? "T" : "F");
   fprintf(out, "  angular resolution:  %f\n", angularResolution);
   fprintf(out, "  n time series samples:  %d\n", nSamples);

   fprintf(out, "  measXmitPowerDbmH:  %f\n", measXmitPowerDbmH);
   fprintf(out, "  measXmitPowerDbmV:  %f\n", measXmitPowerDbmV);

   fprintf(out, "\n");
}

void
DsRadarBeam::print( ostream &out ) const
{
   
  out << "BEAM PARAMS" << endl;
   
  out << "  data time:  " << utimstr(dataTime) << endl;
  if (nanoSecs != 0) {
    out << "  nano secs:  " << nanoSecs << endl;
  }
  if (referenceTime != 0) {
    out << "  reference time: " << utimstr(referenceTime) << endl;
  }

  out << "  byte width:  " << byteWidth << endl;
  out << "  volume number:  " << volumeNum << endl;
  out << "  tilt number:  " << tiltNum << endl;
  out << "  scan mode: "
      << DsRadarParams::scanMode2Str(scanMode).c_str() << endl;
  out << "  antenna transition? " << (antennaTransition? "T" : "F") << endl;
   
  out << "  azimuth:  " << azimuth << endl;
  out << "  elevation:  " << elevation << endl;
  out << "  target elevation:  " << targetElev << endl;
  out << "  target azimuth:  " << targetAz << endl;
   
  out << "  beam is indexed? " << (beamIsIndexed? "T" : "F") << endl;
  out << "  angular resolution: " << angularResolution << endl;
  out << "  n time series samples: " << nSamples << endl;
  
  out << "  measXmitPowerDbmH: " << measXmitPowerDbmH << endl;
  out << "  measXmitPowerDbmV: " << measXmitPowerDbmV << endl;
  
  out << endl;

}

// print data as raw bytes

void
DsRadarBeam::printByteData( FILE *out ) const
{
   
  ui08 *byteData = (ui08 *) _data;
  for (int i = 0; i < _dataLen; i++) {
    fprintf(out, "%d ", byteData[i]);
  }
  fprintf(out, "\n\n");
  fflush(out);

}

// print bytes as floats

void
DsRadarBeam::printFloatData( FILE *out,
			     const char *field_name,
			     int field_num,
			     int num_fields,
			     double scale,
			     double bias,
			     int byteWidth /* = 1 */,
			     int missingVal /* = 0 */) const
{
  
  if (byteWidth == 4) {

    // 4-byte floats

    printFl32(out, field_name, field_num, num_fields,
	      missingVal);
    
  } else if (byteWidth == 2) {

    // 2-byte unsigned ints

    printUi16AsFloats(out, field_name, field_num, num_fields,
		      scale, bias, missingVal);

  } else {

    // 1-byte unsigned ints

    printUi08AsFloats(out, field_name, field_num, num_fields,
		      scale, bias, missingVal);
    
  }
  
}

void
DsRadarBeam::printUi08AsFloats( FILE *out,
				const char *field_name,
				int field_num,
				int num_fields,
				double scale,
				double bias,
				int missingVal) const
{
  
  fprintf(out, "================================================\n");
  fprintf(out, "Field num: %d, name '%s'\n", field_num, field_name);
  ui08 *data = ((ui08 *) _data) + field_num;
  int nmissing = 0;
  for (int i = 0; i < _dataLen; i += num_fields) {
    if (data[i] == missingVal) {
      nmissing++;
    } else {
      if (nmissing > 0) {
	fprintf(out, "%d*miss ", nmissing);
	nmissing = 0;
      }
      fprintf(out, "%.1f ", data[i] * scale + bias);
    }
  }
  if (nmissing > 0) {
    fprintf(out, "%d*miss ", nmissing);
    nmissing = 0;
  }
  fprintf(out, "\n\n");
  fflush(out);

}

// print ui16 data as floats

void
DsRadarBeam::printUi16AsFloats( FILE *out,
				const char *field_name,
				int field_num,
				int num_fields,
				double scale,
				double bias,
				int missingVal ) const
{
  
  fprintf(out, "================================================\n");
  fprintf(out, "Field num: %d, name '%s'\n", field_num, field_name);
  ui16 *data = ((ui16*) _data) + field_num;
  int nmissing = 0;
  int nn = _dataLen / sizeof(ui16);
  for (int i = 0; i < nn; i += num_fields) {
    if (data[i] == missingVal) {
      nmissing++;
    } else {
      if (nmissing > 0) {
	fprintf(out, "%d*miss ", nmissing);
	nmissing = 0;
      }
      fprintf(out, "%.3f ", data[i] * scale + bias);
    }
  }
  if (nmissing > 0) {
    fprintf(out, "%d*miss ", nmissing);
    nmissing = 0;
  }
  fprintf(out, "\n\n");
  fflush(out);

}

// print fl32 data

void
DsRadarBeam::printFl32( FILE *out,
			const char *field_name,
			int field_num,
			int num_fields,
			int missingVal ) const
{
  
  fl32 missingFloat = (fl32) missingVal;
  
  fprintf(out, "================================================\n");
  fprintf(out, "Field num: %d, name '%s'\n", field_num, field_name);
  fl32 *data = ((fl32*) _data) + field_num;
  int nmissing = 0;
  int nn = _dataLen / sizeof(fl32);
  for (int i = 0; i < nn; i += num_fields) {
    if (data[i] == missingFloat) {
      nmissing++;
    } else {
      if (nmissing > 0) {
	fprintf(out, "%d*miss ", nmissing);
	nmissing = 0;
      }
      fprintf(out, "%.3f ", data[i]);
    }
  }
  if (nmissing > 0) {
    fprintf(out, "%d*miss ", nmissing);
    nmissing = 0;
  }
  fprintf(out, "\n\n");
  fflush(out);

}

void
DsRadarBeam::_alloc( int in_data_len )
{

  if((in_data_len != _dataLen) || _beam == NULL) {
    _dataLen = in_data_len;
    _beamLen = sizeof(DsBeamHdr_t) + _dataLen;
    _beam = urealloc(_beam, _beamLen);
    _data = (void *) ((ui08 *) _beam + sizeof(DsBeamHdr_t));
  }

}


