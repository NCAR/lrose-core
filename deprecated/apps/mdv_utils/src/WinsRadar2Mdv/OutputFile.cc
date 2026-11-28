/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1992, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// OutputFile.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name,
		       const Params &params) :
  _progName(prog_name), _params(params)

{

}

/////////////
// destructor

OutputFile::~OutputFile()
  
{
  
}

////////////////////////////////////////////////////
// loadData()
//
// Load object up with the data
//

void OutputFile::loadData(const WinsRadar &radar)
  
{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  const WinsRadar::header_t &rhdr = radar.getHeader();

  DateTime centTime(rhdr.year, rhdr.month, rhdr.day,
		    rhdr.hour, rhdr.min_start);
  DateTime endTime(rhdr.year, rhdr.month, rhdr.day,
		   rhdr.hour, rhdr.min_end);

  mhdr.time_begin = centTime.utime();
  mhdr.time_end = endTime.utime();
  mhdr.time_centroid = centTime.utime();
  mhdr.time_expire = endTime.utime() + 3600;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = rhdr.nx;
  mhdr.max_ny = rhdr.ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  STRncopy(mhdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);
  
  _mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  // add fields

  _mdvx.clearFields();
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  fhdr.nx = rhdr.nx;
  fhdr.ny = rhdr.ny;
  fhdr.nz = 1;
  
  fhdr.encoding_type = Mdvx::ENCODING_INT8;
  fhdr.data_element_nbytes = sizeof(ui08);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  
  if (rhdr.log_flag) {
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_LOG;
  } else {
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  }
  fhdr.scaling_type = Mdvx::SCALING_DYNAMIC;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  fhdr.dz_constant = true;
  fhdr.nz = 1;

  fhdr.proj_type = Mdvx::PROJ_FLAT;
  fhdr.proj_origin_lat = rhdr.lat / 10000.0;
  fhdr.proj_origin_lon = rhdr.lon / 10000.0;
  fhdr.grid_dx = rhdr.dx / 1000.0;
  fhdr.grid_dy = rhdr.dy / 1000.0;
  fhdr.grid_minx = -1.0 * (fhdr.grid_dx * ((rhdr.nx / 2.0) - 0.5));
  fhdr.grid_miny = -1.0 * (fhdr.grid_dy * ((rhdr.ny / 2.0) - 0.5));
  fhdr.grid_minz = 0.5;
  fhdr.grid_dz = 1.0;

  fhdr.proj_rotation = 0.0;
  
  fhdr.scale = radar.getScale();
  fhdr.bias = radar.getBias();
  fhdr.bad_data_value = 255;
  fhdr.missing_data_value = 255;

  STRncopy(fhdr.field_name, radar.getFieldName(), MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, radar.getFieldName(), MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, radar.getFieldUnits(), MDV_UNITS_LEN);
  if (strstr(fhdr.units, "dBZ") != NULL) {
    STRncopy(fhdr.transform, "dBZ", MDV_TRANSFORM_LEN);
  }
  
  // vlevel header
    
  vhdr.type[0] = Mdvx::VERT_TYPE_COMPOSITE;
  vhdr.level[0] = 0.5;

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, radar.getData());

  // rescale the field into floats

  field->convertType(Mdvx::ENCODING_FLOAT32);

  // remap as required

  MdvxRemapLut lut;
  if (_params.output_projection == Params::OUTPUT_PROJ_FLAT) {
    field->remap2Flat(lut,
		      _params.output_grid.nx,
		      _params.output_grid.ny,
		      _params.output_grid.minx,
		      _params.output_grid.miny,
		      _params.output_grid.dx,
		      _params.output_grid.dy,
		      _params.output_origin.lat,
		      _params.output_origin.lon,
		      0.0);
  } else if (_params.output_projection == Params::OUTPUT_PROJ_LATLON) {
    field->remap2Latlon(lut,
			_params.output_grid.nx,
			_params.output_grid.ny,
			_params.output_grid.minx,
			_params.output_grid.miny,
			_params.output_grid.dx,
			_params.output_grid.dy);
  } else if (_params.output_projection == Params::OUTPUT_PROJ_LAMBERT) {
    field->remap2Lc2(lut,
		     _params.output_grid.nx,
		     _params.output_grid.ny,
		     _params.output_grid.minx,
		     _params.output_grid.miny,
		     _params.output_grid.dx,
		     _params.output_grid.dy,
		     _params.output_origin.lat,
		     _params.output_origin.lon,
		     _params.lambert_lat1,
		     _params.lambert_lat2);
  }

  // round back into bytes and compress

  field->convertRounded(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_ZLIB);

  // add field to mdvx object
  
  _mdvx.addField(field);
    
}

////////////////////////////////////////
// writeVol()
//
// Write out volume in MDV format.
//

int OutputFile::writeVol()
  
{
  
  PMU_auto_register("In OutputFile::writeVol");
  
  if (_params.debug) {
    fprintf(stderr, "Writing MDV file, time %s, to url %s\n",
	    utimstr(_mdvx.getMasterHeader().time_centroid),
	    _params.output_url);
  }
  
  // write to directory

  _mdvx.setAppName(_progName);
  _mdvx.setWriteLdataInfo();
  if (_mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  } else {
    return 0;
  }

}
