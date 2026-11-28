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


#include "setOfPointsMgr.hh"
#include <Mdv/DsMdvx.hh>
#include <toolsa/umisc.h>
#include <rapformats/GenPt.hh>
#include <Mdv/MdvxField.hh>

//
// Constructor. Reads the stations and pushes
// them back into the correct vector.
//
setOfPointsMgr::setOfPointsMgr(Params *TDRP_params){
  _params = TDRP_params;
  _setsOfPoints.clear();
  _lastPrintT=0.0;
  _lastWriteT=0;
  _first = true;

  return;

}
//
//
// Method to add a point. Determines if we are done and if
// we should save.
//
void setOfPointsMgr::addPoint(double lat,
			      double lon,
			      double alt,
			      double t){

  time_t outputTime;
  switch( _params->timeStamp ){

    case Params::END :
    outputTime = _lastWriteT + _params->outputInterval;
    break;

    case Params::BEGIN :
    outputTime = _lastWriteT;
    break;

    case Params::MIDDLE :
    outputTime = _lastWriteT + double(_params->outputInterval) / 2.0;
    break;
  }
  

  //
  // If it's out first go-around, we need to set up
  // the timing.
  //
  if (_first){
    _first = false;
    _lastWriteT = _params->outputInterval*(int)floor(double(t)/double(_params->outputInterval));
    if(_params->debug)
    {
      cerr << endl;
      cerr << "Last Write Time = " << utimstr(_lastWriteT) << endl;
      cerr << endl;
    }
  }

  //
  // See if the time has advanced enough that we need to write output.
  //
  if (t > _lastWriteT + _params->outputInterval){
    if(_params->debug)
    {
      cerr << endl;
      cerr << "Writing output for time: " << utimstr(_lastWriteT + _params->outputInterval) << endl;
      cerr << endl;
    }

    _outPut(outputTime);
    _lastWriteT += _params->outputInterval;
  }

  //
  // In REALTIME also do output if clock has turned
  //
  if ((_params->mode == Params::REALTIME) && (time(NULL) > _lastWriteT + _params->outputInterval)){
    _outPut(outputTime);
    _lastWriteT += _params->outputInterval;
  }

  //
  // Do some debugging, if needed.
  //
  if (_params->debug){
    if (t-_lastPrintT > 5){
      cerr << "At " << utimstr((time_t)t) << " ";
      cerr  << _setsOfPoints.size() << " bundles are active." << endl;
      _lastPrintT = t;
    }
  }

  //
  // See if we can add this to an existing string.
  //
  for (unsigned i=0; i < _setsOfPoints.size(); i++){
    //
    int retVal = _setsOfPoints[i]->addPoint(lat,lon, alt,t);
    //
    if (retVal == 0) return; // Added point to an existing set. Done.
    //
  }
  //
  // OK - if we got to here, we did not
  // add this point to an existing bundle.
  // Create a new bundle.
  //
  setOfPoints *S = new setOfPoints(_params, lat, lon, alt, t);
  //
  _setsOfPoints.push_back( S );
  //
  return;
  //
}


void setOfPointsMgr::_outPut(time_t dataTime){


  //
  // Set up Master, field, vlevel headers
  //

 //
  // Prepare to write MDV files. Declare a master, field
  // and vlevel header.
  //
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t Mhdr;
  //
  // Use 'memset' to set all the bytres in these structures to 0.
  //
  memset(&fhdr, 0, sizeof(fhdr));
  memset(&vhdr, 0, sizeof(vhdr));
  memset(&Mhdr, 0, sizeof(Mhdr));
  //
  // Set up the vlevel header. This is pretty simple as its is
  // just surface data.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nx = _params->gridDef.nx;
  fhdr.ny = _params->gridDef.ny;
  fhdr.nz = 1;
  //
  fhdr.grid_dx = _params->gridDef.dx;
  fhdr.grid_dy = _params->gridDef.dy;
  //

 
  //
  fhdr.proj_origin_lat =  _params->gridDef.latOrigin;
  fhdr.proj_origin_lon =  _params->gridDef.lonOrigin;
  //
  //
  //
  if (_params->useLatlon){
    fhdr.grid_minx = _params->gridDef.minx;
    fhdr.grid_miny = _params->gridDef.miny;
     fhdr.proj_type = Mdvx::PROJ_LATLON;
  } else {
    fhdr.grid_minx = _params->gridDef.minx;
    fhdr.grid_miny = _params->gridDef.miny;
    fhdr.proj_type = Mdvx::PROJ_FLAT;
  }
  //
  // Set up an uncompressed grid of floating point values.
  //
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  //
  // State what vlevel type we have.
  //
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 0;

  if (_params->useZeroForMissing)
    fhdr.bad_data_value =  0.0;   
  else
    fhdr.bad_data_value = -1.0;   

  fhdr.missing_data_value = fhdr.bad_data_value;
  //
  sprintf( fhdr.field_name_long,"%s", "Ltg flash extent");
  sprintf( fhdr.field_name,"%s", "flashExtent");
  sprintf( fhdr.units,"%s", "none");
  sprintf( fhdr.transform,"%s","none");

  //
  // Set up some things in the master header.
  //
  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  Mhdr.vlevel_included = 1;
  //
  //
  sprintf(Mhdr.data_set_info,"%s","ltgFlashExtent");
  sprintf(Mhdr.data_set_name,"%s","ltgFlashExtent");
  sprintf(Mhdr.data_set_source,"%s", "ltgFlashExtent");
  //
  // Set the times in the master and field headers.
  //

  Mhdr.time_gen = dataTime;
  Mhdr.time_begin = dataTime;
  Mhdr.time_end = dataTime;
  Mhdr.time_expire = dataTime;
  Mhdr.time_centroid = dataTime;
  //
  fhdr.forecast_time = Mhdr.time_centroid;
  //
  fhdr.forecast_delta = 0;
  //

  fl32 *data = (fl32 *)malloc(sizeof(fl32)*fhdr.nx*fhdr.ny);
  if (data == NULL){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  for (int i=0; i < fhdr.nx*fhdr.ny; i++){
    data[i] = fhdr.bad_data_value;
  }


  for (unsigned i=0; i < _setsOfPoints.size(); i++){

    //
    // Skip those with too few points.
    //
    if (_setsOfPoints[i]->getSize() < _params->minNumEntries) continue;

    //
    // For the other bundles, add their points into the data.
    //
    int np = _setsOfPoints[i]->getNumPairs();
    for (int k=0; k < np; k++){
      int index = _setsOfPoints[i]->getNthY(k)*fhdr.nx + _setsOfPoints[i]->getNthX(k);

      if (data[index] ==   fhdr.bad_data_value)
	data[index] = 1.0;
      else
	data[index] = data[index] + 1.0;
	  
    }

  }


  // Normalize, if requested.
  if ((_params->normSpatial) || (_params->normTemporal)){
    for (long l=0; l < fhdr.nx*fhdr.ny; l++){
      if (data[l] !=  fhdr.bad_data_value){
	if (_params->normSpatial)  data[l] = data[l] / (fhdr.grid_dx*fhdr.grid_dy);
	if (_params->normTemporal) data[l] = data[l] / _params->outputInterval;
      }
    }
  }

  //
  // Declare a DsMdvx object so we can load it up with fields.
  //
  DsMdvx outMdvx;
  //
  outMdvx.setMasterHeader( Mhdr );

  MdvxField *field;
  //
  field = new MdvxField(fhdr, vhdr, data);
  //
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }
  
  outMdvx.addField(field);
  
  //
  // Finally, now that the DsMdvx object is all loaded up, use it
  // to write out the data.
  //
  if (outMdvx.writeToDir(_params->output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->output_url << endl;
    cerr << outMdvx.getErrStr() << endl;
  }

  free(data);

  for (unsigned i=0; i < _setsOfPoints.size(); i++){
    delete _setsOfPoints[i];
  }
  _setsOfPoints.clear();

  return;
}

//
// Destructor. Cleans out the vector.
//
setOfPointsMgr::~setOfPointsMgr(){

  for (unsigned i=0; i < _setsOfPoints.size(); i++){
    delete _setsOfPoints[i];
  }
  _setsOfPoints.clear();

  return;

}
