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
/////////////////////////////////////////////////////////////
// TDWRadial.cc
//
// TDWRadial object
//
// Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2001
//
///////////////////////////////////////////////////////////////

//#include <stdio.h>
#include "TDWRadial.hh"
#include <dataport/bigend.h>
using namespace std;

const int TDWRadial::SAMPLES_PER_DWELL = 4096;
// Constructors
TDWRadial::TDWRadial()
{
  _messageId = 0;
  _messageLength = 0;
  _volumeCount = 0;
  _volumeFlag = 0;
  _powerTrans = 0;
  _playbackFlag = 0;
  _scanInfoFlag = 0;
  _currentElevation = -1.0;
  _angularScanRate = -1.0;
  _PRI = 0;
  _finalRangeSample = 0;
  _rngSamplesPerDwell = 0;
  _azimuth = -1;
  _totalNoisePower = -1;
  _timeStamp = 0;
  _baseDataType = 0;
  _volElevStatusFlag = 0;
  _intergerAzimuth = 0;
  _emptyShort = 0;

}

TDWRadial::TDWRadial(const TDWRadial& input_params ) {
  copy(input_params);
}

TDWRadial::TDWRadial(TDWR_data_header_t *input_params ) {
  copy(input_params);
}

// equals operator

TDWRadial& TDWRadial::operator=( const TDWRadial &inputParams )
{
  copy( inputParams );
  return( *this );
}


// copy 

void TDWRadial::copy( const TDWRadial &inputParams )
{
  _messageId                = inputParams._messageId;
  _messageLength            = inputParams._messageLength;
  _volumeCount              = inputParams._volumeCount; 
  _volumeFlag               = inputParams._volumeFlag;
  _powerTrans               = inputParams._powerTrans;
  _playbackFlag             = inputParams._playbackFlag;
  _scanInfoFlag             = inputParams._scanInfoFlag;
  _currentElevation         = inputParams._currentElevation;
  _angularScanRate          = inputParams._angularScanRate;
  _PRI                      = inputParams._PRI;
  _finalRangeSample         = inputParams._finalRangeSample;
  _rngSamplesPerDwell       = inputParams._rngSamplesPerDwell;
  _azimuth                  = inputParams._azimuth;
  _totalNoisePower          = inputParams._totalNoisePower;
  _timeStamp                = inputParams._timeStamp;
  _baseDataType             = inputParams._baseDataType;
  _volElevStatusFlag        = inputParams._volElevStatusFlag;
  _intergerAzimuth          = inputParams._intergerAzimuth;
  _emptyShort               = inputParams._emptyShort;

}

void TDWRadial::copy( const TDWR_data_header_t *inputParams )
{
  _messageId                = inputParams->message_id;
  _messageLength            = inputParams->message_length;
  _volumeCount              = inputParams->volume_count; 
  _volumeFlag               = inputParams->volume_flag;
  _powerTrans               = inputParams->power_trans;
  _playbackFlag             = inputParams->playback_flag;
  _scanInfoFlag             = inputParams->scan_info_flag;
  _currentElevation         = inputParams->current_elevation;
  _angularScanRate          = inputParams->angular_scan_rate;
  _PRI                      = inputParams->pri;
  _finalRangeSample         = inputParams->final_range_sample;
  _rngSamplesPerDwell       = inputParams->rng_samples_per_dwell;
  _azimuth                  = inputParams->azimuth;
  _totalNoisePower          = inputParams->total_noise_power;
  _timeStamp                = inputParams->timestamp;
  _baseDataType             = inputParams->base_data_type;
  _volElevStatusFlag        = inputParams->vol_elev_status_flag;
  _intergerAzimuth          = inputParams->interger_azimuth;
  _emptyShort               = inputParams->empty_short;

}

// Print methods

void TDWRadial::print(FILE *out)
{
  fprintf(out, "RADAR PARAMS\n");

  (void) fprintf(out, "  message id:  %d\n",  _messageId);
  (void) fprintf(out, "  message length:  %d\n",   _messageLength);
  (void) fprintf(out, "  volume count:  %d\n",  _volumeCount);
  (void) fprintf(out, "  volume flag:  %d\n",  _volumeFlag);
  (void) fprintf(out, "  power trans:  %d\n",  _powerTrans);
  (void) fprintf(out, "  playbackFlag:  %d\n",  _playbackFlag);
  (void) fprintf(out, "  scanInfoFlag:  %d\n",  _scanInfoFlag);
  (void) fprintf(out, "  currentElevation:  %f\n",  _currentElevation);
  (void) fprintf(out, "  angularScanRate:  %f\n",  _angularScanRate);
  (void) fprintf(out, "  pri:  %d\n",  _PRI);
  (void) fprintf(out, "  finalRangeSample:  %d\n",  _finalRangeSample);
  (void) fprintf(out, "  rngSamplesPerDwell:  %d\n",  _rngSamplesPerDwell);
  (void) fprintf(out, "  Azimuth:  %f\n",  _azimuth);
  (void) fprintf(out, "  totalNoisePower:  %f\n",  _totalNoisePower);
  (void) fprintf(out, "  timeStamp:  %d\n",  _timeStamp);
  (void) fprintf(out, "  baseDataType:  %d\n",  _baseDataType);
  (void) fprintf(out, "  volElevStatusFlag:  %d\n",  _volElevStatusFlag);
  (void) fprintf(out, "  intergerAzimuth:  %d\n",  _intergerAzimuth);
  (void) fprintf(out, "  emptyShort:  %d\n",  _emptyShort);

  (void) fprintf(out, "\n");

}

void TDWRadial::print(ostream &out)
{
  out << "RADAR PARAMS" << endl;

  out << "  message id:  " <<  _messageId << endl;
  out << "  message length:  " <<   _messageLength << endl;
  out << "  volume count:  " <<  _volumeCount << endl;
  out << "  volume flag:  " <<  _volumeFlag << endl;
  out << "  power trans:  " <<  _powerTrans << endl;
  out << "  playbackFlag:  " << _playbackFlag << endl;
  out << "  scanInfoFlag:  " <<  _scanInfoFlag << endl;
  out << "  currentElevation:  " <<  _currentElevation << endl;
  out << "  angularScanRate:  " <<  _angularScanRate << endl;
  out << "  pri:  " <<  _PRI << endl;
  out << "  finalRangeSample:  " <<  _finalRangeSample << endl;
  out << "  rngSamplesPerDwell:  " <<  _rngSamplesPerDwell << endl;
  out << "  Azimuth:  " <<  _azimuth << endl;
  out << "  totalNoisePower:  " <<  _totalNoisePower << endl;
  out << "  timeStamp:  " <<  _timeStamp << endl;
  out << "  baseDataType:  " <<  _baseDataType << endl;
  out << "  volElevStatusFlag:  " <<  _volElevStatusFlag << endl;
  out << "  intergerAzimuth:  " <<  _intergerAzimuth << endl;
  out << "  emptyShort:  " <<  _emptyShort << endl;
  out << endl;

}

//////////////////////////////////////////////////
// isLLWASData - LLWAS data also is transmitted from
//              the radar, this method returns true
//              if it is detected

const bool TDWRadial::isLLWASData() {

  if (_messageId == LLWASII_DATA || 
			_messageId == LLWASIII_DATA || 
			_messageId == LLWASII_MAPPING)
	  return TRUE;

  return FALSE;
}

//////////////////////////////////////////////////
// isLowPrf 
//              the radar, this method returns true
//              if it is detected

const bool TDWRadial::isLowPrf() {

  if (_messageId == LOW_PRF_BASE_DATA)
	  return TRUE;

  return FALSE;
}

////////////////////////////////////////////////////////
// newTilt - a new elevation/tilt starts when a
//              scan info flag is set 

const bool TDWRadial::newTilt() {

  if (_scanInfoFlag & START_OF_NEW_ELEVATION)
	  return TRUE;

  return FALSE;
}

////////////////////////////////////////////////////////
// newVolume - a new volume starts if the volume flag 
//             flag is

const bool TDWRadial::newVolume() {

  if (_volumeFlag & BEGINNING_OF_VOL_SCAN)
	  return TRUE;

  return FALSE;
}

////////////////////////////////////////////////////////
// getScanType - returns either a 1 or 2

const int TDWRadial::getScanType() {
  int scan_type = 0;
  

  scan_type = (_volumeFlag & SCAN_STRATEGY);

  if (scan_type == HAZARDOUS) {
    // cerr << "scan type 2" << endl;
    return (2);
  }

  if (scan_type == MONITOR || scan_type == CLEAR_AIR || 
             scan_type == CLUTTER_COL || scan_type == CLUTTER_EDIT) {
    // cerr << "scan type 1" << endl;
    return (1);
  }

  cerr << "scan type " << scan_type << " unknown, contact programmer" << endl;
  return (-1);
}

////////////////////////////////////////////////////////
// getScanMode - returns either a ppi or rhi

const int TDWRadial::getScanMode() {
  int scan_type = 0;
  

  if ((_scanInfoFlag & SECTOR_SCAN) == SECTOR_SCAN)
	// return sector
	return (SECTOR);
  else {
    scan_type = (int) _volumeFlag & SCAN_STRATEGY;
	if (scan_type == RHI_SCAN)
	  return (RHI);
	else
	  return (PPI);
  }
}

////////////////////////////////////////////////////////
// loadScanParams - load private variables from tdwr header

const void TDWRadial::loadScanParams (scan_description_t& sParams) {

	sParams.final_range_sample = _finalRangeSample;
	sParams.rng_samples_per_dwell = _rngSamplesPerDwell;
	sParams.dwell_flag = _dwellFlag;
	sParams.pri = _PRI;
	sParams.message_id = _messageId;
	sParams.elevation = _currentElevation;
	sParams.target_elev = _currentElevation;
	sParams.tilt_num = _scanInfoFlag >> 24;
	sParams.volume_num = _volumeCount;

}

/////////////////////////////////////////////////////////
// BE_to_frame_hdr - convert from big endian if necessary
//

void TDWRadial::BE_to_frame_hdr(Packet_hdr_t *hdr) {

  BE_to_array_32(&hdr->seq, 4);
  BE_to_array_16(&hdr->msg_length, 2);
  BE_to_array_16(&hdr->frames_per_msg, 2);
  BE_to_array_16(&hdr->frame_num_in_msg, 2);
  BE_to_array_16(&hdr->frame_offset, 2);
  BE_to_array_16(&hdr->frame_length, 2);
}

/////////////////////////////////////////////////////////
// BE_to_tdwr_data_hdr - convert from big endian if necessary
// 

void TDWRadial::BE_to_tdwr_data_hdr(TDWR_data_header_t *hdr) {
  BE_to_array_16(&hdr->message_id, 2);
  BE_to_array_16(&hdr->message_length, 2);
  BE_to_array_16(&hdr->volume_count, 2);
  BE_to_array_16(&hdr->volume_flag, 2);
  BE_to_array_16(&hdr->power_trans, 2);
  BE_to_array_16(&hdr->playback_flag, 2);
  BE_to_array_32(&hdr->scan_info_flag, 4);
  BE_to_array_32(&hdr->current_elevation, 4);
  BE_to_array_32(&hdr->angular_scan_rate, 4);
  BE_to_array_16(&hdr->pri, 2);
  BE_to_array_16(&hdr->dwell_flag, 2);
  BE_to_array_16(&hdr->final_range_sample, 2);
  BE_to_array_16(&hdr->rng_samples_per_dwell, 2);
  BE_to_array_32(&hdr->azimuth, 4);
  BE_to_array_32(&hdr->total_noise_power, 4);
  BE_to_array_32(&hdr->timestamp, 4);
  BE_to_array_16(&hdr->base_data_type, 2);
  BE_to_array_16(&hdr->vol_elev_status_flag, 2);
  BE_to_array_16(&hdr->interger_azimuth, 2);
  BE_to_array_16(&hdr->empty_short, 2);
}


