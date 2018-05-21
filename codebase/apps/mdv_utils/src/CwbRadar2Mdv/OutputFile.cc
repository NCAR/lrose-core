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
// June 2003
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

void OutputFile::loadData(const InputFile &inFile)
  
{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_gen = time(NULL);
  mhdr.time_begin = inFile.getTime().utime();
  mhdr.time_end = inFile.getTime().utime();
  mhdr.time_centroid = inFile.getTime().utime();
  mhdr.time_expire = mhdr.time_centroid + 3600;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  if (_params.vlevel_units == Params::VLEVEL_KM) {
    mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  } else {
    mhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
  }
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = inFile.getNx();
  mhdr.max_ny = inFile.getNy();
  mhdr.max_nz = inFile.getNz();
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  STRncopy(mhdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);

  string source;
  const vector<string> &radarNames = inFile.getRadarNames();
  for (size_t ii = 0; ii < radarNames.size(); ii++) {
    source += radarNames[ii];
    source += " ";
  }
  STRncopy(mhdr.data_set_source, source.c_str(), MDV_NAME_LEN);
  
  _mdvx.setMasterHeader(mhdr);
  
  // vlevel header

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  for (int i = 0; i < inFile.getNz(); i++) {
    if (_params.vlevel_units == Params::VLEVEL_KM) {
      vhdr.type[i] = Mdvx::VERT_TYPE_Z;
      vhdr.level[i] = inFile.getZLevels()[i];
    } else {
      vhdr.type[i] = Mdvx::VERT_FLIGHT_LEVEL;
      vhdr.level[i] =
	((int) ((inFile.getZLevels()[i] / 0.03048) / 5.0)) * 5.0;
    }
  }

  // fill in field headers and vlevel headers
  
  // add fields

  _mdvx.clearFields();
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx = inFile.getNx();
  fhdr.ny = inFile.getNy();
  fhdr.nz = inFile.getNz();
  if (fhdr.nz > 100) {
    fhdr.nz = 100;
  }
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_DYNAMIC;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  if (_params.vlevel_units == Params::VLEVEL_KM) {
    fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  } else {
    fhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
  }
  fhdr.dz_constant = false;

  if (inFile.getProjType() != "LL") {
    cerr << "WARNING - CwbRadar2Mdv" << endl;
    cerr << "  Only supports LatLon projection at this time." << endl;
    cerr << "  Output proj in input file: " << inFile.getProjType() << endl;
    cerr << "  This will be ignored." << endl;
  }
  
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.proj_origin_lat = 0.0;
  fhdr.proj_origin_lon = 0.0;
  fhdr.grid_dx = inFile.getDx();
  fhdr.grid_dy = inFile.getDy();
  fhdr.grid_dz = 1.0;
  fhdr.grid_minx = inFile.getNwLon();
  fhdr.grid_miny = inFile.getNwLat() - (fhdr.grid_dy * (fhdr.ny - 1));
  fhdr.grid_minz = vhdr.level[0];
  
  fhdr.proj_rotation = 0.0;
  
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.missing_data_value = (fl32) inFile.getMissing();
  fhdr.bad_data_value = fhdr.missing_data_value;

  STRncopy(fhdr.field_name, _params.output_short_field_name,
	   MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long, inFile.getFieldName().c_str(),
	   MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.units, inFile.getFieldUnits().c_str(), MDV_UNITS_LEN);
  if (strstr(fhdr.units, "dBZ") != NULL ||
      strstr(fhdr.units, "dbz") != NULL) {
    MdvxFieldCode::entry_t entry;
    if (MdvxFieldCode::getEntryByName("Radar Reflectivity", entry) == 0) {
      fhdr.field_code = entry.code;
    }
    STRncopy(fhdr.transform, "dBZ", MDV_TRANSFORM_LEN);
  }
  
  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, inFile.getFieldData());
  
  // remap as required

  if (_params.output_projection != Params::OUTPUT_PROJ_NATIVE) {
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
  }
  
  // round back into bytes and compress
  
  field->convertRounded((Mdvx::encoding_type_t) _params.output_encoding,
			Mdvx::COMPRESSION_ZLIB);

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
