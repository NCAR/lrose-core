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
/////////////////////////////////////////////////////////////
// Gridder.cc
//
// Gridder object
//
//
///////////////////////////////////////////////////////////////

#include "Gridder.hh"
#include <toolsa/pmu.h>
  
// constructor.
Gridder::Gridder (Params *TDRP_params){
  //
  _count = 0;
  _endTime = 0L;
  _params = TDRP_params;
  //
  // Allocate memory for the grids.
  //
  _twoDdataAlt = (float *) malloc(sizeof(float) * 
			       _params->output_grid.nx * 
			       _params->output_grid.ny);
  _twoDdataDensity = (float *) malloc(sizeof(float) * 
				      _params->output_grid.nx * 
				      _params->output_grid.ny);
  if (
      (_twoDdataAlt == NULL) ||
      (_twoDdataDensity == NULL)
      ){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  _resetData();
  _firstPoint = true;

  //
  // Set up the master, vlevel and field headers.
  //
  memset(&_twoDfhdr, 0, sizeof(_twoDfhdr));
  //
  memset(&_twoDvhdr, 0, sizeof(_twoDvhdr));
  //
  memset(&_Mhdr, 0, sizeof(_Mhdr));
  //
  // Set up the vlevel headers.
  //
  _twoDvhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  _twoDvhdr.level[0] = 0.0;
  //
  //
  // Then the field headers.
  //
  _twoDfhdr.nx = _params->output_grid.nx;
  _twoDfhdr.ny = _params->output_grid.ny;
  _twoDfhdr.nz = 1;
  //
  _twoDfhdr.grid_dx = _params->output_grid.dx;
  _twoDfhdr.grid_dy = _params->output_grid.dy;
  //
  _twoDfhdr.grid_minx = _params->output_grid.minx;
  _twoDfhdr.grid_miny = _params->output_grid.miny;
  //
  if (_params->output_projection == Params::OUTPUT_PROJ_FLAT){
    _twoDfhdr.proj_type = Mdvx::PROJ_FLAT;
  } else {
    _twoDfhdr.proj_type = Mdvx::PROJ_LATLON;
  }
  //
  _twoDfhdr.proj_origin_lat =  _params->output_origin.lat;
  _twoDfhdr.proj_origin_lon =  _params->output_origin.lon;
  //
  if (_params->takeZeroAsBadValue){
    _twoDfhdr.bad_data_value = 0.0;   _twoDfhdr.missing_data_value = 0.0;
  } else {
    _twoDfhdr.bad_data_value = -1.0;   _twoDfhdr.missing_data_value = -1.0;
  }
  //
  //
  //
  _twoDfhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _twoDfhdr.data_element_nbytes = sizeof(fl32);
  _twoDfhdr.volume_size = _twoDfhdr.nx * _twoDfhdr.ny * sizeof(fl32);
  _twoDfhdr.compression_type = Mdvx::COMPRESSION_NONE;
  _twoDfhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _twoDfhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  //
  _twoDfhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _twoDfhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _twoDfhdr.dz_constant = 1;
  //
  //
  //
  // OK - set up the master header.
  //
  _Mhdr.data_dimension = 2;
  _Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  _Mhdr.num_data_times = 1;
  _Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  _Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  _Mhdr.n_fields = 2;
  _Mhdr.max_nx = _params->output_grid.nx;
  _Mhdr.max_ny = _params->output_grid.ny;
  _Mhdr.max_nz = 1; 
  //
  _Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _Mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  //
  sprintf(_Mhdr.data_set_info,"%s","ASDI");
  sprintf(_Mhdr.data_set_name,"%s","ASDI");
  sprintf(_Mhdr.data_set_source,"%s","ASDI");
  //
  // The times are not yet set up - do that later, as volumes are output.
  //
  _Proj = new MdvxProj(_Mhdr, _twoDfhdr);
  //
  return;
  //
}
/////////////////////////////////////////
//
// Method to set data grids to 0.0
// Could use memset to speed things up, although I find that a bit
// non-portable.
//
void Gridder::_resetData(){
  _numAdded = 0;
  for (int i=0; i < _params->output_grid.nx; i++){
    for (int j=0; j < _params->output_grid.ny; j++){
      _twoDdataAlt[j * _params->output_grid.nx + i] = 0.0;
      _twoDdataDensity[j * _params->output_grid.nx + i] = 0.0;
    }
  }

}


///////////////////////////////////////
//
// Method to add a point to the grid.
//
void Gridder::_incPoint(double lat,
			double lon,
			double alt){

  int ix, iy;
  if (!_Proj->latlon2xyIndex(lat, lon, ix, iy)){
    //
    // Increment the density.
    //
    _twoDdataDensity[iy * _params->output_grid.nx + ix] += 1.0;
    //
    // Put the altitude in the grid.
    //
    if (_twoDdataAlt[iy * _params->output_grid.nx + ix] == 0.0){
      //
      // First entry at this grid point, just overwrite.
      //
      _twoDdataAlt[iy * _params->output_grid.nx + ix] = alt;
    } else {
      //
      // It is not the first entry at this gridpoint.
      // See if we wish to take this entry or not.
      //
      if (
	  ((_params->takeMaxAltitude) && (alt >  _twoDdataAlt[iy * _params->output_grid.nx + ix])) ||
	  (!((_params->takeMaxAltitude)) && (alt <  _twoDdataAlt[iy * _params->output_grid.nx + ix]))
	  ) {
	_twoDdataAlt[iy * _params->output_grid.nx + ix] = alt;
      }
    }
    //      
    _numAdded++; 
  }
  return;
}
/////////////////////////////////////////////////
//
// Method to add an LMA entry to the grid.
//
// 
void  Gridder::addToGrid(double lat, 
			 double lon,
			 double alt,
			 time_t entryTime){
  if (_count == 50){
    PMU_auto_register("Adding ltg data point.");
    _count = 0;
  }
  _count++;
  //
  // Is this the first time we have been called? If so, set
  // the end time for this interval.
  //
  if (_firstPoint){
    _firstPoint = false;
    if (_params->mode == Params::ARCHIVE){
      _endTime = entryTime + _params->outputInterval;
    } else {
       _endTime = time(NULL) + _params->outputInterval;
    }
    _entries.clear();
  }
  //
  // Do we need to output the grids?
  //
  if (
      (entryTime > _endTime) ||
      (entryTime == _endTime)
      ){
    _outputGrids();
    do {
      _endTime += _params->outputInterval;
      if (_endTime < entryTime) _outputGrids(); // Catch up.
    } while (_endTime < entryTime);
  }
  //
  // Increment this point in the grid.
  //
  // _incPoint(lat, lon, alt);
  //
  _lma_entry_t l;
  l.lat = lat; l.lon = lon; l.alt = alt; l.time = entryTime;
  _entries.push_back(l);
  //
  return;
}
//
void Gridder::_outputGrids(){

  if (_params->debug){
    cerr << "Writing data for ";
    cerr << utimstr(_endTime - _params->lookbackInterval);
    cerr << " to " << utimstr(_endTime) << endl;
  }
  //
  // Get all the acceptable points into a new vector, nV.
  //
  vector <_lma_entry_t> nV;
  for (unsigned i=0; i < _entries.size(); i++){
    if (_entries[i].time >= _endTime - _params->lookbackInterval){
      nV.push_back(_entries[i]);
      _incPoint(_entries[i].lat, _entries[i].lon, _entries[i].alt);
    }
  }
  if (_params->debug){
    cerr << _numAdded << " points in grid." << endl;
  }
  //
  // Copy the valid entries back into the main vector.
  //
  _entries.clear();
  for (unsigned i=0; i < nV.size(); i++){
    _entries.push_back(nV[i]);
  }
  nV.clear(); // Basically done with nV

  //
  //
  // Set the times in the master and field headers.
  //
  _Mhdr.time_gen = time(NULL);
  _Mhdr.time_begin = _endTime - _params->lookbackInterval;
  _Mhdr.time_end = _endTime;
  _Mhdr.time_expire = _endTime + _params->Expiry;
  
  switch (_params->timestamp){

  case Params::TIMESTAMP_START :
    _Mhdr.time_centroid = _Mhdr.time_begin;
    break;

  case Params::TIMESTAMP_MIDDLE :
    _Mhdr.time_centroid = _Mhdr.time_end - _params->lookbackInterval/2;
    break;

  case Params::TIMESTAMP_END :
    _Mhdr.time_centroid = _Mhdr.time_end;
    break;

  default :
    cerr << "Invalid timestamping option." << endl;
    exit(-1);
    break;

  }

  _twoDfhdr.forecast_time = _Mhdr.time_centroid;
  //
  // Write out the grids.
  //
  DsMdvx outMdvx;

  outMdvx.setMasterHeader( _Mhdr ); 
  outMdvx.clearFields();
  //
  sprintf( _twoDfhdr.field_name_long,"%s", "asdiAlt");
  sprintf( _twoDfhdr.field_name,"%s", "asdiAlt");
  sprintf( _twoDfhdr.units,"%s", "FlightLevel");
  sprintf( _twoDfhdr.transform,"%s","none");
  //
  MdvxField *twoDfieldAlt = new MdvxField(_twoDfhdr, _twoDvhdr, _twoDdataAlt);    
  if (twoDfieldAlt->convertRounded(Mdvx::ENCODING_INT16,
				   Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "2D convert failed - I cannot go on.\n");
    exit(-1);
  }
  //
  // add field to mdvx object
  //
  outMdvx.addField(twoDfieldAlt);   
  //
  // 
  //
  sprintf( _twoDfhdr.field_name_long,"%s", "asdiDensity");
  sprintf( _twoDfhdr.field_name,"%s", "asdiDensity");
  sprintf( _twoDfhdr.units,"%s", "counts");
  sprintf( _twoDfhdr.transform,"%s","none");
  //
  MdvxField *twoDfieldDensity = new MdvxField(_twoDfhdr, _twoDvhdr, _twoDdataDensity);    
  if (twoDfieldDensity->convertRounded(Mdvx::ENCODING_INT16,
				       Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "2D convert failed - I cannot go on.\n");
    exit(-1);
  }
  //
  outMdvx.addField(twoDfieldDensity);   
  //
  //
  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }
  //
  _resetData();


  return;

}

// destructor.
Gridder::~Gridder(){

  delete _Proj;
  free(_twoDdataAlt);
  free(_twoDdataDensity);

}

