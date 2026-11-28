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
// asdiGridder.cc
//
// asdiGridder object
//
//
///////////////////////////////////////////////////////////////


#include "asdiGridder.hh"

#include <Spdb/DsSpdb.hh>
#include <toolsa/gridLineConnect.hh>
#include <toolsa/pjg_flat.h>
#include <Mdv/DsMdvx.hh>
#include <rapformats/acPosVector.hh>
#include <toolsa/pmu.h>
#include <cstring> // for memset

using namespace std;
  
// constructor.
asdiGridder::asdiGridder (Params *P){
  //
  // Make copy of params.
  //
  _params = P;
  //
  //
  // Allocate memory for the grids.
  //
  _twoDdata = (float *) malloc(sizeof(float) *
                               _params->output_grid.nx *
                               _params->output_grid.ny);

  if (_params->output3D){
    _threeDdata = (float *) malloc(sizeof(float) *
				   _params->output_grid.nx *
				   _params->output_grid.ny *
				   _params->output_grid.nz);
  }

  if (
      (_twoDdata == NULL) ||
      ((_params->output3D) && (_threeDdata == NULL))
      ){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

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
  sprintf( _twoDfhdr.field_name_long,"%s", "tracks2D");
  sprintf( _twoDfhdr.field_name,"%s", "tracks2D");
  sprintf( _twoDfhdr.units,"%s", "altitude");
  sprintf( _twoDfhdr.transform,"%s","none");
  //
  sprintf( _threeDfhdr.field_name_long,"%s", "trackCounts3D");
  sprintf( _threeDfhdr.field_name,"%s", "trackCounts3D");
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
  if (_params->output3D){
    _Mhdr.data_dimension = 3;
  } else {
    _Mhdr.data_dimension = 2;
  }
  _Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  _Mhdr.num_data_times = 1;
  _Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  _Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  if (_params->output3D){
    _Mhdr.n_fields = 2;
  } else {
    _Mhdr.n_fields = 1;
  }

  _Mhdr.max_nx = _params->output_grid.nx;
  _Mhdr.max_ny = _params->output_grid.ny;
  _Mhdr.max_nz = _params->output_grid.nz;
  //
  _Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  _Mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  //
  sprintf(_Mhdr.data_set_info,"%s","ASDI data");
  sprintf(_Mhdr.data_set_name,"%s","ASDI data");
  sprintf(_Mhdr.data_set_source,"%s","ASDI data");

  return;
}
////////////////////////////////////////////////
//
// Small method to set data to zero.
//
void asdiGridder::_resetData(){
  //
  memset(_twoDdata, 0, sizeof(float) *  _params->output_grid.nx * _params->output_grid.ny);
  //
  if (!(_params->output3D)) return;
  //
  memset(_threeDdata, 0, sizeof(float) * _params->output_grid.nx * _params->output_grid.ny * _params->output_grid.nz);
  //
  return;
}


// 
void asdiGridder::makeGrid(time_t triggerTime){
  //
  time_t runStart=time(NULL);
  //
  //
  if (_params->debug)
    cerr << "Starting run for " << utimstr(triggerTime) << endl;
  //
  // Reset the output data.
  //
  if (_params->debug)
    cerr << "Resetting data buffer .... " << endl;
  _resetData();
  //
  // Get the data.
  //
  if (_params->debug){
    cerr << "Fetch starts " << utimstr(triggerTime - _params->lookBack);
    cerr << ", ends " << utimstr(triggerTime);
    cerr << " from " << _params->input_url << endl;
  }
  //
  PMU_auto_register("Processing data.");
  //
  DsSpdb asdiVecs;
  asdiVecs.getInterval( _params->input_url,
			triggerTime - _params->lookBack,
			triggerTime );
  if (_params->debug){
    cerr << asdiVecs.getNChunks() << " vectors found at ";
    cerr << utimstr( triggerTime ) << endl;
  }
  //
  // Loop through the chunks, disassemble them into
  // vectors, and plonk them into the oputput grid.
  //
  if (_params->debug)
    cerr << "Processing data ...." << endl;
  //
  MdvxProj Proj(_Mhdr, _twoDfhdr);
  int numQCfails = 0;
  for (int i=0; i <  asdiVecs.getNChunks(); i++){
    acPosVector A;
    //
    if (0 != A.disassemble(asdiVecs.getChunks()[i].data,
			   asdiVecs.getChunks()[i].len)){
      cerr << "Dissassembly failed for point " << i << endl;
      continue;
    }
    //
    // Register occasionally.
    //
    if (i % 100 == 0){
      PMU_auto_register("Processing away...");
      if (_params->debug){
	cerr << endl << "VECTOR : " << i << " of " << asdiVecs.getNChunks() << endl;
	A.print(cerr);
      }
    }
    //
    // If one point is outside of the grid, forget it.
    //
    int startIX, startIY;
    int endIX, endIY;
    if ((Proj.latlon2xyIndex(A.getLatPrevious(), A.getLonPrevious(), startIX, startIY)) ||
	(Proj.latlon2xyIndex(A.getLatCurrent(),  A.getLonCurrent(),  endIX,   endIY))){
      continue; // Point outside of grid.
    }
    //
    // Apply QC tests, if desired.
    //
    if (_params->applyQCtests){
      //
      // Altitude. Convert from FL to Km.
      //
      if (
	  (A.getAltPrevious() * 0.03 > _params->maxAlt) ||
	  (A.getAltCurrent()  * 0.03 > _params->maxAlt)
	  ){
	if (_params->debug){
	  cerr << "Alt 1 : " << A.getAltPrevious();
	  cerr << " Alt 2 : " << A.getAltCurrent();
	  cerr << " failed alt QC test." << endl;
	}
	numQCfails++;
	continue;
      }
      //
      // Distance.
      //
      double dist, dir;
      PJGLatLon2RTheta(A.getLatPrevious(), A.getLonPrevious(),
		       A.getLatCurrent(),  A.getLonCurrent(),
                       &dist, &dir);
      if (dist > _params->maxDist){
	if (_params->debug){
	  cerr << "Distance : " << dist;
	  cerr << " failed distance QC test." << endl;
	}
	numQCfails++;
	continue;
      }
      //
      // Speed.
      //
      double timeInHours = double(A.getTimeCurrent() - A.getTimePrevious())/3600.0;
      if (timeInHours == 0.0){
	if (_params->debug){
	  cerr << "Two reports at same time, speed infinite, ignoring." << endl;
	}
	numQCfails++;
	continue;
      }
      double speed = dist / timeInHours;
      if (speed > _params->maxSpeed){
	if (_params->debug){
	  cerr << "Speed : " << speed;
	  cerr << " failed speed QC test." << endl;
	}
	numQCfails++;
	continue;
      }
    } // Done with QC tests.
    //
    // At least one point is in the grid, add it to the grid.
    //
    gridLineConnect G(startIX, startIY, endIX, endIY);
    int morePoints;
    do {
      //
      int ix, iy;
      double t;      
      morePoints = G.nextPoint(ix, iy, t);
      //
      // If we are inside the grid, get the altitude
      // and add this to the grid.
      //
      if (
	  (ix > -1) && (iy > -1) &&
	  (ix < _params->output_grid.nx) &&
	  (iy < _params->output_grid.ny)
	  ){
	//
	// Use linear interpolation to get the altitude.
	//
	double pointAlt = -1000.0;
	//
	if ((A.getAltCurrent() != acPosVector::missingVal) &&
	    (A.getAltPrevious() != acPosVector::missingVal)){
	  pointAlt = t*A.getAltCurrent() + (1.0-t)*A.getAltPrevious();
	} else {
	  if (A.getAltCurrent() != acPosVector::missingVal)
	    pointAlt = A.getAltCurrent();
	  //
	  if (A.getAltPrevious() != acPosVector::missingVal)
	    pointAlt = A.getAltPrevious();
	}
	//
	// Skip it if we can't get altitude.
	//
	if (pointAlt < 0.0) continue;
	//
	// Convert altitude units from FL to Km if requested.
	//
	if (_params->altsInKm){
	  pointAlt *= 0.03;
	}
	//
	_addToGrid(ix, iy, pointAlt);
	//
      }
    } while (morePoints);
  }
  //
  // Output the grids.
  //
  // Set the times in the master and field headers.
  //
  _Mhdr.time_gen = time(NULL);
  _Mhdr.time_begin = triggerTime - _params->lookBack;
  _Mhdr.time_end = triggerTime;
  _Mhdr.time_expire = triggerTime;

  switch (_params->timestamp){

  case Params::TIMESTAMP_START :
    _Mhdr.time_centroid = _Mhdr.time_begin;
    break;

  case Params::TIMESTAMP_MIDDLE :
    _Mhdr.time_centroid = _Mhdr.time_end - _params->lookBack/2;
    break;

  case Params::TIMESTAMP_END :
    _Mhdr.time_centroid = _Mhdr.time_end;
    break;

  default :
    cerr << "Invalid timestamping option." << endl;
    exit(-1);
    break;
  }

  DsMdvx outMdvx;

  outMdvx.setMasterHeader( _Mhdr );
  outMdvx.clearFields();

  MdvxField *twoDfield = new MdvxField(_twoDfhdr, _twoDvhdr, _twoDdata);
  if (twoDfield->convertRounded(Mdvx::ENCODING_INT16,
                               Mdvx::COMPRESSION_ZLIB)){
    cerr << "2D convert failed - I cannot go on." << endl;
    exit(-1);
  }

  outMdvx.addField(twoDfield);

  if (_params->output3D){
    MdvxField *threeDfield = new MdvxField(_threeDfhdr, _threeDvhdr, _threeDdata);
    if (threeDfield->convertRounded(Mdvx::ENCODING_INT16,
				    Mdvx::COMPRESSION_ZLIB)){
      cerr << "3D convert failed - I cannot go on." << endl;
      exit(-1);
    }
    //
    outMdvx.addField(threeDfield);
  }

 //
  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }
  //
  time_t runEnd = time(NULL);
  //
  if (_params->debug){
    cerr << "Run took " << runEnd - runStart << " seconds." << endl;
    if (_params->applyQCtests){
      cerr << numQCfails << " quality control failures " << endl;
      double pfail = double(100.0*numQCfails)/double(asdiVecs.getNChunks());
      cerr << pfail << " percent." << endl;
    }
  }
  //
  return;
}
////////////////////////////////////////////////////////////////
//
// Add to the grid at a point.
//
void asdiGridder::_addToGrid(int ix, int iy, double pointAlt){

  int twoDindex = iy * _params->output_grid.nx + ix;
  //
  // Add this to the 2D grid.
  //
  if (_twoDdata[twoDindex] == 0.0){
    //
    // No other tack has been here, just accept the value.
    //
    _twoDdata[twoDindex] = pointAlt;
  } else {
    //
    if ((_params->takeMaximumAlt) && (pointAlt > _twoDdata[twoDindex]))
      _twoDdata[twoDindex] = pointAlt;
    //
    if (!((_params->takeMaximumAlt)) && (pointAlt < _twoDdata[twoDindex]))
      _twoDdata[twoDindex] = pointAlt;
    //
  }
  //
  // If we are not doing 3D data, we're done.
  //
  if (!(_params->output3D)) return;
  //
  // Increment the point in 3D space.
  //
  int iz = (int)rint( (pointAlt - _params->output_grid.minz - _params->output_grid.dz/2.0) 
		      / _params->output_grid.dz );

  if ((iz < 0) || (iz > _params->output_grid.nz-1)) return; // Out of bounds.

  int threeDindex = iz * _params->output_grid.nx * _params->output_grid.ny +
    iy * _params->output_grid.nx + ix;

  _threeDdata[threeDindex] += 1.0;

  return;
}
  
// destructor.
asdiGridder::~asdiGridder(){
  free(_twoDdata);
  if (_params->output3D) free(_threeDdata);
  return;
}


