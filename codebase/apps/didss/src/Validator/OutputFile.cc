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
// OutputFile.cc
//
// Handles output to MDV files
//
// This was initially copied from the MM5Ingest program.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 2000
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"
#include "Params.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/udatetime.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
using namespace std;

/////////////////////////////////////////////////////
//
// Destructor.
//
OutputFile::~OutputFile(){

}

////////////////////////////////////////////////////
// Constructor
//
// Initialize the MDVX object
//

OutputFile::OutputFile(time_t dataTime,
		       Mdvx::master_header_t inMhdr,
		       Params *P,
		       float *truthDetected,
		       float *forecastDetected,
		       float *contingency,
		       float ContingencyMissing)
  
{

  // set the master header - initially copy the input.
  
  Mdvx::master_header_t outMhdr;

  outMhdr = inMhdr;

  outMhdr.time_gen = time(NULL);
  outMhdr.time_begin = dataTime;
  outMhdr.time_end = dataTime;
  outMhdr.time_centroid = dataTime;
  outMhdr.time_expire = 0;
    
  outMhdr.num_data_times = 1;

  if (P->composite_truth) // Data dimension changes from 3 to 2 in this case
    {
      outMhdr.data_dimension = 2;
      outMhdr.max_nz = 1;
    }

  outMhdr.data_collection_type = Mdvx::DATA_SYNTHESIS;
  //outMhdr.native_vlevel_type = Mdvx::VERT_TYPE_SIGMA_P;
  //outMhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
  outMhdr.vlevel_included = TRUE;
  //outMhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  outMhdr.data_ordering = Mdvx::ORDER_XYZ;
  //outMhdr.max_nx = _params.output_grid.nx;
  //outMhdr.max_ny = _params.output_grid.ny;


  outMhdr.n_chunks = 0;
  outMhdr.field_grids_differ = FALSE;
  //outMhdr.sensor_lon = 0.0;
  //outMhdr.sensor_lat = 0.0;
  //outMhdr.sensor_alt = 0.0;

  STRncopy(outMhdr.data_set_info, "Validator output", MDV_INFO_LEN);
  STRncopy(outMhdr.data_set_name, "Validator output", MDV_NAME_LEN);
  STRncopy(outMhdr.data_set_source, "Validator", MDV_NAME_LEN);

  outMhdr.n_fields = 3;

  _mdvx.setMasterHeader(outMhdr);
  
  // fill in field headers and vlevel headers
  
  // _npointsPlane = _params.output_grid.nx * _params.output_grid.ny;

  // add the three output fields

  _mdvx.clearFields();
  
  for (int out_field = 0; out_field < 3; out_field++) {

    
    Mdvx::field_header_t fhdr;
    MEM_zero(fhdr);
    Mdvx::vlevel_header_t vhdr;
    MEM_zero(vhdr);

    fhdr.forecast_delta = 0;
    fhdr.forecast_time = dataTime;
    
    fhdr.nx = P->grid_nx;
    fhdr.ny = P->grid_ny;
    if (P->composite_truth){
      fhdr.nz = 1;
    } else {
      fhdr.nz = P->grid_nz;
    }

    switch (P->grid_projection) {

    case Params::FLAT :
      fhdr.proj_type = Mdvx::PROJ_FLAT;
      break;

    case Params::LATLON :
      fhdr.proj_type = Mdvx::PROJ_LATLON;
      break;

    case Params::LAMBERT :
      fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      break;

    default :
      cerr << "Unsupported projection specified - I cannot cope." << endl;
      exit(-1);
      break; 

    }


    fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    fhdr.data_element_nbytes = sizeof(fl32);
    fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
    fhdr.compression_type = Mdvx::COMPRESSION_NONE;
    fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    fhdr.scaling_type = Mdvx::SCALING_NONE;

    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SIGMA_P;
    fhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
    fhdr.dz_constant = false;

    fhdr.proj_origin_lat = P->grid_origin_lat;
    fhdr.proj_origin_lon = P->grid_origin_lon;
    
    fhdr.grid_dx = P->grid_dx;
    fhdr.grid_dy = P->grid_dy;
    fhdr.grid_dz = P->grid_dz;

    fhdr.grid_minx = P->grid_minx;
    fhdr.grid_miny = P->grid_miny;
    fhdr.grid_minz = P->composite_min;

    fhdr.proj_rotation = 0.0;

    fhdr.bad_data_value = ContingencyMissing;
    fhdr.missing_data_value = ContingencyMissing;

    // vlevel header

    for (int iz = 0; iz < fhdr.nz; iz++) {

      if (P->composite_truth){
	vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
      } else {
	vhdr.type[iz] = Mdvx::VERT_TYPE_Z;
      }
      vhdr.level[iz] = P->composite_min + iz * P->grid_dz;
    }

    // Point to the data

    float *data;
    switch (out_field){
    case 0 :
      data = truthDetected;
      STRncopy(fhdr.field_name_long, "Truth", MDV_LONG_FIELD_LEN);
      STRncopy(fhdr.field_name, "Truth", MDV_SHORT_FIELD_LEN);
      break;
    case 1 :
      data = forecastDetected;
      STRncopy(fhdr.field_name_long, "Forecast", MDV_LONG_FIELD_LEN);
      STRncopy(fhdr.field_name, "Forecast", MDV_SHORT_FIELD_LEN);
      break;
    case 2 :
      data = contingency;
      STRncopy(fhdr.field_name_long, "Contingency", MDV_LONG_FIELD_LEN);
      STRncopy(fhdr.field_name, "Contingency", MDV_SHORT_FIELD_LEN);
      break;
    default :
      cerr << "Too many fields in output loop. I cannot cope." << endl;
      exit(-1);
      break;
    }

    STRncopy(fhdr.units, "none", MDV_UNITS_LEN);
    STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
    fhdr.field_code = 77; // Undefined.

    // create field

    MdvxField *field = new MdvxField(fhdr, vhdr, data);

    // Convert field to INT8 encoding.

    if (field->convertRounded(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }

    // add field to mdvx object

    _mdvx.addField(field);
    
  } // out_field


  PMU_auto_register("Writing output");
  
  if (P->Debug) {
    cerr << "Writing MDV file, time " << utimstr(dataTime);
    cerr << " to " << P->OutDir << endl;
  }
  
  // write to directory
  
  _mdvx.setWriteLdataInfo();
  if (_mdvx.writeToDir(P->OutDir)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _mdvx.getErrStr() << endl;
    exit(-1);
  } 




}





