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
////////////////////////////////////////////////////////////////////////
// PlanTransform.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
///////////////////////////////////////////////////////////////////////

#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include "PlanTransform.hh"
using namespace std;

const int PlanTransform::_azSign[8] = { 1, -1, 1, -1, 1, -1, 1, -1 };
const int PlanTransform::_xSign[8] = { 1, 1, 1, 1, -1, -1, -1, -1 };
const int PlanTransform::_ySign[8] = { 1, 1, -1, -1, -1, -1, 1, 1 };
const bool PlanTransform::_swapXy[8] = { false, true, true, false,
					 false, true, true, false };

//////////////////////
// Abstract Base class

PlanTransform::PlanTransform(const Params &params,
			     const string &mdv_url) :
  Transform(params, mdv_url)

{
}

PlanTransform::~PlanTransform()

{
  freeLut();
}

void PlanTransform::freeLut()

{
  _lut.clear();
}

////////////////////////////////////////////////////////////////
// write the output volume

int PlanTransform::writeVol(const DsRadarParams &radarParams,
			    time_t startTime, time_t endTime,
			    const vector<FieldInfo> &fields)

{

  PMU_auto_register("PlanTransform::writeVol");

  if (_params.debug) {
    cerr << "**** Start PlanTransform::writeVol() ****" << endl;
  }
  
  OutputMdv mdv("Dsr2Rapic", _params, _geomType);
  
  int volDuration = endTime - startTime;
  if (volDuration > _params.max_vol_duration) {
    if (_params.debug) {
      cerr << "WARNING - PlanTransform::_writeVol" << endl;
      cerr << "  Vol duration exceed max allowable" << endl;
      cerr << "  Vol duration: " << volDuration << " secs" << endl;
      cerr << "  Max duration: " << _params.max_vol_duration
	   << " secs" << endl;
    }
    return -1;
  }

  time_t midTime;
  if (_params.auto_mid_time) {
    midTime = startTime + (endTime - startTime) / 2;
  } else {
    midTime = endTime - _params.age_at_end_of_volume;
  }

  if (_params.debug) {
    cerr << "startTime: " << DateTime::str(startTime) << endl;
    cerr << "endTime: " << DateTime::str(endTime) << endl;
    cerr << "midTime: " << DateTime::str(midTime) << endl;
  }
  
  mdv.setMasterHeader(startTime, midTime, endTime,
		      _nx, _ny, _nz,
		      _radarLat, _radarLon, _radarAlt,
		      radarParams.radarName.c_str());
  
  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    const FieldInfo &fld = fields[ifield];
    if (fld.isLoaded) {
      mdv.addField(fld.name.c_str(),
		   fld.units.c_str(),
		   fld.isDbz,
		   _nx, _dx, _minx,
		   _ny, _dy, _miny,
		   _nz, _dz, _minz,
		   _elevArray,
		   _radarLat,
		   _radarLon,
		   _radarAlt,
		   fld.byteWidth,
		   fld.scale,
		   fld.bias,
		   fld.encoding,
		   fld.compression,
		   _outputFields[ifield]);
    }
  } // ifield

  // add coverage if requested

  if (_coverage != NULL) {

    Mdvx::compression_type_t compression = Mdvx::COMPRESSION_ZLIB;
    if (_params.output_compression == Params::RLE_COMPRESSION) {
      compression = Mdvx::COMPRESSION_RLE;
    } else if (_params.output_compression == Params::BZIP_COMPRESSION) {
      compression = Mdvx::COMPRESSION_BZIP;
    } else if (_params.output_compression == Params::GZIP_COMPRESSION) {
      compression = Mdvx::COMPRESSION_GZIP;
    } else if (_params.output_compression == Params::NO_COMPRESSION) {
      compression = Mdvx::COMPRESSION_NONE;
    }

    mdv.addField("Coverage", "", false,
		 _nx, _dx, _minx,
		 _ny, _dy, _miny,
		 _nz, _dz, _minz,
		 _elevArray,
		 _radarLat,
		 _radarLon,
		 _radarAlt,
		 4, 1.0, 0.0,
		 Mdvx::ENCODING_INT8,
		 compression,
		 _coverage);
  }
  
  mdv.addChunks(radarParams, _elevArray);

  if (mdv.writeVol(_mdvUrl.c_str())) {
    cerr << "ERROR - PlanTransform::writeVol" << endl;
    cerr << "  Cannot write output volume" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "**** End PlanTransform::writeVol() ****" << endl;
  }

  return 0;

}
