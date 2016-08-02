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

#include <rapformats/UfRecord.hh>
#include <rapformats/DsRadarMsg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/mem.h>
#include <dataport/bigend.h>
#include <cmath>
#include <cerrno>
using namespace std;

///////////////
// constructor

UfRecord::UfRecord()

{
  clear();
  _debug = false;
}

///////////////
// destructor

UfRecord::~UfRecord()

{
  free();
}

///////////////////////
// clear 

void UfRecord::clear()

{

  MEM_zero(manHdr);
  MEM_zero(dataHdr);
  fieldInfo.clear();
  fieldHdrs.clear();
  fieldNames.clear();
  fieldData.clear();

  maxGates = 0;
  numSamples = 0;
  polarizationCode = 0;
  startRange = -9999;
  gateSpacing = -9999;
  radarConstant = -9999;
  noisePower = -9999;
  receiverGain = -9999;
  peakPower = -9999;
  antennaGain = -9999;
  pulseWidth = -9999;
  horizBeamWidth = -9999;
  vertBeamWidth = -9999;
  wavelength = -9999;
  prf = -9999;
  nyquistVel = -9999;

  volNum = 0;
  tiltNum = 0;
  targetAngle = -9999;

  elevation = -9999;
  azimuth = -9999;
  beamTime = 0;

}

///////////////////////
// free up memory

void UfRecord::free()

{

  fieldInfo.clear();
  fieldHdrs.clear();
  fieldNames.clear();
  fieldData.clear();

}

///////////////////////////////////////////////
// disassemble the object from a raw UF record
// Returns 0 on success, -1 on failure

int UfRecord::disassemble(const void *buf, int nBytes)

{

  clear();

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

  memcpy(&manHdr, record, sizeof(UF_mandatory_header_t));
  UfRadar::BE_to_mandatory_header(manHdr);

  if (_debug) {
    UfRadar::print_mandatory_header(cerr, manHdr);
  }

  int year = manHdr.year;
  if (year < 1900) {
    if (year < 70) {
      year += 2000;
    } else {
      year += 1900;
    }
  }
  DateTime btime(year,
		 manHdr.month,
		 manHdr.day,
		 manHdr.hour,
		 manHdr.minute,
		 manHdr.second);
  beamTime = btime.utime();

  azimuth = (double) manHdr.azimuth / 64.0;
  elevation = (double) manHdr.elevation / 64.0;

  volNum = manHdr.volume_scan_num;
  tiltNum = manHdr.sweep_num;
  
  if (_debug) {
    cerr << "  time: " << btime.strn() << endl;
    cerr << "  azimuth: " << azimuth << endl;
    cerr << "  elevation: " << elevation << endl;
    cerr << "  volNum: " <<volNum << endl;
    cerr << "  tiltNum: " << tiltNum << endl;
  }

  int dheader_offset = (manHdr.data_header_pos - 1);
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

  memcpy(&dataHdr, record + dheader_offset, sizeof(dataHdr));
  UfRadar::BE_to_data_header(dataHdr);
  
  if (_debug) {
    UfRadar::print_data_header(cerr, dataHdr);
  }

  for (int ifield = 0; ifield < dataHdr.num_ray_fields; ifield++) {

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

    fieldInfo.push_back(fInfo);
    fieldNames.push_back(UfRadar::label(fInfo.field_name, 2));
  }

  maxGates = 0;
  for (size_t ifield = 0; ifield < fieldInfo.size(); ifield++) {
    
    UF_field_header_t fhdr;
    int fld_hdr_offset = (fieldInfo[ifield].field_pos - 1);
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
    UfRadar::BE_to_field_header(fhdr, fieldNames[ifield]);
    fieldHdrs.push_back(fhdr);

    if (_debug) {
      char fieldName[32];
      MEM_zero(fieldName);
      memcpy(fieldName, fieldInfo[ifield].field_name, 2);
      UfRadar::print_field_header(cerr, fieldName, ifield, fhdr);
    }

    if (fhdr.num_volumes > maxGates) {
      maxGates = fhdr.num_volumes;
    }
    numSamples = fhdr.num_samples;
    polarizationCode = fhdr.polarization;
    startRange = (double) fhdr.start_range + fhdr.start_center / 1000.0;
    gateSpacing = (double) fhdr.volume_spacing / 1000.0;
    horizBeamWidth = fhdr.horiz_beam_width / 64.0;
    vertBeamWidth = fhdr.vert_beam_width / 64.0;
    wavelength = fhdr.wavelength / 64.0;
    prf = 1000000.0 / fhdr.pulse_rep_time;

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
    BE_to_array_16(buf.getPtr(), buf.getLen());
    fieldData.push_back(buf);
    
  }

  return 0;

}

////////////////////////////////////////
// Loads up the object from a DsRadarMsg
// Returns 0 on success, -1 on failure

int UfRecord::loadFromDsRadarMsg(const DsRadarMsg &radarMsg,
				 int rayNumInVol,
				 const vector<field_tranlation_t> &fieldTrans)

{
  
  clear();

  // set params, flags , and radar beam data

  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  // const DsRadarCalib &radarCalib = radarMsg.getRadarCalib();
  // const DsRadarFlags &radarFlags = radarMsg.getRadarFlags();
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  const vector<DsFieldParams *> fieldParams = radarMsg.getFieldParams();

  int numFieldsIn = (int) fieldParams.size();
  int numFieldsOut = (int) fieldTrans.size();
  
  int numGates = radarParams.getNumGates();
  int numFieldGatesIn = numFieldsIn * numGates;
  int numFieldGatesOut = numFieldsOut * numGates;
  
  beamTime = radarBeam.dataTime;
  volNum = radarBeam.volumeNum;
  tiltNum = radarBeam.tiltNum;
  elevation = radarBeam.elevation;
  azimuth = radarBeam.azimuth;
  if (radarBeam.scanMode == DS_RADAR_RHI_MODE) {
    targetAngle = radarBeam.targetAz;
  } else {
    targetAngle = radarBeam.targetElev;
  }

  // Fill UF_MandatoryHeader data

  memcpy(manHdr.uf_string, "UF", 2);

  // Compute the UF record length --number of 16 bit words:
  // recLen = (sizeof the various headers)/ 2 + dataLen
  // (We divide by 2 since we are looking for length in terms of a 2 byte
  // unit.

  manHdr.record_length =
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

  manHdr.optional_header_pos = dataHeaderPosition;
  
  // local header position: since we dont have one, this
  // will give the position of the first 16 bit word of UF_data_header_t
  // in the uf buffer. Note that the first word is in postion 1.;

  manHdr.local_use_header_pos = dataHeaderPosition;
  
  // data header position
  
  manHdr.data_header_pos = dataHeaderPosition;
  manHdr.record_num = rayNumInVol;
  manHdr.volume_scan_num = radarBeam.volumeNum;
  manHdr.ray_num = rayNumInVol;
  manHdr.ray_record_num = 1;

  // radarBeam tilts start at 0, 
  // we start at 1. The UF processing by RSL lib reqires
  // it. I should make it a param.

  manHdr.sweep_num = radarBeam.tiltNum + 1;

  // Copy radarName into siteName

  const char *radName = radarParams.radarName.c_str();
  strncpy(manHdr.radar_name, radName, 8);
  strncpy(manHdr.site_name, radName, 8);

  // record latitude:
  // check for location override,
  // convert to degress, minutes, seconds

  double latitude = radarParams.latitude;
  double longitude = radarParams.longitude;
  double altitude = radarParams.altitude;
  
  // record latitude--convert to degress, minutes, seconds

  manHdr.lat_degrees = (si16)latitude;
  manHdr.lat_minutes = (si16)((latitude - manHdr.lat_degrees) * 60);
  manHdr.lat_seconds = (si16)(((latitude - manHdr.lat_degrees) * 60 -
			       manHdr.lat_minutes) * 60)*64;
  
  // record longitude--convert to degress, minutes, seconds

  manHdr.lon_degrees = (si16)longitude;
  manHdr.lon_minutes = (si16)((longitude - manHdr.lon_degrees) * 60);
  manHdr.lon_seconds = (si16)(((longitude - manHdr.lon_degrees) * 60 -
			       manHdr.lon_minutes) * 60)* 64;

  // antennaAlt in meters.
  // altiude has units of km. 

  manHdr.antenna_height = (si16)(altitude * 1000 + .5);
  
  // Fill year,month, day, hour, min, seconds
  // in the UF_mandatory_header_t struct.
  // (year is just the last two digits of the year.)
  
  DateTime btime(beamTime);
  manHdr.year = btime.getYear() - (btime.getYear()/100)*100;
  manHdr.day = btime.getDay();
  manHdr.month = btime.getMonth();
  manHdr.hour  = btime.getHour();
  manHdr.minute = btime.getMin();
  manHdr.second = btime.getSec();

  memcpy(manHdr.time_zone, "UT", 2);

  // azimuth * 64

  manHdr.azimuth = (si16)(radarBeam.azimuth * 64 + .5);
  
  // elevation * 64

  manHdr.elevation = (si16)(radarBeam.elevation * 64 + .5 );

  manHdr.sweep_mode = radarParams.scanMode;
  
  // fixed_angle is the target elevation (X 64).

  if (radarBeam.scanMode == DS_RADAR_RHI_MODE) {
    double fixedAng = radarBeam.targetAz;
    if (fixedAng < -360) {
      fixedAng = radarBeam.azimuth;
    }
    manHdr.fixed_angle =  (si16)(fixedAng * 64 + .5 );
  } else {
    double fixedAng = radarBeam.targetElev;
    if (fixedAng < -360) {
      fixedAng = radarBeam.elevation;
    }
    manHdr.fixed_angle =  (si16)(fixedAng * 64 + .5 );
  }
  
  // sweepRate(degrees/sec  * 64) 

  manHdr.sweep_rate = UF_NO_DATA;

  // Find file generation time(UTC):
  
  DateTime currentTime(time(0));
  currentTime.set( currentTime.utime());
  manHdr.gen_year = currentTime.getYear() - (currentTime.getYear()/100)*100;
  manHdr.gen_month = currentTime.getMonth();
  manHdr.gen_day = currentTime.getDay();

  // Facility. Dont have...
  strncpy(manHdr.gen_facility, "none", 8);
  
  manHdr.missing_data_val = UF_NO_DATA;

  // Fill UF_data_header_t uf_dh

  dataHdr.num_ray_fields = numFieldsOut;
  dataHdr.num_ray_records = 1;
  dataHdr.num_record_fields = numFieldsOut;

  // compute the indices of each output field in the Dsr beam

  vector<int> dsrFieldIndex;
  for (int ifld = 0; ifld < numFieldsOut; ifld++) {
    string inputName = fieldTrans[ifld].input_name;
    int index = -1;
    for (int ii = 0; ii < (int) fieldParams.size(); ii++) {
      if (inputName == fieldParams[ii]->name) {
	index = ii;
	break;
      }
    }
    dsrFieldIndex.push_back(index);
  } // ifld
  
  // Fill in fldNamesPositions with field names and field header
  // postions.
  
  fieldInfo.clear();

  for (int ifld = 0; ifld < numFieldsOut; ifld++) {

    UF_field_info_t info;
    
    // copy in the UF field name
    
    const field_tranlation_t &fld = fieldTrans[ifld];
    int ufNameLen = fld.uf_name.size();
    if (ufNameLen > 2) {
      ufNameLen = 2;
    }
    MEM_zero(info);
    memcpy(info.field_name, fld.uf_name.c_str(), ufNameLen);
    
    // Put in the field header position:
    // Recall the UF structure: Mandatory Header  
    //                          OptLocal Header
    //                          Local Header
    //                          Data Header
    //                          n * (Field Name, 
    //                               Data Position)
    //                          n * Field Header ...
    
    info.field_pos =
      manHdr.data_header_pos + sizeof(UF_data_header_t)/2 +
      numFieldsOut * sizeof(UF_field_info_t)/2 +
      ifld * sizeof(UF_field_header_t)/2;

    fieldInfo.push_back(info);
    
  }

  // Create field headers and data

  fieldHdrs.clear();
  fieldData.clear();

  // Allocate space to hold radarBeam data
  // as floats and shorts.
  
  TaArray<float> floatData_;
  float *floatData = floatData_.alloc(numFieldGatesOut);
  memset(floatData, 0, sizeof(float)*numFieldGatesOut);
    
  // Transform byte data to float data.
  // float = byte * scale + bias.
  // Byte data is stored gate by gate with 
  // all field values at each gate. 

  maxGates = 0;

  for ( int ifld = 0; ifld < numFieldsOut; ifld++ ) {
    
    // get the Dsr field index
    int dsrIndex = dsrFieldIndex[ifld];
    if (dsrIndex < 0) {
      // field missing
      continue;
    }
    
    const DsFieldParams &fld = *fieldParams[dsrIndex];
    float scale = fld.scale;
    float bias = fld.bias;
    int missingInput = fld.missingDataValue;
      
    // Jump gate to gate and record float 
    // data for particular field.
    // Note that float data is stored 
    // field by field rather than gate by gate.
    
    if (radarBeam.byteWidth == 1) {
      ui08 *bData = (ui08 *) radarBeam.getData();
      for(int kk = dsrIndex, ll = 0 ; kk < numFieldGatesIn;
          kk += numFieldsIn, ll++) {
	if (bData[kk] == missingInput) {
	  floatData[ifld * numGates + ll] = -9999.0;
	} else {
	  floatData[ifld * numGates + ll] = bData[kk] *  scale + bias;
	}
      }
    } else if (radarBeam.byteWidth == 2) {
      ui16 *sData = (ui16 *) radarBeam.getData();
      for(int kk = dsrIndex, ll = 0 ; kk < numFieldGatesIn;
          kk += numFieldsIn, ll++) {
	if (sData[kk] == missingInput) {
	  floatData[ifld * numGates + ll] = -9999.0;
	} else {
	  floatData[ifld * numGates + ll] = sData[kk] *  scale + bias;
	}
      }
    } else {
      fl32 *fData = (fl32 *) radarBeam.getData();
      for(int kk = dsrIndex, ll = 0 ; kk < numFieldGatesIn;
          kk += numFieldsIn, ll++) {
	if (fData[kk] == (fl32) missingInput) {
	  floatData[ifld * numGates + ll] = -9999.0;
	} else {
	  floatData[ifld * numGates + ll] = fData[kk];
	}
      }
    }
    
  }

  //
  // Fill UF_field_header_t's and convert float data to shorts with 
  // appropriate scale factor.
  //

  for ( int ifld = 0; ifld < numFieldsOut; ifld++ ) {
    
    // get the Dsr field index
    int dsrIndex = dsrFieldIndex[ifld];

    const field_tranlation_t &fld = fieldTrans[ifld];

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

    fhdr.data_pos = manHdr.data_header_pos + sizeof(UF_data_header_t)/2 +
      numFieldsOut * sizeof(UF_field_info_t)/2 +
      numFieldsOut * sizeof(UF_field_header_t)/2 +
      ifld * numGates;
				     
    // Set scaling factor such that
    //   shortValue/scaleFactor = meteorlogical units.
    // Thus we need shortVal = fabs(floatVal) * scaleFactor < 32768

    // default value
    fhdr.scale_factor = 100;
    
    if (fld.scale > 0) {
      
      fhdr.scale_factor = fld.scale;
      
    } else {
      
      // compute scale dynamically
      // We set scale_factor = 32000/(max fabs(floatVal))
      
      double max = 0;
      for ( int igate = 0; igate < numGates; igate++) {
	float absval = fabs( floatData[ ifld *  numGates + igate]);
	if( absval >  max)
	  max = absval;
      }
      if(max > 0) {
	fhdr.scale_factor = (si16)(32000/max);
      }
	
    }
      
    //
    // range to first gate (km)
    // radarParams.startRange has units of km.
    //
    fhdr.start_range = (si16)(radarParams.startRange + .5);
    startRange = radarParams.startRange;
    //
    // 
    // adjustment to center of first gate.
    //
    fhdr.start_center = 0;

    //
    // gate spacing (m) 
    // radarParams.gateSpacing has units of km.
    //
    fhdr.volume_spacing = (si16)(radarParams.gateSpacing * 1000  +.5 );
    gateSpacing = radarParams.gateSpacing;

    fhdr.num_volumes = radarParams.numGates;
    fhdr.volume_depth =  UF_NO_DATA;

    if (radarParams.numGates > maxGates) {
      maxGates = radarParams.numGates;
    }
    
    //
    // horizontal beam width * 64
    //
    fhdr.horiz_beam_width = (si16)(radarParams.horizBeamWidth  * 64 + .5);
    horizBeamWidth = radarParams.horizBeamWidth;

    //
    // vertical beam width * 64
    //
    fhdr.vert_beam_width = (si16)(radarParams.vertBeamWidth * 64 + .5);
    vertBeamWidth = radarParams.vertBeamWidth;
    
    fhdr.receiver_bandwidth = UF_NO_DATA;
    
    fhdr.polarization =  radarParams.polarization;
    polarizationCode = fhdr.polarization;
    
    //
    // wavelength in cm. * 64 
    // radarParams.wavelength has units of cm.
    //
    fhdr.wavelength =  (si16) floor(radarParams.wavelength * 64 + .5);
    wavelength = radarParams.wavelength;

    fhdr.num_samples =  radarParams.samplesPerBeam;
    numSamples = radarParams.samplesPerBeam;
    
    memcpy(fhdr.threshold_field, "  ", 2);
    
    fhdr.threshold_val = UF_NO_DATA;
    
    //
    // Scale is used to scale noiserPower, receiverGain, peakPower
    // and antennaGain from floats to shorts. So first find
    // the max of the absolute value of the four varibales.
    // Scale is such that short/scale = float.
    //
    
    double maxPowerGain = 0.0;
    if (fabs(radarParams.receiverGain) >  maxPowerGain) {
      maxPowerGain = fabs(radarParams.receiverGain);
    }
    
    //
    // Peak power in dBm(mW)
    //
    double peakPowerDbm = 0.0;
    if (radarParams.xmitPeakPower > 0) {
      peakPowerDbm = 10.0 * log10(radarParams.xmitPeakPower * 1000);
    }
    
    if (fabs(peakPowerDbm) > maxPowerGain) {
      maxPowerGain =  fabs(peakPowerDbm);
    }
    
    if ( fabs(radarParams.antennaGain) > maxPowerGain) {
      maxPowerGain = fabs(radarParams.antennaGain);
    }

    fhdr.scale = (si16)(32000/maxPowerGain);
    
    //
    // What is this?
    //
    memcpy(fhdr.edit_code, "NO", 2);
    
    //
    // pulseRep in micro seconds.
    //
    if( radarParams.pulseRepFreq != 0) {
      fhdr.pulse_rep_time = (si16)(1/radarParams.pulseRepFreq * 1000000 + .5);
    } else {
      fhdr.pulse_rep_time = UF_NO_DATA;
    }
    prf = radarParams.pulseRepFreq;
    
    fhdr.volume_bits = UF_NO_DATA;

    bool isVel = false;
    if (fieldInfo[ifld].field_name[0] == 'V') {
      isVel = true;
    }
    nyquistVel = radarParams.unambigVelocity;

    if (isVel) {

      // nyquist velocity
      fhdr.word20.nyquist_vel = (si16)(fhdr.scale_factor * 
				       radarParams.unambigVelocity + .5);
      
      // flag is set to 1 if velocity is bad
      fhdr.word21.fl_string[0] = 0;
      fhdr.word21.fl_string[1] = 0;

    } else {
      //
      // RadarConstant is such that dB(Z) = (RC + data)/scale + 20 * log(range in km).
      // check this...
      // 
      fhdr.word20.radar_const = (si16)radarParams.radarConstant;
      radarConstant = radarParams.radarConstant;
      //
      // nosiePower dB(mW)* scale
      //
      fhdr.word21.noise_power = UF_NO_DATA;
      noisePower = -9999.0;

    }
    
    //
    // receiverGain(dB) * scale
    //  radarParams.receiverGain has units dB.
    //
    fhdr.receiver_gain = (si16)( radarParams.receiverGain * 
				 fhdr.scale + .5 );
    receiverGain = radarParams.receiverGain;
    
    //
    // peakPower(dB(mW)) * scale
    //
    // 
    fhdr.peak_power = (si16)(peakPowerDbm * fhdr.scale + .5);
    peakPower = peakPowerDbm;
    
    //
    // antennaGain(dB) * scale
    // radarParams.antennaGain has units dB.
    //
    fhdr.antenna_gain = (si16)( radarParams.antennaGain * 
				fhdr.scale + .5 );
    
    fhdr.pulse_duration = (si16)( radarParams.pulseWidth * 64.0 + .5 );
    
    // Convert float field data to short data.
    // Note we store short data field by field.
    // (ifld == field number)
    //

    MemBuf shortDataBuf;
    shortDataBuf.reserve(numGates * sizeof(si16));
    si16 *shortData = (si16 *) shortDataBuf.getPtr();

    if (dsrIndex < 0) {
      
      // field missing
      for (int igate = 0; igate < numGates; igate++) {
	shortData[igate] = UF_NO_DATA;
      }
      
    } else {
      
      for (int igate = 0; igate < numGates; igate++) {
	int ii = ifld * numGates + igate;
	if (floatData[ii] == -9999.0) {
	  shortData[igate] = UF_NO_DATA;
	} else {
	  shortData[igate] = (si16) (floatData[ii] * fhdr.scale_factor);
	}
      }

    }

    fieldHdrs.push_back(fhdr);
    fieldData.push_back(shortDataBuf);
      
  } // ifld loop
  
  return 0;

}

//////////////////////////////////////////////////////////////
// Write record to open file
// Returns 0 on success, -1 on failure

int UfRecord::write(FILE *fp,
		    bool littleEndian /* = false */)
  
{

  // byte swapping

  // reverse byte swapping sense if little-endian required
  
  if (littleEndian) {
    BE_reverse();
  }

  // fortran record length

  si32 fortRecLen = BE_from_si32(manHdr.record_length * sizeof(si16));

  // mandatory and data headers

  UF_mandatory_header_t manHdrBE = manHdr;
  UfRadar::BE_from_mandatory_header(manHdrBE);

  UF_data_header_t dataHdrBE = dataHdr;
  UfRadar::BE_from_data_header(dataHdrBE);

  // load up field info, headers and data
  
  MemBuf fieldInfoBuf;
  MemBuf fieldHdrBuf;
  MemBuf shortDataBuf;

  for(int ifld = 0; ifld < (int) fieldHdrs.size(); ifld++) {
    
    string fieldName = UfRadar::label(fieldInfo[ifld].field_name, 2);
    
    UF_field_info_t infoBE = fieldInfo[ifld];
    UfRadar::BE_from_field_info(infoBE);
    fieldInfoBuf.add(&infoBE, sizeof(UF_field_info_t));
    
    UF_field_header_t hdrBE = fieldHdrs[ifld];
    UfRadar::BE_from_field_header(hdrBE, fieldName);
    fieldHdrBuf.add(&hdrBE, sizeof(UF_field_header_t));
    
    shortDataBuf.add(fieldData[ifld].getPtr(), fieldData[ifld].getLen());
    
  } // ifld

  // swap data
  
  BE_from_array_16(shortDataBuf.getPtr(), shortDataBuf.getLen());
  
  // undo reverse

  if (littleEndian) {
    BE_reverse();
  }

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
  
  if (fwrite(&manHdrBE, sizeof(manHdrBE), 1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing mandatory header" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (fwrite(&dataHdrBE, sizeof(dataHdrBE),  1, fp) != 1) {
    int errNum = errno;
    cerr << "ERROR - UfRecord::write" << endl;
    cerr << "  Writing data header" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (fwrite(fieldInfoBuf.getPtr(), 1, fieldInfoBuf.getLen(), fp) !=
      fieldInfoBuf.getLen()) {
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

  for (int ifld = 0; ifld < (int) fieldData.size(); ifld++) {

    const si16 *shortData = (const si16 *) fieldData[ifld].getPtr();
    int nData = fieldData[ifld].getLen() / sizeof(si16);
    
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

    UfRadar::print_mandatory_header(out, manHdr);
    UfRadar::print_data_header(out, dataHdr);
    printDerived(out);
    
  }
  
  for (size_t ii = 0; ii < fieldInfo.size(); ii++) {
    if (print_headers) {
      UfRadar::print_field_info(out, (int) ii, fieldInfo[ii]);
      UfRadar::print_field_header(out, fieldNames[ii], (int) ii, fieldHdrs[ii]);
    }
    if (print_data) {
      UfRadar::print_field_data(out, fieldNames[ii], (int) ii,
				fieldHdrs[ii].num_volumes,
				fieldHdrs[ii].scale_factor,
				manHdr.missing_data_val,
				(si16 *) fieldData[ii].getPtr());
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
  out << "    maxGates: " << maxGates << endl;
  out << "    numSamples: " << numSamples << endl;
  out << "    polarizationCode: " << polarizationCode << endl;
  out << "    startRange: " << startRange << endl;
  out << "    gateSpacing: " << gateSpacing << endl;
  out << "    radarConstant: " << radarConstant << endl;
  out << "    noisePower: " << noisePower << endl;
  out << "    receiverGain: " << receiverGain << endl;
  out << "    peakPower: " << peakPower << endl;
  out << "    antennaGain: " << antennaGain << endl;
  out << "    pulseWidth: " << pulseWidth << endl;
  out << "    horizBeamWidth: " << horizBeamWidth << endl;
  out << "    vertBeamWidth: " << vertBeamWidth << endl;
  out << "    wavelength: " << wavelength << endl;
  out << "    prf: " << prf << endl;
  out << "    nyquistVel: " << nyquistVel << endl;
  out << "    volNum: " << volNum << endl;
  out << "    tiltNum: " << tiltNum << endl;
  out << "    targetAngle: " << targetAngle << endl;
  out << "    elevation: " << elevation << endl;
  out << "    azimuth: " << azimuth << endl;
  out << "    time: " << DateTime::str(beamTime) << endl;
  out << "***********************************" << endl;

}


