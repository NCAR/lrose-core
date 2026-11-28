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
// This is the module of the MtSatHRIT2Mdv application that does most
// of the work, reading the data and writing the MDV files. Include
// MDV header files.
//
#include "MtSatHRIT2Mdv.hh"

#include <iostream>
#include <stdlib.h>
#include <cstdio>

#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "fieldDataMgr.hh"

//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
MtSatHRIT2Mdv::MtSatHRIT2Mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
MtSatHRIT2Mdv::~MtSatHRIT2Mdv(){
  return;
}
//
// Main method 
//
void MtSatHRIT2Mdv::MtSatHRIT2MdvFile( char *FilePath ){
  //

  //
  // Parse the time from the file name.
  //
  date_time_t filenameTime;
  filenameTime.unix_time = 0L;

  int numDigits = strlen("YYYYMMDDhhmmss");
  int iend = strlen(FilePath) - numDigits;
  
  for (int i=0; i < iend; i++){
    
    bool gotTime = true;
    for (int j=i; j < i+numDigits; j++){
      if (!(isdigit((int)FilePath[j]))){
	gotTime = false;
	break;
      }
    }
    if (gotTime){
      if (6 != sscanf(FilePath + i,"%4d%2d%2d%2d%2d%2d",
		      &filenameTime.year, &filenameTime.month, &filenameTime.day, 
		      &filenameTime.hour, &filenameTime.min, &filenameTime.sec)){
	cerr << "Failed to parse time from " << FilePath + i << endl;
	return;
      }
      uconvert_to_utime( &filenameTime );
    }
  }
  
  if (filenameTime.unix_time == 0L){
    cerr << "Failed to parse time from " << FilePath << endl;
    return;
  }

  fieldDataMgr ir1Field(FilePath, FALSE, _params );

  char subString[256];
  sprintf(subString, "%4d%02d%02d%02d%02d%02d",
	  filenameTime.year, filenameTime.month, filenameTime.day, 
	  filenameTime.hour, filenameTime.min, filenameTime.sec);

  char fileName[1024];

  _findFile( _params->_input[0].dir, _params->_input[0].filenameSubString,
	     subString, fileName, TRUE, _params);
  fieldDataMgr visField(fileName, TRUE, _params );

  _findFile( _params->_input[1].dir, _params->_input[1].filenameSubString,
	     subString, fileName, FALSE, _params );
  fieldDataMgr ir2Field(fileName, FALSE, _params );

  _findFile( _params->_input[2].dir, _params->_input[2].filenameSubString,
	     subString, fileName, FALSE, _params );
  fieldDataMgr ir3Field( fileName, FALSE, _params );

  _findFile( _params->_input[3].dir, _params->_input[3].filenameSubString,
	     subString, fileName, FALSE, _params );
  fieldDataMgr ir4Field( fileName, FALSE, _params );

  //
  // Prepare to write MDV files. Declare a master, field
  // and vlevel header.
  //
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t Mhdr;
  //
  // Use 'memset' to set all the bytes in these structures to 0.
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
  //
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
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 0;
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
  sprintf(Mhdr.data_set_info,"%s","MtSatHRIT2Mdv");
  sprintf(Mhdr.data_set_name,"%s","MtSatHRIT2Mdv");
  sprintf(Mhdr.data_set_source,"%s", "MtSatHRIT2Mdv");
  //
  // Set the times in the master and field headers.
  //
  time_t timeToUse = visField.getTime();
  if (_params->useFilenameTime) timeToUse = filenameTime.unix_time;

  Mhdr.time_gen = timeToUse;
  Mhdr.time_begin = timeToUse;
  Mhdr.time_end = timeToUse;
  Mhdr.time_expire = timeToUse + 3600;
  Mhdr.time_centroid = timeToUse;
  //
  fhdr.forecast_time = Mhdr.time_centroid;
  //
  fhdr.forecast_delta = 0;
  fhdr.proj_type = Mdvx::PROJ_LATLON; // This is implied for these data

  fhdr.bad_data_value = fieldDataMgr::badVal;
  fhdr.missing_data_value = fieldDataMgr::badVal;

  for (int idomain=0; idomain < _params->domains_n; idomain++){

    if (_params->outputFields != Params::OUTPUT_VIS){

      if (
	  ((_params->_domains[idomain].requireNorthDisk) && (!(ir1Field.northCovered()))) ||
	  ((_params->_domains[idomain].requireSouthDisk) && (!(ir1Field.southCovered())))
	  ){
	if (_params->debug){
	  cerr << "Skipping scan to write to " << _params->_domains[idomain].url << ", ir coverage is not sufficient." << endl;
	}
	continue; // Skip this scan, coverage is not sufficient.
      }

      ir1Field.loadData( _params->_domains[idomain].lowerLeftLat, _params->_domains[idomain].lowerLeftLon,
			 _params->_domains[idomain].upperRightLat, _params->_domains[idomain].upperRightLon,
			 _params->_domains[idomain].delLat, _params->_domains[idomain].delLon );

      ir2Field.loadData( _params->_domains[idomain].lowerLeftLat, _params->_domains[idomain].lowerLeftLon,
			 _params->_domains[idomain].upperRightLat, _params->_domains[idomain].upperRightLon,
			 _params->_domains[idomain].delLat, _params->_domains[idomain].delLon );
      
      ir3Field.loadData( _params->_domains[idomain].lowerLeftLat, _params->_domains[idomain].lowerLeftLon,
			 _params->_domains[idomain].upperRightLat, _params->_domains[idomain].upperRightLon,
			 _params->_domains[idomain].delLat, _params->_domains[idomain].delLon );
      
      ir4Field.loadData( _params->_domains[idomain].lowerLeftLat, _params->_domains[idomain].lowerLeftLon,
			 _params->_domains[idomain].upperRightLat, _params->_domains[idomain].upperRightLon,
			 _params->_domains[idomain].delLat, _params->_domains[idomain].delLon );
    }


    if (_params->outputFields != Params::OUTPUT_IR){

      if (
	  ((_params->_domains[idomain].requireNorthDisk) && (!(visField.northCovered()))) ||
	  ((_params->_domains[idomain].requireSouthDisk) && (!(visField.southCovered())))
	  ){
	if (_params->debug){
	  cerr << "Skipping scan to write to " << _params->_domains[idomain].url << ", vis coverage is not sufficient." << endl;
	}
	continue; // Skip this scan, coverage is not sufficient.
      }

      if (  _params->_domains[idomain].writeVisAtFullRes){
	visField.loadData( _params->_domains[idomain].lowerLeftLat, _params->_domains[idomain].lowerLeftLon,
			   _params->_domains[idomain].upperRightLat, _params->_domains[idomain].upperRightLon,
			   _params->_domains[idomain].delLat/4.0, _params->_domains[idomain].delLon/4.0 );
      } else {
	visField.loadData( _params->_domains[idomain].lowerLeftLat, _params->_domains[idomain].lowerLeftLon,
			   _params->_domains[idomain].upperRightLat, _params->_domains[idomain].upperRightLon,
			   _params->_domains[idomain].delLat, _params->_domains[idomain].delLon );
      } 
    }

    // Then set up the field header with the metadata.
    //
    fhdr.nx = ir1Field.getNx();
    fhdr.ny = ir1Field.getNy();
    fhdr.nz = 1;
    //
    fhdr.grid_dx = _params->_domains[idomain].delLon;
    fhdr.grid_dy = _params->_domains[idomain].delLat;
    //
    fhdr.proj_type = Mdvx::PROJ_LATLON; // This is implied for these data.
    //
    fhdr.proj_origin_lat =  _params->_domains[idomain].lowerLeftLat;
    fhdr.proj_origin_lon =  _params->_domains[idomain].lowerLeftLon;
    //
    //
    fhdr.grid_minx = _params->_domains[idomain].lowerLeftLon;
    fhdr.grid_miny = _params->_domains[idomain].lowerLeftLat;
    //
    // Declare a DsMdvx object so we can load it up with fields.
    //
    DsMdvx outMdvx;
    //
    outMdvx.setMasterHeader( Mhdr ); 
    outMdvx.clearFields();
    //
    // Get the fields, add them to the DsMdvx object.
    //
    fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);

    if (_params->outputFields != Params::OUTPUT_VIS){
      //
      //////////////// IR1 /////////////////////////////////
      //
      if (ir1Field.isOk()){
	sprintf( fhdr.field_name_long,"%s", _params->_fieldNames[0].longFieldname);
	sprintf( fhdr.field_name,"%s", _params->_fieldNames[0].shortFieldname);
	sprintf( fhdr.units,"%s", "C");    sprintf( fhdr.transform,"%s","none");
	//
	MdvxField *mdvIr1Field;
	mdvIr1Field = new MdvxField(fhdr, vhdr, ir1Field.getData() );
	if (mdvIr1Field->convertRounded(Mdvx::ENCODING_INT16,
					Mdvx::COMPRESSION_ZLIB)){
	  cerr << "Conversion of IR1 field failed - I cannot go on." << endl;
	  exit(-1);
	}
	outMdvx.addField(mdvIr1Field);
      } else {
	if (_params->debug){
	  cerr << "ERROR : IR1 field did not seem to load OK." << endl;
	}
      }
      //
      //////////////// IR2 /////////////////////////////////
      //
      if (ir2Field.isOk()){
	sprintf( fhdr.field_name_long,"%s", _params->_fieldNames[1].longFieldname);
	sprintf( fhdr.field_name,"%s", _params->_fieldNames[1].shortFieldname);
	sprintf( fhdr.units,"%s", "C");    sprintf( fhdr.transform,"%s","none");
	//
	MdvxField *mdvIr2Field;
	mdvIr2Field = new MdvxField(fhdr, vhdr, ir2Field.getData() );
	if (mdvIr2Field->convertRounded(Mdvx::ENCODING_INT16,
					Mdvx::COMPRESSION_ZLIB)){
	  cerr << "Conversion of IR2 field failed - I cannot go on." << endl;
	  exit(-1);
	}
	outMdvx.addField(mdvIr2Field);
      } else {
	if (_params->debug){
	  cerr << "ERROR : IR2 field did not seem to load OK." << endl;
	}
      }
      //
      //////////////// IR3 /////////////////////////////////
      //
      if (ir3Field.isOk()){
	sprintf( fhdr.field_name_long,"%s", _params->_fieldNames[2].longFieldname);
	sprintf( fhdr.field_name,"%s", _params->_fieldNames[2].shortFieldname);     
	sprintf( fhdr.units,"%s", "C");    sprintf( fhdr.transform,"%s","none");
	//
	MdvxField *mdvIr3Field;
	mdvIr3Field = new MdvxField(fhdr, vhdr, ir3Field.getData() );
	if (mdvIr3Field->convertRounded(Mdvx::ENCODING_INT16,
					Mdvx::COMPRESSION_ZLIB)){
	  cerr << "Conversion of IR3 field failed - I cannot go on." << endl;
	  exit(-1);
	}
	outMdvx.addField(mdvIr3Field);
      } else {
	if (_params->debug){
	  cerr << "ERROR : IR3 field did not seem to load OK." << endl;
	}
      }
      //
      //////////////// IR4 /////////////////////////////////
      //
      if (ir4Field.isOk()){
	sprintf( fhdr.field_name_long,"%s", _params->_fieldNames[3].longFieldname);
	sprintf( fhdr.field_name,"%s", _params->_fieldNames[3].shortFieldname);
	sprintf( fhdr.units,"%s", "C");    sprintf( fhdr.transform,"%s","none");
	//
	MdvxField *mdvIr4Field;
	mdvIr4Field = new MdvxField(fhdr, vhdr, ir4Field.getData() );
	if (mdvIr4Field->convertRounded(Mdvx::ENCODING_INT16,
					Mdvx::COMPRESSION_ZLIB)){
	  cerr << "Conversion of IR4 field failed - I cannot go on." << endl;
	  exit(-1);
	}
	outMdvx.addField(mdvIr4Field);
      } else {
	if (_params->debug){
	  cerr << "ERROR : IR4 field did not seem to load OK." << endl;
	}
      }
    }

    if (_params->outputFields != Params::OUTPUT_IR){
      //
      //////////////// VIS /////////////////////////////////
      //
      if (visField.isOk()){
	if (  _params->_domains[idomain].writeVisAtFullRes){
	  fhdr.nx = visField.getNx();    fhdr.ny = visField.getNy();
	  fhdr.grid_dx /= 4.0;  fhdr.grid_dy /= 4.0; 
	  fhdr.volume_size = fhdr.nx * fhdr.ny * sizeof(fl32);
	}
	//
	sprintf( fhdr.field_name_long,"%s", _params->_fieldNames[4].longFieldname);
	sprintf( fhdr.field_name,"%s", _params->_fieldNames[4].shortFieldname);
	sprintf( fhdr.units,"%s", "%");    sprintf( fhdr.transform,"%s","none");
	//
	MdvxField *mdvVisField;
	mdvVisField = new MdvxField(fhdr, vhdr, visField.getData() );
	if (mdvVisField->convertRounded(Mdvx::ENCODING_INT16,
					Mdvx::COMPRESSION_ZLIB)){
	  cerr << "Conversion of VIS field failed - I cannot go on." << endl;
	  exit(-1);
	}
	outMdvx.addField(mdvVisField);
      } else {
	if (_params->debug){
	  cerr << "ERROR : VIS field did not seem to load OK." << endl;
	}
      }
    }
    //
    // Finally, now that the DsMdvx object is all loaded up, use it
    // to write out the data.
    //
    if (outMdvx.writeToDir(_params->_domains[idomain].url)) {
      cerr << "ERROR - Output::write" << endl;
      cerr << "  Cannot write to url: " << _params->_domains[idomain].url << endl;
      cerr << outMdvx.getErrStr() << endl;
    }

    if (_params->debug) cerr << "Wrote to " << _params->_domains[idomain].url << endl;

  } // End of loop through domains.

  return;


}

//
// Small routine to find a file with a substring in it and
// a certain date/time in it. If this is found, 0 is returned
// and the filename is placed in the 'fileName' string (caller
// to allocate). If it is not found, -1 is returned and
// 'fileName' is set to "NONE"
//

int MtSatHRIT2Mdv::_findFile( char *dir, char *nameSubString,
			      char *dateSubString, char *fileName,
			      bool isVis, Params *TDRP_params){

  if (isVis && (TDRP_params->outputFields == Params::OUTPUT_IR)){
     return 0;
  }

  if (!(isVis) && (TDRP_params->outputFields == Params::OUTPUT_VIS)){
     return 0;
  }

  if (_params->debug){
    cerr << "_findFile looking for file with ";
    cerr << nameSubString << " in the name for date ";
    cerr << dateSubString << endl;
  }


  sprintf(fileName, "NONE");

  time_t startTime = time(NULL);

  int go = 1;
  int icount = 0;

  do {

    DIR *dirp;
    dirp = opendir(dir);
  
    if (dirp == NULL){
      cerr << "Cannot open directory '" << dir << "'" << endl;
      perror(dir); 
      return -1;
    }

    struct dirent *dp;
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
      
      if (_params->verbose) {
	cerr << "Found file: " << dp->d_name << endl;
      }

      if (NULL == strstr(dp->d_name, nameSubString)) {
	if (_params->verbose) {
	  cerr << "-->> ignoring, does not have subString: "
	       << nameSubString << endl;
	}
	continue;
      }

      // check for required file extension
      
      if (strlen(_params->required_file_ext) > 0) {
	int extLen = strlen(_params->required_file_ext);
	const char *fileExt = dp->d_name + strlen(dp->d_name) - extLen;
	if (strcmp(fileExt, _params->required_file_ext) != 0) {
	  if (_params->verbose) {
	    cerr << "-->> did not find required extension: "
		 << _params->required_file_ext << endl;
	    cerr << "-->> ignoring file" << endl;
	  }
	  continue;
	}
      }
      
      // check for ignored file extension

      if (strlen(_params->ignored_file_ext) > 0) {
	int extLen = strlen(_params->ignored_file_ext);
	const char *fileExt = dp->d_name + strlen(dp->d_name) - extLen;
	if (strcmp(fileExt, _params->ignored_file_ext) == 0) {
	  if (_params->verbose) {
	    cerr << "-->> found ignored extension: "
		 << _params->ignored_file_ext << endl;
	    cerr << "-->> ignoring file" << endl;
	  }
	  continue;
	}
      }
      
      if (NULL != strstr(dp->d_name, dateSubString)) {
	if (_params->debug){
	  cerr << endl << "Found " << dp->d_name << " in " << dir << endl;
	}
	sprintf(fileName,"%s/%s", dir, dp->d_name);
	closedir(dirp);
	return 0;
      }
    }
    
    closedir(dirp);

    //
    // If this is a vis file, and we are using the expected times
    // that vis data will arrive, and this file is outside of those times,
    // then return (ie. don't wait for a vis file that is not
    // going to show up).
    //
    if ((_params->useVisExpectedTimes) && (NULL != strstr(nameSubString, "vis"))){

      char *p = dateSubString + strlen("20070814");
      int fileHour, fileMin;
      if (2!=sscanf(p,"%2d%2d", &fileHour, &fileMin)){
	cerr << "Failed to parse hour and minute from " << p << endl;
	return -1;
      }

      int fileTimeOfDay = 60*fileHour + fileMin;
      int minTimeOfDay = 60*_params->visMinTime.hour + _params->visMinTime.min;
      int maxTimeOfDay = 60*_params->visMaxTime.hour + _params->visMaxTime.min;
      
      bool expectVisFile = false;
      if (fileTimeOfDay <= maxTimeOfDay) expectVisFile = true;
      if (fileTimeOfDay >= minTimeOfDay) expectVisFile = true;

      if (!(expectVisFile)){
	if (_params->debug){
	  cerr << "No vis file expected at " << dateSubString;
	  cerr << " - checked, but will not wait around for one." << endl;
	}
	return -1;
      }

    }


    icount++;
    if (icount == _params->fileFindTimeout.numTries) go=0;

    if (go){
      for (int q=0; q < _params->fileFindTimeout.sleepBetweenTries; q++){
	PMU_auto_register("waiting for data file");
	sleep(1);
      }
    }

  } while (go);

  time_t endTime = time(NULL);

  if (_params->debug){
    cerr << "_findFile failed to find file with ";
    cerr << nameSubString << " in the name for date ";
    cerr << dateSubString << " after ";
    cerr << endTime - startTime << " seconds." << endl;
  }

  return -1; // Didn't find it.

}
