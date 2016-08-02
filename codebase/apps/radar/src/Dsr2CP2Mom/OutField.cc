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
///////////////////////////////////////////////////////////////
// OutField.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////

#define __in_outfield_cc__
#include "OutField.hh"

#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

OutField::OutField(const Params &params) :
  _params(params)
  
{
  dsrFieldNum = -1;
  cp2FieldId = -1;
  memset(&hdr, 0, sizeof(CP2Net::CP2ProductHeader));
  data = NULL;
}

// destructor

OutField::~OutField()

{
  if (data) {
    delete data;
  }
}

// Set field values
// returns 0 on success, -1 on error

int OutField::init(const string &field_name,
		   int dsr_field_num,
		   int cp2_field_id,
		   const DsRadarParams &radar_params,
		   const DsFieldParams &field_params,
		   const DsRadarBeam &radar_beam,
		   long long beam_num)

{

  fieldName = field_name;
  dsrFieldNum = dsr_field_num;
  cp2FieldId = cp2_field_id;
  
  hdr.beamNum = beam_num;
  hdr.gates = radar_params.numGates;
  hdr.az = radar_beam.azimuth;
  hdr.el = radar_beam.elevation;
  hdr.gateWidthKm = radar_params.gateSpacing;
  
  // unpack data

  if (data) {
    delete data;
  }
  data = new double[radar_params.numGates];
  
  if(field_params.byteWidth == 1) {
    
    ui08 *dd = (ui08 *) radar_beam.getData() + dsrFieldNum;
    ui08 missing = (ui08) field_params.missingDataValue;
    double scale = field_params.scale;
    double bias = field_params.bias;
    for (int ii = 0; ii < radar_params.numGates;
	 ii++, dd += radar_params.numFields) {
      if (*dd == missing) {
	data[ii] = -9999;
      } else {
	data[ii] = *dd * scale + bias;
      }
    }

  } else if (field_params.byteWidth == 2) {
    
    ui16 *dd = (ui16 *) radar_beam.getData() + dsrFieldNum;
    ui16 missing = (ui16) field_params.missingDataValue;
    double scale = field_params.scale;
    double bias = field_params.bias;
    for (int ii = 0; ii < radar_params.numGates;
	 ii++, dd += radar_params.numFields) {
      if (*dd == missing) {
	data[ii] = -9999;
      } else {
	data[ii] = *dd * scale + bias;
      }
    }

  } else if (field_params.byteWidth == 4) {

    fl32 *dd = (fl32 *) radar_beam.getData() + dsrFieldNum;
    fl32 missing = (fl32) field_params.missingDataValue;
    for (int ii = 0; ii < radar_params.numGates;
	 ii++, dd += radar_params.numFields) {
      if (*dd == missing) {
	data[ii] = -9999;
      } else {
	data[ii] = *dd;
      }
    }

  } else {
    
    cerr << "ERROR - invalid byte width: " << field_params.byteWidth << endl;
    return -1;

  }

  return 0;

}

// print

void OutField::print(ostream &out)

{

  out << "============ OutField ==============" << endl;
  out << "  fieldName: " << fieldName << endl;
  out << "  dsrFieldNum: " << dsrFieldNum << endl;
  out << "  cp2FieldId: " << cp2FieldId << endl;
  out << "  beamNum: " << hdr.beamNum << endl;
  out << "  ngates: " << hdr.gates << endl;
  out << "  az: " << hdr.az << endl;
  out << "  el: " << hdr.el << endl;
  out << "  gateWidthKm: " << hdr.gateWidthKm << endl;

}

