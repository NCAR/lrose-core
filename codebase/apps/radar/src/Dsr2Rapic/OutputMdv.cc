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
// July 2001
//
///////////////////////////////////////////////////////////////

#include "OutputMdv.hh"
#include "Beam.hh"

#include <dataport/bigend.h>

#include <toolsa/mem.h>
#include <toolsa/str.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
using namespace std;

//////////////
// Constructor

OutputMdv::OutputMdv(const string &prog_name,
		     const Params &params,
		     output_grid_geom_t geom_type) :
  _progName(prog_name),
  _params(params),
  _geomType(geom_type)
  
{

}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
}

/////////////////////////
// set the master header

void OutputMdv::setMasterHeader(time_t start_time,
				time_t mid_time,
				time_t end_time,
				int nx,
				int ny,
				int nz,
				double radar_lat,
				double radar_lon,
				double radar_alt,
				const char *radar_name)

{
  
  // set master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  
  mhdr.time_gen = time(NULL);
  mhdr.time_begin = start_time;
  mhdr.time_end = end_time;
  mhdr.time_centroid = mid_time;
  mhdr.time_expire = end_time + (end_time - start_time) * 2;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  if (_geomType == CART_OUTPUT_GRID) {
    mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  } else if (_geomType == PPI_OUTPUT_GRID) {
    mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  } else if (_geomType == POLAR_OUTPUT_GRID) {
    mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  } else if (_geomType == RHI_OUTPUT_GRID) {
    mhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
    mhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
  }
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = nx;
  mhdr.max_ny = ny;
  mhdr.max_nz = nz;
  mhdr.n_fields = 0;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = radar_lon;
  mhdr.sensor_lat = radar_lat;
  mhdr.sensor_alt = radar_alt;
  
  _mdvx.setMasterHeader(mhdr);
  
  _mdvx.setDataSetInfo(_params.data_set_info);
  _mdvx.setDataSetName(radar_name);
  _mdvx.setDataSetSource(_params.data_set_source);
  
}

////////////////////
// addField()
//

void OutputMdv::addField(const char *field_name,
			 const char *units,
			 bool isDbz,
			 int nx,
			 double dx,
			 double minx,
			 int ny,
			 double dy,
			 double miny,
			 int nz,
			 double dz,
			 double minz,
			 const vector<double> &vlevel_array,
			 double radar_lat,
			 double radar_lon,
			 double radar_alt,
			 int input_byte_width,
			 double input_scale,
			 double input_bias,
			 Mdvx::encoding_type_t encoding,
			 Mdvx::compression_type_t compression,
			 const fl32 *data)
  
{
  
  if (_params.debug) {
    cerr << "  Adding field: " << field_name << endl;
  }

  // field header

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = nz;

  if (_geomType == CART_OUTPUT_GRID) {
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
    fhdr.dz_constant = true;
  } else if (_geomType == PPI_OUTPUT_GRID) {
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.dz_constant = false;
  } else if (_geomType == POLAR_OUTPUT_GRID) {
    fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
    fhdr.dz_constant = false;
  } else if (_geomType == RHI_OUTPUT_GRID) {
    fhdr.proj_type = Mdvx::PROJ_RHI_RADAR;
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_AZ;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
    fhdr.dz_constant = false;
  }

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = nx * ny * nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.proj_origin_lat = radar_lat;
  fhdr.proj_origin_lon = radar_lon;
  fhdr.user_data_fl32[0] = radar_alt;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = dz;

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = minz;

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = Beam::missFl32;
  fhdr.missing_data_value = Beam::missFl32;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  STRncopy(fhdr.field_name_long, field_name, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  if (isDbz) {
    STRncopy(fhdr.transform, "dBZ", MDV_TRANSFORM_LEN);
  } else {
    STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
  }
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  int nLevels = nz;
  if (nLevels > MDV_MAX_VLEVELS) {
    nLevels = MDV_MAX_VLEVELS;
  }
  if (_geomType == CART_OUTPUT_GRID) {
    for (int i = 0; i < nLevels; i++) {
      vhdr.level[i] = minz + i * dz;
      vhdr.type[i] = Mdvx::VERT_TYPE_Z;
    }
  } else if (_geomType == PPI_OUTPUT_GRID) {
    for (int i = 0; i < nLevels; i++) {
      vhdr.level[i] = vlevel_array[i];
      vhdr.type[i] = Mdvx::VERT_TYPE_ELEV;
    }
  } else if (_geomType == POLAR_OUTPUT_GRID) {
    for (int i = 0; i < nLevels; i++) {
      vhdr.level[i] = vlevel_array[i];
      vhdr.type[i] = Mdvx::VERT_TYPE_ELEV;
    }
  } else if (_geomType == RHI_OUTPUT_GRID) {
    for (int i = 0; i < nLevels; i++) {
      vhdr.level[i] = vlevel_array[i];
      vhdr.type[i] = Mdvx::VERT_TYPE_AZ;
    }
  }
  
  // use input scale and bias?

  bool useInputScaleAndBias = false;
  if (encoding == Mdvx::ENCODING_INT8 && input_byte_width == 1) {
    useInputScaleAndBias = true;
  } else if (encoding == Mdvx::ENCODING_INT16 && input_byte_width <= 2) {
    useInputScaleAndBias = true;
  }

  // create field

  MdvxField *fld = NULL;

  if (useInputScaleAndBias) {

    int npts = nx * ny * nz;
    fhdr.bad_data_value = 0;
    fhdr.missing_data_value = 0;
    fhdr.scale = input_scale;
    fhdr.bias = input_bias;

    if (encoding == Mdvx::ENCODING_INT8) {

      // 8-bit
      
      ui08 *bdata = new ui08[npts];
      ui08 *bp = bdata;
      const fl32 *fp = data;
      for (int i = 0; i < npts; i++, bp++, fp++) {
	if (*fp == Beam::missFl32) {
	  *bp = 0;
	} else {
	  int bb = (int) ((*fp - input_bias) / input_scale + 0.5);
	  if (bb < 2) {
	    bb = 2;
	  } else if (bb > 255) {
	    bb = 255;
	  }
	  *bp = bb;
	}
      }
      fhdr.encoding_type = Mdvx::ENCODING_INT8;
      fhdr.data_element_nbytes = 1;
      fhdr.volume_size = nx * ny * nz * sizeof(ui08);
      fld = new MdvxField(fhdr, vhdr, bdata);
      delete[] bdata;

    } else {
      
      // 16-bit

      ui16 *sdata = new ui16[npts];
      ui16 *sp = sdata;
      const fl32 *fp = data;
      for (int i = 0; i < npts; i++, sp++, fp++) {
	if (*fp == Beam::missFl32) {
	  *sp = 0;
	} else {
	  int ss = (int) ((*fp - input_bias) / input_scale + 0.5);
	  if (ss < 2) {
	    ss = 2;
	  } else if (ss > 65535) {
	    ss = 65535;
	  }
	  *sp = ss;
	}
      }
      fhdr.encoding_type = Mdvx::ENCODING_INT16;
      fhdr.data_element_nbytes = 2;
      fhdr.volume_size = nx * ny * nz * sizeof(ui16);
      fld = new MdvxField(fhdr, vhdr, sdata);
      delete[] sdata;
      
    }

    fld->computeMinAndMax(true);
    fld->compress(compression);
  
  } else {
    
    fld = new MdvxField(fhdr, vhdr, data);
    fld->convertType(encoding, compression, Mdvx::SCALING_DYNAMIC);
    
    // if (useInputScaleAndBias) {
    //   fld->convertType(encoding, compression, Mdvx::SCALING_SPECIFIED,
    //                    scale, bias);
    // } else {
    //   fld->convertType(encoding, compression, Mdvx::SCALING_DYNAMIC);
    // }

  } // if (useInputScaleAndBias)

  // set strings

  fld->setFieldName(field_name);
  fld->setFieldNameLong(field_name);
  fld->setUnits(units);
  if (isDbz) {
    fld->setTransform("dBZ");
  } else {
    fld->setTransform("none");
  }
  
  // add to object
  
  _mdvx.addField(fld);

}

////////////////////////////////////////
// addChunks()
//
// Add the DsRadarParams and elevation list as mdv chunks.
//

void OutputMdv::addChunks(const DsRadarParams &rparams,
                          const vector<double> &elevArray)

{
  MdvxChunk *chunk;

  // create a new radar param chunk

  chunk = new MdvxChunk;
  chunk->setId(Mdvx::CHUNK_DSRADAR_PARAMS);
  chunk->setInfo("DsRadar params");

  // load the chunk with radar parameters

  DsRadarParams_t rparamsStruct;
  rparams.encode(&rparamsStruct);
  chunk->setData(&rparamsStruct, sizeof(DsRadarParams_t));
  _mdvx.addChunk(chunk);

  chunk = new MdvxChunk;

  if (_geomType == RHI_OUTPUT_GRID) {

    // create a new azimuths chunk
    
    chunk->setId(Mdvx::CHUNK_DSRADAR_AZIMUTHS);
    chunk->setInfo("RHI azimuth angles");
    
    MemBuf azBuf;
    si32 numAz = (si32)elevArray.size();
    azBuf.add(&numAz, sizeof(si32));
    
    for( size_t i=0; i < elevArray.size(); i++ ) {
      fl32 az = (fl32)elevArray[i];
      azBuf.add((char *) &az, sizeof(fl32));
    }
    BE_from_array_32(azBuf.getPtr(), azBuf.getLen());
    chunk->setData(azBuf.getPtr(), azBuf.getLen());
    _mdvx.addChunk(chunk);

  } else {

    // create a new elevations chunk
    
    chunk->setId(Mdvx::CHUNK_DSRADAR_ELEVATIONS);
    chunk->setInfo("Radar Elevation angles");
    
    // load the chunk with elevation angles
    
    MemBuf elevBuf;
    si32 numElev = (si32)elevArray.size();
    elevBuf.add(&numElev, sizeof(si32));
    
    for( size_t i=0; i < elevArray.size(); i++ ) {
      fl32 elevation = (fl32)elevArray[i];
      elevBuf.add((char *) &elevation, sizeof(fl32));
    }
    BE_from_array_32(elevBuf.getPtr(), elevBuf.getLen());
    chunk->setData(elevBuf.getPtr(), elevBuf.getLen());
    _mdvx.addChunk(chunk);

  }

}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol(const char *url)

{
  
  if (_params.debug) {
    time_t ftime = _mdvx.getMasterHeader().time_centroid;
    cerr << "Writing MDV file, time : "
      	 << utimstr(ftime)
	 << " to URL: " << url << endl;
  }

  // write out file
  
  if (_mdvx.writeToDir(url)) {
    cerr << "ERROR - OutputMdv::writeVol" << endl;
    cerr << _mdvx.getErrStr();
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Done writing MDV file" << endl;
  }

  return 0;

}


