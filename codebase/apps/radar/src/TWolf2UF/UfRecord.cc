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

/*********************************************************************
 * UfRecord.cc
 *
 * Object represting a single UF record
 *
 *********************************************************************/

#include "UfRecord.hh"
#include "DateTime.hh"
#include "ByteOrder.hh"
#include <cmath>
#include <ctime>
#include <cerrno>
#include <cstdio>
using namespace std;

///////////////
// constructor

UfRecord::UfRecord()

{
  _init();
}

///////////////
// destructor

UfRecord::~UfRecord()

{
  free();
}

///////////////////////
// initialize

void UfRecord::_init()

{

  _debug = false;

  memset(&_manHdr, 0, sizeof(_manHdr));
  memset(&_dataHdr, 0, sizeof(_dataHdr));
  _fieldInfo.clear();
  _fieldHdrs.clear();
  _fieldNames.clear();
  _shortData.clear();

  _numGates = 0;
  _numSamples = 0;
  _polarizationCode = 0;
  _startRangeKm = -9999;
  _gateSpacingKm = -9999;
  _radarConstant = -9999;
  _noisePowerDbm = -9999;
  _receiverGainDb = -9999;
  _peakPowerDbm = -9999;
  _antennaGainDb = -9999;
  _pulseWidthUs = -9999;
  _horizBeamWidthDeg = -9999;
  _vertBeamWidthDeg = -9999;
  _wavelengthCm = -9999;
  _prf = -9999;
  _nyquistVel = -9999;

  _volNum = 0;
  _tiltNum = 0;
  _targetAngle = -9999;

  _elevation = -9999;
  _azimuth = -9999;
  _beamTime = 0;

}

///////////////////////
// clear data

void UfRecord::clearData()

{

  memset(&_manHdr, 0, sizeof(_manHdr));
  memset(&_dataHdr, 0, sizeof(_dataHdr));
  free();
  
  _numGates = 0;
  _numSamples = 0;
  _startRangeKm = -9999;
  _gateSpacingKm = -9999;

  _volNum = 0;
  _tiltNum = 0;
  _targetAngle = -9999;

  _elevation = -9999;
  _azimuth = -9999;
  _beamTime = 0;

}

///////////////////////
// free up memory

void UfRecord::free()

{

  _fieldInfo.clear();
  _fieldHdrs.clear();
  _fieldNames.clear();
  _shortData.clear();

}

///////////////////////////////////////////////
// disassemble the object from a raw UF record
// Returns 0 on success, -1 on failure

int UfRecord::disassemble(const void *buf, int nBytes)

{

  clearData();

  if (_debug) {
    cerr << "======================================" << endl;
    cerr << "  sizeof(UF_mandatory_header_t): "
         << sizeof(UF_mandatory_header_t) << endl;
    cerr << "  sizeof(UF_optional_header_t) : "
         << sizeof(UF_optional_header_t) << endl;
    cerr << "  sizeof(UF_field_info_t)      : "
         << sizeof(UF_field_info_t) << endl;
    cerr << "  sizeof(UF_data_header_t)     : "
         << sizeof(UF_data_header_t) << endl;
    cerr << "  sizeof(UF_field_header_t)    : "
         << sizeof(UF_field_header_t) << endl;
    cerr << "======================================" << endl;
  }

  const si16 *record = (const si16 *) buf;
  int nShorts = nBytes / sizeof(si16);
  int nExpected = sizeof(UF_mandatory_header_t) / sizeof(si16);

  if (_debug) {
    cerr << "  -->> reading mandatory header, size: "
         << sizeof(UF_mandatory_header_t) << endl;
    cerr << "    record min nbytes: " << nExpected * sizeof(si16) << endl;
  }

  if (nShorts < nExpected) {
    cerr << "ERROR - UfRecord::disassemble" << endl;
    cerr << "  Record too short, found nShorts, nBytes = " << nShorts
         << ", " << nShorts * sizeof(si16) << endl;
    cerr << "  Expecting mandatory header, nBytes: "
         << nExpected * sizeof(si16) << endl;
    return -1;
  }

  memcpy(&_manHdr, record, sizeof(UF_mandatory_header_t));
  UfRadar::BE_to_mandatory_header(_manHdr);

  if (_debug) {
    UfRadar::print_mandatory_header(cerr, _manHdr);
  }

  int year = _manHdr.year;
  if (year < 1900) {
    if (year < 70) {
      year += 2000;
    } else {
      year += 1900;
    }
  }
  DateTime btime(year,
		 _manHdr.month,
		 _manHdr.day,
		 _manHdr.hour,
		 _manHdr.minute,
		 _manHdr.second);
  _beamTime = btime.utime();

  _azimuth = (double) _manHdr.azimuth / 64.0;
  _elevation = (double) _manHdr.elevation / 64.0;

  _volNum = _manHdr.volume_scan_num;
  _tiltNum = _manHdr.sweep_num;
  
  if (_debug) {
    cerr << "  time: " << btime.strn() << endl;
    cerr << "  azimuth: " << _azimuth << endl;
    cerr << "  elevation: " << _elevation << endl;
    cerr << "  volNum: " << _volNum << endl;
    cerr << "  tiltNum: " << _tiltNum << endl;
  }

  int dheader_offset = (_manHdr.data_header_pos - 1);
  nExpected = dheader_offset + sizeof(UF_data_header_t) / sizeof(si16);
  if (_debug) {
    cerr << "  -->> reading data header, size: "
         << sizeof(UF_data_header_t) << endl;
    cerr << "    data offset bytes: " << dheader_offset * sizeof(si16) << endl;
    cerr << "    record min nbytes: " << nExpected * sizeof(si16) << endl;
  }

  if (nShorts < nExpected) {
    cerr << "ERROR - UfRecord::disassemble" << endl;
    cerr << "  Record too short, found nShorts, nBytes = " << nShorts
         << ", " << nShorts * sizeof(si16) << endl;
    cerr << "  Data offset: " << dheader_offset << endl;
    cerr << "  Expecting nBytes: "
         << nExpected * sizeof(si16) << endl;
    return -1;
  }

  memcpy(&_dataHdr, record + dheader_offset, sizeof(_dataHdr));
  UfRadar::BE_to_data_header(_dataHdr);
  
  if (_debug) {
    UfRadar::print_data_header(cerr, _dataHdr);
  }

  for (int ifield = 0; ifield < _dataHdr.num_ray_fields; ifield++) {

    UF_field_info_t fInfo;
    int info_offset = dheader_offset +
      (sizeof(UF_data_header_t) +
       ifield * sizeof(UF_field_info_t)) / sizeof(si16);
    int nExpected = info_offset + sizeof(UF_field_info_t) / sizeof(si16);
    
    if (_debug) {
      cerr << "  -->> reading field info, size: "
           << sizeof(UF_field_info_t) << endl;
      cerr << "    info offset bytes: " << info_offset * sizeof(si16) << endl;
      cerr << "    record min nbytes: " << nExpected * sizeof(si16) << endl;
    }
    
    if (nShorts < nExpected) {
      cerr << "ERROR - UfRecord::disassemble" << endl;
      cerr << "  Record too short, found nShorts, nBytes = " << nShorts
           << ", " << nShorts * sizeof(si16) << endl;
      cerr << "  Data offset: " << dheader_offset << endl;
      cerr << "  Field number: " << ifield << endl;
      cerr << "  Field info offset: " << info_offset << endl;
      cerr << "  Expecting nBytes: "
           << nExpected * sizeof(si16) << endl;
      return -1;
    }
    
    memcpy(&fInfo, record + info_offset, sizeof(fInfo));
    UfRadar::BE_to_field_info(fInfo);

    if (_debug) {
      UfRadar::print_field_info(cerr, ifield, fInfo);
    }

    _fieldInfo.push_back(fInfo);
    _fieldNames.push_back(UfRadar::label(fInfo.field_name, 2));
  }

  _numGates = 0;
  for (size_t ifield = 0; ifield < _fieldInfo.size(); ifield++) {
    
    UF_field_header_t fhdr;
    int fld_hdr_offset = (_fieldInfo[ifield].field_pos - 1);
    int nExpected = fld_hdr_offset + sizeof(UF_field_header_t) / sizeof(si16);

    if (_debug) {
      cerr << "  -->> reading field header, size: "
           << sizeof(UF_field_header_t) << endl;
      cerr << "    field header offset bytes: "
           << fld_hdr_offset * sizeof(si16) << endl;
      cerr << "    record min nbytes: " << nExpected * sizeof(si16) << endl;
    }
    
    if (nShorts < nExpected) {
      cerr << "ERROR - UfRecord::disassemble" << endl;
      cerr << "  Record too short, found nShorts, nBytes = " << nShorts
           << ", " << nShorts * sizeof(si16) << endl;
      cerr << "  Field number: " << ifield << endl;
      cerr << "  Field header offset: " << fld_hdr_offset << endl;
      cerr << "  Expecting nBytes: "
           << nExpected * sizeof(si16) << endl;
      return -1;
    }

    memcpy(&fhdr, record + fld_hdr_offset, sizeof(fhdr));
    UfRadar::BE_to_field_header(fhdr, _fieldNames[ifield]);
    _fieldHdrs.push_back(fhdr);

    if (_debug) {
      char fieldName[32];
      memset(&fieldName, 0, sizeof(fieldName));
      memcpy(fieldName, _fieldInfo[ifield].field_name, 2);
      UfRadar::print_field_header(cerr, fieldName, ifield, fhdr);
    }

    if (fhdr.num_volumes > _numGates) {
      _numGates = fhdr.num_volumes;
    }
    _numSamples = fhdr.num_samples;
    _polarizationCode = fhdr.polarization;
    _startRangeKm = (double) fhdr.start_range + fhdr.start_center / 1000.0;
    _gateSpacingKm = (double) fhdr.volume_spacing / 1000.0;
    _horizBeamWidthDeg = fhdr.horiz_beam_width / 64.0;
    _vertBeamWidthDeg = fhdr.vert_beam_width / 64.0;
    _wavelengthCm = fhdr.wavelength / 64.0;
    _prf = 1000000.0 / fhdr.pulse_rep_time;

    int data_offset = fhdr.data_pos - 1;
    nExpected = data_offset + fhdr.num_volumes;

    if (_debug) {
      cerr << "  -->> reading field data, len: "
           << fhdr.num_volumes * sizeof(si16) << endl;
      cerr << "    field data offset bytes: "
           << data_offset * sizeof(si16) << endl;
      cerr << "    record min nbytes: " << nExpected * sizeof(si16) << endl;
    }
    
    if ((int) nShorts < nExpected) {
      cerr << "ERROR - UfRecord::disassemble" << endl;
      cerr << "  Record too short, found nShorts, nBytes = " << nShorts
           << ", " << nShorts * sizeof(si16) << endl;
      cerr << "  Field number: " << ifield << endl;
      cerr << "  Field data offset: " << data_offset << endl;
      cerr << "  Expecting nBytes: "
           << nExpected * sizeof(si16) << endl;
      return -1;
    }

    MemBuf buf;
    buf.load(record + data_offset, fhdr.num_volumes * sizeof(si16));
    ByteOrder::swap16(buf.getPtr(), buf.getLen());
    _shortData.push_back(buf);
    
  }

  return 0;

}

////////////////////////////////////////
// Loads up the data for a ray
// Returns 0 on success, -1 on failure

int UfRecord::loadRayData(int volume_num,
                          int sweep_num,
                          bool is_rhi,
                          int ray_num_in_vol,
                          const DateTime &ray_time,
                          double lat,
                          double lon,
                          double alt_m,
                          double el,
                          double az,
                          int n_gates,
                          int n_samples,
                          double start_range_m,
                          double gate_spacing_m,
                          const double *snr_db,
                          const double *vel,
                          const double *width,
                          const double *ht_km)
  
{

  clearData();

  // set field names

  vector<string> _fieldNames;
  _fieldNames.push_back("SN"); // SNR
  _fieldNames.push_back("VR"); // Vel
  _fieldNames.push_back("SW"); // Width
  _fieldNames.push_back("HT"); // Height

  // set field data pointers and scale factors

  vector<const double *> fieldData;
  fieldData.push_back(snr_db);
  fieldData.push_back(vel);
  fieldData.push_back(width);
  fieldData.push_back(ht_km);

  vector<double> scaleFactor;
  scaleFactor.push_back(20.0);
  scaleFactor.push_back(20.0);
  scaleFactor.push_back(100.0);
  scaleFactor.push_back(100.0);

  // set number of fields and gates
  
  int numFieldsOut = _fieldNames.size();
  int numFieldGatesOut = numFieldsOut * n_gates;
  
  _beamTime = ray_time.utime();
  _volNum = volume_num;
  _tiltNum = sweep_num;
  _elevation = el;
  _azimuth = az;
  if (is_rhi) {
    _targetAngle = az;
  } else {
    _targetAngle = el;
  }
  _numGates = n_gates;
  _numSamples = n_samples;

  // Fill UF_MandatoryHeader data

  memcpy(_manHdr.uf_string, "UF", 2);

  // Compute the UF record length --number of 16 bit words:
  // recLen = (sizeof the various headers)/ 2 + dataLen
  // (We divide by 2 since we are looking for length in terms of a 2 byte
  // unit.

  _manHdr.record_length =
    (sizeof(UF_mandatory_header_t) + sizeof(UF_data_header_t)
     + numFieldsOut * sizeof(UF_field_info_t) 
     + numFieldsOut * sizeof(UF_field_header_t)) / sizeof(si16)
    + numFieldGatesOut;
  
  // data header position relative to the start 
  // of the UF buffer of 16bit words or shorts.
  // Note that the first short is position 1.

  int dataHeaderPosition = 1 + sizeof(UF_mandatory_header_t)/2;

  // optional header position: since we dont have one, this
  // will give the position of the first 16 bit word of UF_data_header_t
  // in the uf buffer. Note that the first word is in postion 1.

  _manHdr.optional_header_pos = dataHeaderPosition;
  
  // local header position: since we dont have one, this
  // will give the position of the first 16 bit word of UF_data_header_t
  // in the uf buffer. Note that the first word is in postion 1.;

  _manHdr.local_use_header_pos = dataHeaderPosition;
  
  // data header position
  
  _manHdr.data_header_pos = dataHeaderPosition;
  _manHdr.record_num = ray_num_in_vol;
  _manHdr.volume_scan_num = volume_num;
  _manHdr.ray_num = ray_num_in_vol;
  _manHdr.ray_record_num = 1;

  // radarBeam tilts start at 0, 
  // we start at 1. The UF processing by RSL lib reqires
  // it. I should make it a param.

  _manHdr.sweep_num = _tiltNum + 1;

  // Copy radarName into siteName

  const char *radName = _instrumentName.c_str();
  strncpy(_manHdr.radar_name, radName, 8);
  strncpy(_manHdr.site_name, radName, 8);

  // record latitude:
  // check for location override,
  // convert to degress, minutes, seconds
  // record latitude--convert to degress, minutes, seconds

  _manHdr.lat_degrees = (si16)lat;
  _manHdr.lat_minutes = (si16)((lat - _manHdr.lat_degrees) * 60);
  _manHdr.lat_seconds = (si16)(((lat - _manHdr.lat_degrees) * 60 -
			       _manHdr.lat_minutes) * 60)*64;
  
  // record longitude--convert to degress, minutes, seconds

  _manHdr.lon_degrees = (si16)lon;
  _manHdr.lon_minutes = (si16)((lon - _manHdr.lon_degrees) * 60);
  _manHdr.lon_seconds = (si16)(((lon - _manHdr.lon_degrees) * 60 -
			       _manHdr.lon_minutes) * 60)* 64;
  
  // antennaAlt in meters.
  
  _manHdr.antenna_height = (si16)(alt_m + .5);
  
  // Fill year,month, day, hour, min, seconds
  // in the UF_mandatory_header_t struct.
  // (year is just the last two digits of the year.)
  
  _manHdr.year = ray_time.getYear() - (ray_time.getYear()/100)*100;
  _manHdr.day = ray_time.getDay();
  _manHdr.month = ray_time.getMonth();
  _manHdr.hour  = ray_time.getHour();
  _manHdr.minute = ray_time.getMin();
  _manHdr.second = ray_time.getSec();

  memcpy(_manHdr.time_zone, "UT", 2);

  // azimuth * 64

  _manHdr.azimuth = (si16)(az * 64 + .5);
  
  // elevation * 64

  _manHdr.elevation = (si16)(el * 64 + .5 );

  if (is_rhi) {
    _manHdr.sweep_mode = 3;
  } else {
    _manHdr.sweep_mode = 1;
  }
  
  // fixed_angle is the target elevation (X 64).

  _manHdr.fixed_angle = (si16)(_targetAngle * 64 + .5 );

  // sweepRate(degrees/sec  * 64) 

  _manHdr.sweep_rate = UF_NO_DATA;

  // Find file generation time(UTC):
  
  time_t now = time(NULL);
  DateTime currentTime(now);
  currentTime.set( currentTime.utime());
  _manHdr.gen_year = currentTime.getYear() - (currentTime.getYear()/100)*100;
  _manHdr.gen_month = currentTime.getMonth();
  _manHdr.gen_day = currentTime.getDay();

  // Facility. Dont have...
  strncpy(_manHdr.gen_facility, "none", 8);
  
  _manHdr.missing_data_val = UF_NO_DATA;

  // Fill UF_data_header_t uf_dh

  _dataHdr.num_ray_fields = numFieldsOut;
  _dataHdr.num_ray_records = 1;
  _dataHdr.num_record_fields = numFieldsOut;

  // Fill in fldNamesPositions with field names and field header
  // postions.
  
  _fieldInfo.clear();

  for (int ifld = 0; ifld < numFieldsOut; ifld++) {

    UF_field_info_t info;
    
    // copy in the UF field name
    
    string fieldName = _fieldNames[ifld];
    int ufNameLen = fieldName.size();
    if (ufNameLen > 2) {
      ufNameLen = 2;
    }
    memset(&info, 0, sizeof(info));
    memcpy(info.field_name, fieldName.c_str(), ufNameLen);
    
    // Put in the field header position:
    // Recall the UF structure: Mandatory Header  
    //                          OptLocal Header
    //                          Local Header
    //                          Data Header
    //                          n * (Field Name, 
    //                               Data Position)
    //                          n * Field Header ...
    
    info.field_pos =
      _manHdr.data_header_pos + sizeof(UF_data_header_t)/2 +
      numFieldsOut * sizeof(UF_field_info_t)/2 +
      ifld * sizeof(UF_field_header_t)/2;
    
    _fieldInfo.push_back(info);
    
  }

  // Create field headers and data

  _fieldHdrs.clear();
  _shortData.clear();

  //
  // Fill UF_field_header_t's and convert float data to shorts with 
  // appropriate scale factor.
  //

  for ( int ifld = 0; ifld < numFieldsOut; ifld++ ) {
    
    // Enter data position. 
    // Recall the UF structure: Mandatory Header  
    //                          OptLocal Header
    //                          Local Header
    //                          Data Header
    //                          n * (Field Name, 
    //                               Data Position)
    //                          n * Field Headers
    //                          data( n fields * number of gates/field)
    
    UF_field_header_t fhdr;
    
    fhdr.data_pos = _manHdr.data_header_pos + sizeof(UF_data_header_t)/2 +
      numFieldsOut * sizeof(UF_field_info_t)/2 +
      numFieldsOut * sizeof(UF_field_header_t)/2 +
      ifld * n_gates;
				     
    // Set scaling factor such that
    //   shortValue/scaleFactor = meteorlogical units.
    // Thus we need shortVal = fabs(floatVal) * scaleFactor < 32768

    // default value

    // fhdr.scale_factor = scaleFactor[ifld];
    
    // compute scale dynamically
    // We set scale_factor = 32000/(max fabs(floatVal))
    
    double max = 0;
    for (int igate = 0; igate < n_gates; igate++) {
      float absval = fabs(fieldData[ifld][igate]);
      if( absval >  max) {
        max = absval;
      }
    }
    if(max > 0) {
      fhdr.scale_factor = (si16)(32000/max);
    } else {
      fhdr.scale_factor = 25.0;
    }
    
    // range to first gate (km)

    _startRangeKm = start_range_m / 1000.0;
    fhdr.start_range = (si16)(_startRangeKm + .5);
    
    // adjustment to center of first gate.
    
    fhdr.start_center = 0;

    // gate spacing (m) 
    
    _gateSpacingKm = gate_spacing_m / 1000.0;
    fhdr.volume_spacing = (si16)(_gateSpacingKm * 1000.0  + 0.5);

    fhdr.num_volumes = n_gates;
    fhdr.volume_depth =  UF_NO_DATA;

    // horizontal beam width * 64

    fhdr.horiz_beam_width = (si16)( _horizBeamWidthDeg * 64 + .5);

    // vertical beam width * 64
    
    fhdr.vert_beam_width = (si16)(_vertBeamWidthDeg * 64 + .5);
    
    fhdr.receiver_bandwidth = UF_NO_DATA;
    fhdr.polarization =  UF_NO_DATA;
    _polarizationCode = UF_NO_DATA;

    // wavelength in cm. * 64 
    
    fhdr.wavelength =  (si16) floor(_wavelengthCm * 64 + .5);
    fhdr.num_samples =  _numSamples;
    
    memcpy(fhdr.threshold_field, "  ", 2);
    fhdr.threshold_val = UF_NO_DATA;
    
    // Scale is used to scale noiserPower, receiverGain, peakPower
    // and antennaGain from floats to shorts. So first find
    // the max of the absolute value of the four varibales.
    // Scale is such that short/scale = float.
    
    double maxPowerGain = 0.0;
    if (fabs(_receiverGainDb) >  maxPowerGain) {
      maxPowerGain = fabs(_receiverGainDb);
    }
    if (fabs(_peakPowerDbm) >  maxPowerGain) {
      maxPowerGain = fabs(_peakPowerDbm);
    }
    if ( fabs(_antennaGainDb) > maxPowerGain) {
      maxPowerGain = fabs(_antennaGainDb);
    }
    if ( fabs(_noisePowerDbm) > maxPowerGain) {
      maxPowerGain = fabs(_noisePowerDbm);
    }
    fhdr.scale = (si16)(32000/maxPowerGain);
    
    //
    // What is this?
    //
    memcpy(fhdr.edit_code, "NO", 2);
    
    //
    // pulseRep in micro seconds.
    //
    if(_prf > 0) {
      fhdr.pulse_rep_time = (si16)(1.0/_prf * 1000000.0 + .5);
    } else {
      fhdr.pulse_rep_time = UF_NO_DATA;
    }
    
    fhdr.volume_bits = UF_NO_DATA;

    if (_fieldInfo[ifld].field_name[0] == 'V') {

      // nyquist velocity
      fhdr.word20.nyquist_vel = (si16)(fhdr.scale_factor * 
				       _nyquistVel + .5);
      
      // flag is set to 1 if velocity is bad
      fhdr.word21.fl_string[0] = 0;
      fhdr.word21.fl_string[1] = 0;

    } else {

      // RadarConstant is such that dB(Z) =
      //   (RC + data)/scale + 20 * log(range in km).

      fhdr.word20.radar_const = (si16)_radarConstant;

      // nosiePower dB(mW)* scale
      
      fhdr.word21.noise_power = (si16)(_noisePowerDbm * fhdr.scale + .5 );
      
    }
    
    // receiverGain(dB) * scale
    //  _receiverGain has units dB.
    
    fhdr.receiver_gain = (si16)( _receiverGainDb * fhdr.scale + .5 );
    
    // peakPower(dB(mW)) * scale
    
    fhdr.peak_power = (si16)(_peakPowerDbm * fhdr.scale + .5);
    
    // antennaGain(dB) * scale
    // _antennaGain has units dB.
    
    fhdr.antenna_gain = UF_NO_DATA;
    fhdr.pulse_duration = UF_NO_DATA;
    
    // Convert float field data to short data.
    // Note we store short data field by field.
    // (ifld == field number)
    
    MemBuf shortDataBuf;
    shortDataBuf.reserve(n_gates * sizeof(si16));
    si16 *shortData = (si16 *) shortDataBuf.getPtr();
    
    for (int igate = 0; igate < n_gates; igate++) {
      double val = fieldData[ifld][igate];
      if (val < -9990.0) {
        shortData[igate] = UF_NO_DATA;
      } else {
        shortData[igate] = (si16) (val * fhdr.scale_factor);
      }
    }
    
    _fieldHdrs.push_back(fhdr);
    _shortData.push_back(shortDataBuf);
      
  } // ifld loop
  
  return 0;

}

//////////////////////////////////////////////////////////////
// Write record to open file
// Returns 0 on success, -1 on failure

int UfRecord::write(FILE *fp)
  
{

  // fortran record length
  
  si32 fortRecLen = _manHdr.record_length * sizeof(si16);
  ByteOrder::swap32(&fortRecLen, sizeof(si32));

  // mandatory and data headers

  UF_mandatory_header_t _manHdrBE = _manHdr;
  UfRadar::BE_from_mandatory_header(_manHdrBE);

  UF_data_header_t _dataHdrBE = _dataHdr;
  UfRadar::BE_from_data_header(_dataHdrBE);

  // load up field info, headers and data
  
  MemBuf _fieldInfoBuf;
  MemBuf fieldHdrBuf;
  MemBuf shortDataBuf;

  for(int ifld = 0; ifld < (int) _fieldHdrs.size(); ifld++) {
    
    string fieldName = UfRadar::label(_fieldInfo[ifld].field_name, 2);
    
    UF_field_info_t infoBE = _fieldInfo[ifld];
    UfRadar::BE_from_field_info(infoBE);
    _fieldInfoBuf.add(&infoBE, sizeof(UF_field_info_t));
    
    UF_field_header_t hdrBE = _fieldHdrs[ifld];
    UfRadar::BE_from_field_header(hdrBE, fieldName);
    fieldHdrBuf.add(&hdrBE, sizeof(UF_field_header_t));
    
    shortDataBuf.add(_shortData[ifld].getPtr(), _shortData[ifld].getLen());
    
  } // ifld

  // swap data
  
  ByteOrder::swap16(shortDataBuf.getPtr(), shortDataBuf.getLen());
  
  // Recall the UF structure for n fields: 
  //                          FORTRAN record len
  //                          Mandatory Header  
  //                          OptLocal Header
  //                          Local Header
  //                          Data Header
  //                          n * (Field Name, 
  //                                       Data Position)
  //                          n * Field Headers
  //                          data
  //                          FORTRAN record len

  if (fwrite(&fortRecLen, sizeof(si32), 1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - Reformat::writeRecord" << endl;
    cerr << "  Writing leading FORTRAN record length" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (fwrite(&_manHdrBE, sizeof(_manHdrBE), 1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing mandatory header" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (fwrite(&_dataHdrBE, sizeof(_dataHdrBE),  1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing data header" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (fwrite(_fieldInfoBuf.getPtr(), 1, _fieldInfoBuf.getLen(), fp) !=
      _fieldInfoBuf.getLen()) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing field info" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (fwrite(fieldHdrBuf.getPtr(), 1, fieldHdrBuf.getLen(), fp) !=
      fieldHdrBuf.getLen()) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing field headers" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (fwrite(shortDataBuf.getPtr(), 1, shortDataBuf.getLen(), fp) !=
      shortDataBuf.getLen()) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing field data" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (fwrite(&fortRecLen, sizeof(si32), 1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing trailing FORTRAN record length" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////
// check for all missing data
// returns true if all data is missing

bool UfRecord::allDataMissing()

{

  for (int ifld = 0; ifld < (int) _shortData.size(); ifld++) {

    const si16 *shortData = (const si16 *) _shortData[ifld].getPtr();
    int nData = _shortData[ifld].getLen() / sizeof(si16);
    
    for (int ii = 0; ii < nData; ii++) {
      if (shortData[ii] != UF_NO_DATA) {
	return false;
      }
    } // ii

  } // ifld

  return true;

}

///////////////
// print record
//

void UfRecord::print(ostream &out,
		     bool print_headers,
		     bool print_data)
  
{

  if (print_headers) {

    UfRadar::print_mandatory_header(out, _manHdr);
    UfRadar::print_data_header(out, _dataHdr);
    printDerived(out);
    
  }
  
  for (size_t ii = 0; ii < _fieldInfo.size(); ii++) {
    if (print_headers) {
      UfRadar::print_field_info(out, (int) ii, _fieldInfo[ii]);
      UfRadar::print_field_header(out, _fieldNames[ii], (int) ii, _fieldHdrs[ii]);
    }
    if (print_data) {
      UfRadar::print_field_data(out, _fieldNames[ii], (int) ii,
				_fieldHdrs[ii].num_volumes,
				_fieldHdrs[ii].scale_factor,
				_manHdr.missing_data_val,
				(si16 *) _shortData[ii].getPtr());
    }
  } // ii

}

/////////////////////
// print derived data
//

void UfRecord::printDerived(ostream &out) const

{

  out << "***********************************" << endl;
  out << "Values derived from UF file:" << endl;
  out << "    maxGates: " << _numGates << endl;
  out << "    numSamples: " << _numSamples << endl;
  out << "    polarizationCode: " << _polarizationCode << endl;
  out << "    startRangeKm: " << _startRangeKm << endl;
  out << "    gateSpacingKm: " << _gateSpacingKm << endl;
  out << "    radarConstant: " << _radarConstant << endl;
  out << "    noisePowerDbm: " << _noisePowerDbm << endl;
  out << "    receiverGainDb: " << _receiverGainDb << endl;
  out << "    peakPowerDbm: " << _peakPowerDbm << endl;
  out << "    antennaGainDb: " << _antennaGainDb << endl;
  out << "    pulseWidthUs: " << _pulseWidthUs << endl;
  out << "    horizBeamWidthDeg: " << _horizBeamWidthDeg << endl;
  out << "    vertBeamWidthDeg: " << _vertBeamWidthDeg << endl;
  out << "    wavelengthCm: " << _wavelengthCm << endl;
  out << "    prf: " << _prf << endl;
  out << "    nyquistVel: " << _nyquistVel << endl;
  out << "    volNum: " << _volNum << endl;
  out << "    tiltNum: " << _tiltNum << endl;
  out << "    targetAngle: " << _targetAngle << endl;
  out << "    elevation: " << _elevation << endl;
  out << "    azimuth: " << _azimuth << endl;
  out << "    time: " << DateTime::str(_beamTime) << endl;
  out << "***********************************" << endl;

}


