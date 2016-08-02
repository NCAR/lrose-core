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
// OutputMdv.cc
//
// Handles output to MDV files.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2012
//
///////////////////////////////////////////////////////////////

#include "OutputMdv.hh"
#include <dataport/bigend.h>

#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DsLdataInfo.hh>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>

using namespace std;

//////////////
// Constructor

OutputMdv::OutputMdv(const string &prog_name,
		     const Params &params) :
        _progName(prog_name),
        _params(params)
  
{

}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
}

/////////////////////////
// set the master header

void OutputMdv::setMasterHeader(const RadxVol &vol)

{
  
  // set master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.index_number = vol.getVolumeNumber();

  mhdr.time_gen = time(NULL);
  mhdr.time_begin = vol.getStartTimeSecs();
  mhdr.time_end = vol.getEndTimeSecs();
  mhdr.time_centroid = vol.getEndTimeSecs();
  mhdr.time_expire = mhdr.time_centroid;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;  
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = vol.getLongitudeDeg();
  mhdr.sensor_lat = vol.getLatitudeDeg();
  mhdr.sensor_alt = vol.getAltitudeKm();
  
  _mdvx.setMasterHeader(mhdr);
  
  _mdvx.setDataSetInfo(vol.getHistory().c_str());
  _mdvx.setDataSetName(vol.getInstrumentName().c_str());
  _mdvx.setDataSetSource(vol.getSource().c_str());
  
}

////////////////////
// addField()
//

void OutputMdv::addField(const RadxVol &vol,
                         MdvxProj &proj,
                         const vector<double> &vlevels,
                         const string &field_name,
                         const string &field_name_long,
			 const string &units,
                         Radx::DataType_t inputDataType,
                         double inputScale,
                         double inputOffset,
                         fl32 missingVal,
			 const fl32 *data)
  
{
  
  int nz = (int) vlevels.size();
  Mdvx::coord_t coord = proj.getCoord();

  LOGF(LogMsg::DEBUG_VERBOSE, "  Adding field: %s", field_name.c_str());

  if (coord.nx == 0 || coord.ny == 0 || nz == 0) {
    LOGF(LogMsg::WARNING,
	 "  Zero length field, not adding, name:%s, nx,ny,nz:%d,%d,%d",
	 field_name.c_str(), coord.nx, coord.ny, nz);
    return;
  }

  // field header

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx = coord.nx;
  fhdr.ny = coord.ny;
  fhdr.nz = nz;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.proj_type = proj.getProjType();
  fhdr.dz_constant = true;

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = coord.nx * coord.ny * nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.proj_origin_lat = vol.getLatitudeDeg();
  fhdr.proj_origin_lon = vol.getLongitudeDeg();
  fhdr.user_data_fl32[0] = vol.getAltitudeKm();

  fhdr.grid_dx = coord.dx;
  fhdr.grid_dy = coord.dy;
  fhdr.grid_dz = 1.0;
  // if (nz > 1) {
  //   fhdr.grid_dz = vlevels[1] - vlevels[0];
  // }

  fhdr.grid_minx = coord.minx;
  fhdr.grid_miny = coord.miny;
  fhdr.grid_minz = vlevels[0];

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;

  proj.syncXyToFieldHdr(fhdr);
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  int nLevels = nz;
  for (int i = 0; i < nLevels; i++) {
    vhdr.level[i] = vlevels[i];
    vhdr.type[i] = Mdvx::VERT_TYPE_SURFACE;
  }
  
  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, data);
  if (inputDataType == Radx::SI08 ||
      inputDataType == Radx::UI08 ||
      inputDataType == Radx::SI16 ||
      inputDataType == Radx::UI16) {
    fld->convertType(Mdvx::ENCODING_INT16,
		     Mdvx::COMPRESSION_GZIP,
		     Mdvx::SCALING_DYNAMIC);
  } else {
    fld->convertType(Mdvx::ENCODING_FLOAT32,
		     Mdvx::COMPRESSION_GZIP);
  }

  // set strings
  
  fld->setFieldName(field_name.c_str());
  fld->setFieldNameLong(field_name_long.c_str());
  fld->setUnits(units.c_str());
  fld->setTransform("");
  
  // add to object
  
  _mdvx.addField(fld);

}





////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol()

{

  _outputDir = _params.output_cartesian_dir;
  time_t ftime = _mdvx.getMasterHeader().time_centroid;

  if (_mdvx.getNFields() == 0) {
    cerr << "WARNING - no fields in file" << endl;
    cerr << "  Not writing MDV file, time : " << utimstr(ftime) << endl;
    cerr << "  Dir path: " << _outputDir << endl;
    return -1;
  }

  LOGF(LogMsg::DEBUG_VERBOSE, "Writing MDV file, time : %s", utimstr(ftime));
  LOGF(LogMsg::DEBUG_VERBOSE, "  Dir path: %s", _outputDir.c_str());

  // write out file

  if (!_params.write_latest_data_info) {
    _mdvx.clearWriteLdataInfo();
  }
  if (_mdvx.writeToDir(_outputDir)) {
    LOG(LogMsg::ERROR, _mdvx.getErrStr().c_str());
    return -1;
  }
  
  LOGF(LogMsg::DEBUG, "Done writing MDV file, path: %s",
       _mdvx.getPathInUse().c_str());

  return 0;

}

