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
// ClutterTableGenerate.cc
//
// ClutterTableGenerate object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <rapformats/ds_radar.h>
#include <rapformats/clutter_table.h>
#include "ClutterTableGenerate.hh"
using namespace std;

// Constructor

ClutterTableGenerate::ClutterTableGenerate(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "ClutterTableGenerate";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

ClutterTableGenerate::~ClutterTableGenerate()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ClutterTableGenerate::Run()
{
  
  // register with procmap
  
  PMU_auto_register("Run");

  // read in the median dbz data

  Mdvx mdvx;
  mdvx.setReadPath(_params.median_clutter_path);
  mdvx.addReadField(_params.dbz_field_name);
  mdvx.setReadEncodingType(Mdvx::ENCODING_INT8);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (mdvx.readVolume()) {
    cerr << "ERROR - ClutterTableGenerate::Run" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Read in median file: " << _params.median_clutter_path << endl;
  }

  // read in the polar2mdv lookup table

  P2mdv_lookup_file_index_t lut;
  
  if (InitP2mdvIndex(&lut, sizeof(lut),
		     (char *) _progName.c_str(),
		     NULL, POLAR2MDV_LOOKUP_LABEL, NULL,
		     "ClutterTableGenerate::Run")) {
    return -1;
  }

  if (ReadP2mdvLookup(&lut, _params.dsr2mdv_lookup_path,
		      "ClutterTableGenerate::Run")) {
    cerr << "ERROR - ClutterTableGenerate::Run" << endl;
    cerr << "  Cannot read lookup table: "
	 << _params.dsr2mdv_lookup_path << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Read in cart lookup file: "
	 << _params.dsr2mdv_lookup_path << endl;
  }

  // check the geometry

  MdvxField *dbzField = mdvx.getFieldByName(_params.dbz_field_name);
  if (_checkGeom(*dbzField, lut)) {
    cerr << "ERROR - ClutterTableGenerate::Run" << endl;
    cerr << "Geometry does not match between median file and lookup table.";
    cerr << "  median file: " << _params.median_clutter_path << endl;
    cerr << "  cart lookup file: " << _params.dsr2mdv_lookup_path << endl;
    return -1;
  }

  // write the table

  if (_writeTable(mdvx, lut)) {
    cerr << "ERROR - ClutterTableGenerate::Run" << endl;
    cerr << "  Cannot write clutter table." << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////////
// check the geometry between the lookup table and the 
// dbz data grid
//
// Returns 0 on success, -1 on failure.

int ClutterTableGenerate::_checkGeom(const MdvxField &dbzFld,
				     const P2mdv_lookup_file_index_t &lut)

{

  const _MDV_radar_grid_t &grid = lut.lookup_params->grid;
  const Mdvx::field_header_t &dbzFhdr = dbzFld.getFieldHeader();

  int iret = 0;

  if (grid.nx != dbzFhdr.nx) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in nx: clutter file nx = " << dbzFhdr.nx
	 << ", lookup table nx = " << grid.nx << endl;
    iret = -1;
  }

  if (grid.ny != dbzFhdr.ny) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in ny: clutter file ny = " << dbzFhdr.ny
	 << ", lookup table ny = " << grid.ny << endl;
    iret = -1;
  }

  if (grid.nz != dbzFhdr.nz) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in nz: clutter file nz = " << dbzFhdr.nz
	 << ", lookup table nz = " << grid.nz << endl;
    iret = -1;
  }

  if (grid.dx != dbzFhdr.grid_dx) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in dx: clutter file dx = " << dbzFhdr.grid_dx
	 << ", lookup table dx = " << grid.dx << endl;
    iret = -1;
  }

  if (grid.dy != dbzFhdr.grid_dy) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in dy: clutter file dy = " << dbzFhdr.grid_dy
	 << ", lookup table dy = " << grid.dy << endl;
    iret = -1;
  }

  if (grid.dz != dbzFhdr.grid_dz) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in dz: clutter file dz = " << dbzFhdr.grid_dz
	 << ", lookup table dz = " << grid.dz << endl;
    iret = -1;
  }

  if (grid.minx != dbzFhdr.grid_minx) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in minx: clutter file minx = " << dbzFhdr.grid_minx
	 << ", lookup table minx = " << grid.minx << endl;
    iret = -1;
  }

  if (grid.miny != dbzFhdr.grid_miny) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in miny: clutter file miny = " << dbzFhdr.grid_miny
	 << ", lookup table miny = " << grid.miny << endl;
    iret = -1;
  }

  if (grid.minz != dbzFhdr.grid_minz) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in minz: clutter file minz = " << dbzFhdr.grid_minz
	 << ", lookup table minz = " << grid.minz << endl;
    iret = -1;
  }

  if (grid.latitude != dbzFhdr.proj_origin_lat) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in lat: clutter file lat = " << dbzFhdr.proj_origin_lat
	 << ", lookup table lat = " << grid.latitude << endl;
    iret = -1;
  }

  if (grid.longitude != dbzFhdr.proj_origin_lon) {
    cerr << "ERROR - ClutterTableGenerate::_checkGeom" << endl;
    cerr << "  Mismatch in lon: clutter file lon = " << dbzFhdr.proj_origin_lon
	 << ", lookup table lon = " << grid.longitude << endl;
    iret = -1;
  }

  return iret;

}

///////////////////
// write the table

int ClutterTableGenerate::_writeTable(const Mdvx &mdvx,
				      const P2mdv_lookup_file_index_t &lut)
  
{

  // initialize

  int list_offset = 0;

  // compute size of the file headers

  P2mdv_lookup_params_t *lut_params = lut.lookup_params;

  int nelevations = lut_params->nelevations;
  int nazimuths = lut_params->nazimuths;
  int nbeams = nazimuths * nelevations;

  // nplanes = lut_params->grid.nz;
  // nheights = nplanes * N_PLANE_HEIGHT_VALUES;
  
  int headers_size = (DS_FILE_LABEL_LEN +
		      sizeof(clut_table_params_t) +
		      nelevations * sizeof(si32));
  
  int index_size = nbeams * sizeof(clut_table_index_t);

  // make sure directory exists

  Path path(_params.clutter_table_path);
  if (path.makeDirRecurse()) {
    cerr << "ERROR - ClutterTableGenerate::_writeTable" << endl;
    cerr << "  Cannot create directory for path: "
	 << _params.clutter_table_path << endl;
    return -1;
  }
  
  // open clutter table file

  
  FILE *clut_file;
  if ((clut_file = fopen(_params.clutter_table_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - ClutterTableGenerate::_writeTable\n");
    fprintf(stderr, "Cannot open clutter table file for writing.\n");
    perror(_params.clutter_table_path);
    return -1;
  }

  if (_params.debug) {
    cerr << "Writing clutter table: " << _params.clutter_table_path << endl;
  }

  // space fwd over area for clut_table_params header
  // and the clut_table_index array
  
  fseek(clut_file, headers_size + index_size, SEEK_SET);

  // allocate space for the clut_table_index array
  
  clut_table_index_t **clut_table_index = (clut_table_index_t **)
    umalloc2 (nelevations, nazimuths, sizeof(clut_table_index_t));

  // compute the dbz margin in terms of the byte value

  const MdvxField &dbzFld = *mdvx.getFieldByName(_params.dbz_field_name);
  const Mdvx::field_header_t &dbzFhdr = dbzFld.getFieldHeader();
  ui08 *dbz_data = (ui08 *) dbzFld.getVol();
  double dbz_bias = dbzFhdr.bias;
  double dbz_scale = dbzFhdr.scale;
  
  int dbz_scaled_margin = (int) (_params.dbz_margin / dbz_scale + 0.5);
  int dbz_scaled_threshold =
    (int) ((_params.dbz_threshold - dbz_bias) / dbz_scale + 0.5);

  // loop through elevations and azimuths
  
  P2mdv_lookup_index_t **lut_index = lut.lookup_index;
  
  for (int ielev = 0; ielev < nelevations; ielev++) {
    
    for (int iaz = 0; iaz < nazimuths; iaz++) {

      int nclut_points = 0;

      // loop through cartesian points
      
      int ncart_points = lut_index[ielev][iaz].npoints;
      P2mdv_lookup_entry_t *lut_entry = lut_index[ielev][iaz].u.entry;
      
      for (int ipoint = 0; ipoint < ncart_points; ipoint++)  {

	int field_index = lut_entry->index;

	if ((int) dbz_data[field_index] > dbz_scaled_threshold) {

	  nclut_points++;
	  clut_table_entry_t clutter_entry;
	  clutter_entry.ipoint = ipoint;
	  clutter_entry.cart_index = field_index;
	  clutter_entry.dbz =
	    dbz_data[field_index] + dbz_scaled_margin;
	  if (clutter_entry.dbz > 255) clutter_entry.dbz = 255;

	  // put into network byte order

	  BE_from_array_16(&clutter_entry.dbz, sizeof(si16));
	  BE_from_array_16(&clutter_entry.ipoint, sizeof(si16));
	  BE_from_array_32(&clutter_entry.cart_index, sizeof(si32));
	  
	  if (ufwrite((char *) &clutter_entry, sizeof(clut_table_entry_t),
		      1, clut_file) != 1) {

	    fprintf(stderr, "ERROR - ClutterTableGenerate::_writeTable\n");
	    fprintf(stderr, "Writing clutter entry.\n");
	    perror(_params.clutter_table_path);
	    fclose(clut_file);
	    ufree2((void **) clut_table_index);
	    return -1;

	  } // if (ufwrite....

	}

	lut_entry++;

      } // ipoint

      // load index values

      clut_table_index[ielev][iaz].nclut_points = nclut_points;
      clut_table_index[ielev][iaz].u.offset = list_offset;

      // put into network byte order

      BE_from_array_32((ui32 *) &clut_table_index[ielev][iaz].u.offset,
		       (ui32) sizeof(si32));
      BE_from_array_32((ui32 *) &clut_table_index[ielev][iaz].nclut_points,
		       (ui32) sizeof(si32));
      
      // update list offset

      list_offset += nclut_points * sizeof(clut_table_entry_t);

    } // iaz

  } // ielev

  // load up clutter table params structure

  clut_table_params_t clut_table_params;
  clut_table_params.nbytes_char = lut_params->nbytes_char;
  clut_table_params.file_time = time(NULL);

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  clut_table_params.start_time = mhdr.time_begin;
  clut_table_params.mid_time = mhdr.time_centroid;
  clut_table_params.end_time = mhdr.time_end;

  clut_table_params.dbz_scale = dbzFhdr.scale;
  clut_table_params.dbz_bias = dbzFhdr.bias;
  clut_table_params.dbz_margin = _params.dbz_margin;

  clut_table_params.nlist = list_offset;
  clut_table_params.index_offset = headers_size;
  
  clut_table_params.list_offset =
    clut_table_params.index_offset + index_size;

  clut_table_params.lookup_params = *lut.lookup_params;
  
  // put params into network byte order
  
  BE_from_array_32(&clut_table_params, 
		   sizeof(clut_table_params_t) - lut_params->nbytes_char);
  
  // set file label
  
  char file_label[DS_FILE_LABEL_LEN];
  MEM_zero(file_label);
  strncpy(file_label, CLUTTER_TABLE_LABEL, DS_FILE_LABEL_LEN);

  // seek to start of file

  fseek(clut_file, (si32) 0, SEEK_SET);

  // write file label
  
  if (ufwrite(file_label,
	      sizeof(char),
	      DS_FILE_LABEL_LEN,
	      clut_file) != DS_FILE_LABEL_LEN) {
    fprintf(stderr, "ERROR - ClutterTableGenerate::_writeTable\n");
    fprintf(stderr, "Writing clutter file label.\n");
    perror(_params.clutter_table_path);
    fclose(clut_file);
    return -1;
  }
  
  // write params

  if (ufwrite((char *) &clut_table_params,
	      sizeof(clut_table_params_t),
	      1, clut_file) != 1) {
    fprintf(stderr, "ERROR - ClutterTableGenerate::_writeTable\n");
    fprintf(stderr, "Writing clutter file params.\n");
    perror(_params.clutter_table_path);
    fclose(clut_file);
    ufree2((void **) clut_table_index);
    return -1;
  }

  // code the radar_elevations array into network
  // byte order and write to file

  TaArray<fl32> relev_;
  fl32 *radar_elevations = relev_.alloc(nelevations);
  for (int i = 0; i < nelevations; i++) {
    radar_elevations[i] = lut.scan_table->elev_angles[i];
  }
  BE_from_array_32(radar_elevations, nelevations * sizeof(fl32));
  
  if (ufwrite(radar_elevations, sizeof(fl32),
	      nelevations, clut_file) != nelevations) {
    fprintf(stderr, "ERROR - ClutterTableGenerate::_writeTable\n");
    fprintf(stderr, "Writing radar elevations.\n");
    perror(_params.clutter_table_path);
    fclose(clut_file);
    ufree2((void **) clut_table_index);
    return -1;
  }

  // write index
  
  for (int ielev = 0; ielev < nelevations; ielev++) {
    
    if (ufwrite(clut_table_index[ielev], sizeof(clut_table_index_t),
		nazimuths, clut_file) != nazimuths) {
      fprintf(stderr, "ERROR - ClutterTableGenerate::_writeTable\n");
      fprintf(stderr, "Writing index, elen num %ld.\n", (long) ielev);
      perror(_params.clutter_table_path);
      ufree2((void **) clut_table_index);
      fclose(clut_file);
      return -1;
    }
    
  } // ielev

  // clean up

  ufree2((void **) clut_table_index);
  fclose(clut_file);

  return 0;

}
