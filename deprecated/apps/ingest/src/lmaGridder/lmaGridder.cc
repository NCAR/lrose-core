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
// lmaGridder.hh
//
// lmaGridder object
//
//
///////////////////////////////////////////////////////////////

#include "lmaGridder.hh"
#include <toolsa/pmu.h>
#include <stdlib.h>
#include <cmath>

// constructor.
lmaGridder::lmaGridder (Params *TDRP_params){
  //
  _count = 0;
  _endTime = 0L;
  _params = TDRP_params;
  //
  // Allocate memory for the grids.
  //
  _twoDdata = (float *) malloc(sizeof(float) * 
			       _params->output_grid.nx * 
			       _params->output_grid.ny);
 
  _threeDdata = (float *) malloc(sizeof(float) * 
				 _params->output_grid.nx * 
				 _params->output_grid.ny *
				 _params->output_grid.nz);


  

  if (
      (_twoDdata == NULL) ||
      (_threeDdata == NULL)
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
  memset(&_threeDfhdr, 0, sizeof(_threeDfhdr));
  //
  memset(&_twoDvhdr, 0, sizeof(_twoDvhdr));
  memset(&_threeDvhdr, 0, sizeof(_threeDvhdr));
  //
  memset(&_Mhdr, 0, sizeof(_Mhdr));
  //
  // Set up the vlevel headers.
  //
  _twoDvhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  _twoDvhdr.level[0] = 0.0;
  //
  for (int iz = 0; iz < _params->output_grid.nz; iz++) {
    _threeDvhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    _threeDvhdr.level[iz] = _params->output_grid.minz + iz * _params->output_grid.dz;
  }
  //
  // Then the field headers.
  //
  _twoDfhdr.nx = _params->output_grid.nx; _threeDfhdr.nx = _twoDfhdr.nx;
  _twoDfhdr.ny = _params->output_grid.ny; _threeDfhdr.ny = _twoDfhdr.ny;
  _twoDfhdr.nz = 1; _threeDfhdr.nz = _params->output_grid.nz;
  //
  _twoDfhdr.grid_dx = _params->output_grid.dx; _threeDfhdr.grid_dx = _twoDfhdr.grid_dx;
  _twoDfhdr.grid_dy = _params->output_grid.dy; _threeDfhdr.grid_dy = _twoDfhdr.grid_dy;
  _threeDfhdr.grid_dz = _params->output_grid.dz;
  //
  _twoDfhdr.grid_minx = _params->output_grid.minx; _threeDfhdr.grid_minx = _twoDfhdr.grid_minx;
  _twoDfhdr.grid_miny = _params->output_grid.miny; _threeDfhdr.grid_miny = _twoDfhdr.grid_miny;
  _threeDfhdr.grid_minz = _params->output_grid.minz;
  //
  if (_params->output_projection == Params::OUTPUT_PROJ_FLAT){
    _twoDfhdr.proj_type = Mdvx::PROJ_FLAT;
    _threeDfhdr.proj_type = Mdvx::PROJ_FLAT;
  } else {
    _twoDfhdr.proj_type = Mdvx::PROJ_LATLON;
    _threeDfhdr.proj_type = Mdvx::PROJ_LATLON;
  }
  //
  _twoDfhdr.proj_origin_lat =  _params->output_origin.lat;
  _twoDfhdr.proj_origin_lon =  _params->output_origin.lon;
  _threeDfhdr.proj_origin_lat =  _params->output_origin.lat;
  _threeDfhdr.proj_origin_lon =  _params->output_origin.lon;
  //
  if (_params->takeZeroAsBadValue){
    _twoDfhdr.bad_data_value = 0.0;   _twoDfhdr.missing_data_value = 0.0;
    _threeDfhdr.bad_data_value = 0.0; _threeDfhdr.missing_data_value = 0.0;
  } else {
    _twoDfhdr.bad_data_value = -1.0;   _twoDfhdr.missing_data_value = -1.0;
    _threeDfhdr.bad_data_value = -1.0; _threeDfhdr.missing_data_value = -1.0;
  }
  //
  sprintf( _twoDfhdr.field_name_long,"%s", "counts2D");
  sprintf( _twoDfhdr.field_name,"%s", "counts2D");
  sprintf( _twoDfhdr.units,"%s", "counts");
  sprintf( _twoDfhdr.transform,"%s","none");
  //
  sprintf( _threeDfhdr.field_name_long,"%s", "counts3D");
  sprintf( _threeDfhdr.field_name,"%s", "counts3D");
  sprintf( _threeDfhdr.units,"%s", "counts");
  sprintf( _threeDfhdr.transform,"%s","none");
  //
  _twoDfhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _twoDfhdr.data_element_nbytes = sizeof(fl32);
  _twoDfhdr.volume_size = _twoDfhdr.nx * _twoDfhdr.ny * sizeof(fl32);
  _twoDfhdr.compression_type = Mdvx::COMPRESSION_NONE;
  _twoDfhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _twoDfhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  //
  _threeDfhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _threeDfhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _threeDfhdr.dz_constant = 1;
  _twoDfhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _twoDfhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _twoDfhdr.dz_constant = 1;
  //
  _threeDfhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _threeDfhdr.data_element_nbytes = sizeof(fl32);
  _threeDfhdr.volume_size = _threeDfhdr.nx * _threeDfhdr.ny * _threeDfhdr.nz * sizeof(fl32);
  _threeDfhdr.compression_type = Mdvx::COMPRESSION_NONE;
  _threeDfhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _threeDfhdr.scaling_type = Mdvx::SCALING_NONE;     
  //
  //
  // OK - set up the master header.
  //
  _Mhdr.data_dimension = 3;
  _Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  _Mhdr.num_data_times = 1;
  _Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  _Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  _Mhdr.n_fields = 2;
  _Mhdr.max_nx = _params->output_grid.nx;
  _Mhdr.max_ny = _params->output_grid.ny;
  _Mhdr.max_nz = _params->output_grid.nz; 
  //
  _Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _Mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  //
  sprintf(_Mhdr.data_set_info,"%s","3d ltg");
  sprintf(_Mhdr.data_set_name,"%s","3d ltg");
  sprintf(_Mhdr.data_set_source,"%s","LMA");
  //
  // The times are not yet set up - do that later, as volumes are output.
  //
  _Proj = new MdvxProj(_Mhdr, _twoDfhdr);
  if(_params->debug)
  {
    _Proj->print(cerr, true);
  }
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
void lmaGridder::_resetData(){
  _numAdded = 0;
  for (int i=0; i < _params->output_grid.nx; i++){
    for (int j=0; j < _params->output_grid.ny; j++){
      _twoDdata[j * _params->output_grid.nx + i] = 0.0;
      for (int k=0; k < _params->output_grid.nz; k++){
	
	_threeDdata[k * _params->output_grid.nx * _params->output_grid.ny +
		    j * _params->output_grid.nx + i] = 0.0;
      }
    }
  }

}


///////////////////////////////////////
//
// Method to add a point to the grid.
//
void lmaGridder::_incPoint(double lat,
			   double lon,
			   double alt){

  int ix, iy;
  if (_Proj->latlon2xyIndex(lat, lon, ix, iy) == 0){
    //
    _twoDdata[iy * _params->output_grid.nx + ix] += 1.0;
    //
    int iz = (int)rint(((alt/1000.0) - _params->output_grid.minz - _params->output_grid.dz/2.0)/_params->output_grid.dz);
    
    if ((iz < 0) || (iz > _params->output_grid.nz-1)) return;

     _threeDdata[ iz * _params->output_grid.ny * _params->output_grid.nx +
		  iy * _params->output_grid.nx + ix] += 1.0; 
       
     _numAdded++;

  }
  else if (_params->debug)
  {
    cerr << "Point: lat = " << lat << " lon = " << lon << " outside grid. Skipping." << endl;
  }
  
  return;
}
/////////////////////////////////////////////////
//
// Method to add an LMA entry to the grid.
//
// 
void  lmaGridder::addToGrid(double lat, 
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
    if (_params->mode == Params::ARCHIVE ||
	_params->mode == Params::TIME_INTERVAL ){
      _endTime = ( ( _params->outputInterval * 
		     (int)floor(double(entryTime)/double(_params->outputInterval)) ) +
		   _params->outputInterval );
    } else {
      _endTime = ( ( _params->outputInterval * 
		     (int)floor(double(time(NULL))/double(_params->outputInterval)) ) +
		   _params->outputInterval );
    }
    _entries.clear();
  }
  //
  // Do we need to output the grids?
  //

  if (_params->debug){
    cerr << "  GRIDDING :   entryTime : ";
    cerr << utimstr(entryTime);
    cerr << "   _endTime : " << utimstr(_endTime) << endl;
    cerr << "lat = " << lat << "  lon = " << lon << "  alt = " << alt << endl;
    cerr << endl;
  }

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
void lmaGridder::_outputGrids(){

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
    cerr << endl;
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
  _threeDfhdr.forecast_time = _Mhdr.time_centroid;
  //
  // Write out the grids.
  //
  DsMdvx outMdvx;

  outMdvx.setMasterHeader( _Mhdr ); 
  outMdvx.clearFields();

  MdvxField *twoDfield = new MdvxField(_twoDfhdr, _twoDvhdr, _twoDdata);    
  if (twoDfield->convertRounded(Mdvx::ENCODING_INT16,
			       Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "2D convert failed - I cannot go on.\n");
    exit(-1);
  }
  //
  // add field to mdvx object
  //
  outMdvx.addField(twoDfield);   
  //
  MdvxField *threeDfield = new MdvxField(_threeDfhdr, _threeDvhdr, _threeDdata);    
  if (threeDfield->convertRounded(Mdvx::ENCODING_INT16,
			       Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "3D convert failed - I cannot go on.\n");
    exit(-1);
  }
  //
  outMdvx.addField(threeDfield);
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
lmaGridder::~lmaGridder(){

  delete _Proj;
  free(_twoDdata);
  free(_threeDdata);

}

