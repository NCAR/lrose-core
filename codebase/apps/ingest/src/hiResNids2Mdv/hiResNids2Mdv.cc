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
//
// This is the module of the hiResNids2Mdv application that does most
// of the work, reading the netCDF and writing the MDV files. Include
// both MDV and netCDF header files.
//
#include "hiResNids2Mdv.hh"
#include "lightweightDirSearch.hh"

#include <bzlib.h>
#include <cmath>

#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <dirent.h>

#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include "hiResRadialFile.hh"
//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
hiResNids2Mdv::hiResNids2Mdv(Params *P, int iradar, int ifield){
  _params = P;
  _iradar = iradar;
  _ifield = ifield;

  _radar =  _params->_radars[_iradar];
  _inFieldName = _params->_fields[_ifield].inFieldName;
  _outDirName = _params->_fields[_ifield].outDirName;
  _saveAsRadial = _params->_fields[_ifield].saveAsRadial;

  string pathDelim = "/";
  string indir = _params->topInDir;
  string outurl =  _params->topOutUrl;

  _inDir = indir + pathDelim + _radar + pathDelim + _inFieldName;
  _outUrl = outurl + pathDelim +  _outDirName + pathDelim + _radar;

  _specifiedElev = _params->_fields[_ifield].elev;

  if (_params->mode == Params::REALTIME){

    //
    // Sleep, if we are staggering startup. No
    // need to register with procmap as parent
    // process is doing that.
    //
    if (_params->stagger.staggerProcs){
      sleep(iradar *  _params->fields_n + ifield *_params->stagger.procStaggerSecs );
    }

    if (_params->lightweightFileSearch){

      lightweightDirSearch L((char *)_inDir.c_str(), _params->max_realtime_valid_age,
			     _params->debugRealtimeFileSearch, _params->dirScanSleep);

      while(1){
	char *filePath = L.nextFile();
	if (filePath != NULL) hiResNids2MdvFile( filePath );
      }

    } else {

      DsInputPath *inPath = new DsInputPath("hiResNids2Mdv",
					    _params->debugRealtimeFileSearch,
					    _inDir.c_str(),
					    _params->max_realtime_valid_age,
					    PMU_auto_register,
					    false, true);
      
      inPath->setDirScanSleep( _params->dirScanSleep );
      
      while (1){
	char *filePath = inPath->next();
	if (filePath != NULL) hiResNids2MdvFile( filePath );
      }

    }
  } else { // Archive mode, do all files under _inDir
    _procDir( _inDir );
  }

  return;
}

// Process all files under a top directory with scanning and recursion.
void hiResNids2Mdv::_procDir(string dirName){

  DIR *dirp = opendir( dirName.c_str() );
  if (dirp == NULL){
    fprintf(stderr, "Cannot open directory %s\n", dirName.c_str());
    perror(dirName.c_str());
    return;
  }

 struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    if ((dp->d_name[0] == '.' ) || (dp->d_name[0] == '_')) continue;
    string pathDelim = "/";
    string fileName(dp->d_name);
    string fullName = dirName + pathDelim + fileName;

    struct stat buf;
    if (stat(fullName.c_str(), &buf)) continue; // Not likely;

    if (S_ISDIR(buf.st_mode)){ // Directory, recurse into it
      _procDir( fullName );
    }

    if (S_ISREG(buf.st_mode)){ // Regular file, process it
      hiResNids2MdvFile( fullName.c_str() );
    }

  }

  closedir(dirp);


  return;
}


//
// Destructor - does nothing but avoids default destructor.
//
hiResNids2Mdv::~hiResNids2Mdv(){
  return;
}
//
// Main method
//
void hiResNids2Mdv::hiResNids2MdvFile( const char *FilePath ){

  //
  // If we are using threads, make sure we
  // are not exceeding the max number of threads
  // that are actively processing.
  //
  if ( (!(_params->forkIt)) && (_params->threadLimit.limitNumThreads) ){
    while (1){
      if (nt < _params->threadLimit.maxNumThreads) break;
      PMU_auto_register("Thread in wait mode while others process files");
      sleep(1); // Wait for another thread to finish processing and decrement the global nt
    }
    pthread_mutex_lock(&ntMutex);
    nt++;
    pthread_mutex_unlock(&ntMutex);
    if (_params->debug) cerr << "Acquired active thread number " << nt << " of " << _params->threadLimit.maxNumThreads << endl;
  }

  hiResRadialFile H(FilePath, _params, _params->debug);

  // Read the file.
  if (H.getNradials() == 0){
    cerr << " Problem reading file " << FilePath << ", skipping." << endl;
    //
    if ( (!(_params->forkIt)) && (_params->threadLimit.limitNumThreads) ){
      if (_params->debug) cerr << "Releasing active thread number " << nt << endl;
      pthread_mutex_lock(&ntMutex);  nt--;  pthread_mutex_unlock(&ntMutex);
    }
    return;
  }

  if (_specifiedElev > -30.0) H.setElevAngle( _specifiedElev );


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
  // Set up the vlevel header.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_ELEV;
  vhdr.level[0] = H.getElevAngle();
  //
  // Then set up the field header with the metadata.
  //
  fhdr.nz = 1;
  //
  //
  fhdr.proj_origin_lat =  H.getLat();
  fhdr.proj_origin_lon =  H.getLon();
  //
  // Set up an uncompressed grid of floating point values.
  //
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  //
  // State what vlevel type we have.
  //
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;

  fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.dz_constant = 0;
  //
  // Set up some things in the master header.
  //
  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  Mhdr.vlevel_included = 1;
  Mhdr.sensor_lat = H.getLat();
  Mhdr.sensor_lon = H.getLon();
  Mhdr.sensor_alt = H.getAlt();
  //
  sprintf(Mhdr.data_set_info,"%s","Level III data");
  sprintf(Mhdr.data_set_name,"%s","Level III data");
  sprintf(Mhdr.data_set_source,"%s", "Level III data");
  
  Mhdr.time_gen = H.getTime();
  Mhdr.time_begin = H.getTime();
  Mhdr.time_end = H.getTime() + _params->expiry;
  Mhdr.time_expire = H.getTime();
  Mhdr.time_centroid = H.getTime();

  fhdr.forecast_time = H.getTime();
  //
  fhdr.forecast_delta = 0;

  fhdr.bad_data_value = H.getMissingFl32();   fhdr.missing_data_value = H.getMissingFl32();
  //
  sprintf( fhdr.field_name_long,"%s", H.getFieldName());
  sprintf( fhdr.field_name,"%s", H.getFieldName());
  sprintf( fhdr.units,"%s", H.getUnits());
  sprintf( fhdr.transform,"%s","none");
  //
  // Declare a DsMdvx object so we can load it up with fields.
  //
  DsMdvx outMdvx;
  outMdvx.setMasterHeader( Mhdr );

  MdvxField *field = NULL;
  //
  if (_saveAsRadial){
    fhdr.nx = H.getNgates();
    fhdr.ny = H.getNradials();
    fhdr.grid_dx = H.getGateSpacing();
    fhdr.grid_dy = H.getDelAz();
    //
    fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
    //
    fhdr.grid_minx = H.getFirstGateDist();
    fhdr.grid_miny = 0.0;
    //
    fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
    field = new MdvxField(fhdr, vhdr, H.getAllRadialFl32s());
  } else {
    H.remapToCart(_params->res.delta, _params->res.dist);
    fhdr.nx = H.getNxy();
    fhdr.ny = H.getNxy();
    fhdr.grid_dx = H.getDxy();
    fhdr.grid_dy = H.getDxy();
    //
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    //
    fhdr.grid_minx = -fhdr.grid_dx*fhdr.nx/2.0 + fhdr.grid_dx/2.0;
    fhdr.grid_miny = -fhdr.grid_dy*fhdr.ny/2.0 + fhdr.grid_dy/2.0;
    //
    fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
    field = new MdvxField(fhdr, vhdr, H.getCartData());
  }
  //
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }

  outMdvx.addField(field);

  if (outMdvx.writeToDir(_outUrl)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _outUrl << endl;
    cerr << outMdvx.getErrStr() << endl;
  }

  if ( (!(_params->forkIt)) && (_params->threadLimit.limitNumThreads) ){
    if (_params->debug) cerr << "Releasing active thread number " << nt << endl;
    pthread_mutex_lock(&ntMutex);  nt--;  pthread_mutex_unlock(&ntMutex);
  }
  return;

}
