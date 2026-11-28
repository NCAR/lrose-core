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
// This is the module of the medoc2mdv application that does most
// of the work, reading the data and writing the MDV files. Include
// MDV header files.
//
#include "medoc2mdv.hh"

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


//
// Constructor - makes a copy of a pointer to the TDRP parameters.
//
medoc2mdv::medoc2mdv(Params *P){
  _params = P;
  _metaDataFileSet = false;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
medoc2mdv::~medoc2mdv(){
  return;
}


void medoc2mdv::setMetaFileName(char *metaDataFile ){
  _metaDataFileSet = true;
  sprintf(_metaDataFile, "%s", metaDataFile );
  return;
}

//
// Main method 
//
void medoc2mdv::medoc2mdvFile( char *FilePath ){

  FILE *fp = fopen(FilePath, "r");
  if (fp == NULL){
    cerr << "Unable to open " << FilePath << endl;
    return;
  }

  long totalVolumeSize = 0L;
  bool threeDfieldPresent = false;

  for (int i=0; i < _params->fields_n; i++){
    if (_params->_fields[i].is3D){
      totalVolumeSize += _params->grid.nx * _params->grid.ny * _params->vlevels_n;
      threeDfieldPresent = true;
    } else {
      totalVolumeSize += _params->grid.nx * _params->grid.ny;
    }
  }

  fl32 *medocData = (fl32 *) malloc(sizeof(fl32)*totalVolumeSize);
  if (medocData == NULL){
    cerr << "Malloc failed, size " << totalVolumeSize << endl;
    exit(0);
  }

  // Read data in from the file.

  char Line[1024];
  date_time_t dataTime;
  dataTime.unix_time = 0L;

  //
  // Look for the data time
  //

  do {

    while(NULL != fgets(Line, 1024, fp)){

      if (NULL != strstr(Line, "FFFFFFFF")){
	
	if (NULL == fgets(Line, 1024, fp)){
	  fprintf(stderr,"Time decoding failed\n");
	  exit(-1);
	}
	
	if (NULL == fgets(Line, 1024, fp)){
	  fprintf(stderr,"Time Decoding failed\n");
	  exit(-1);
	}
	
	if (6 != sscanf(Line, "%d %d %d %d %d %d",
			&dataTime.day, &dataTime.month, &dataTime.year, 
			&dataTime.hour, &dataTime.min, &dataTime.sec)){
	  fprintf(stderr,"Time Decoding Failed\n");
	  exit(-1);
	}

	if (dataTime.year < 100) 
	  dataTime.year += 2000;

	uconvert_to_utime( &dataTime );
	if (_params->debug) 
	  cerr << "Data found at " << utimstr(dataTime.unix_time) << endl;
	break;
      }
    }

    if (dataTime.unix_time == 0L){
      if (_params->debug){
	cerr << "Unable to locate any more data times." << endl;
      }
      fclose(fp); free(medocData);
      return;
    }
    
    //
    // Now go looking for data.
    //
    int numFound = 0;
    while ((numFound < totalVolumeSize) && (NULL != fgets(Line, 1024, fp))){
      
      //
      // Remove the trailing carriage return at the end of the line.
      //
      if (10 ==  (int)Line[strlen(Line)-1])
	Line[strlen(Line)-1] = char(0);
      
      //
      // Start parsing tokens
      //
      
      char *tok = strtok(Line, " ");
      
      do {
	
	if (tok == NULL) break;      
	
	if (_isNum(tok)){
	  medocData[numFound] = atof( tok );
	  if (
	      (medocData[numFound] < _params->range.min) ||
	      (medocData[numFound] > _params->range.max)
	      ){
	    medocData[numFound] = _params->range.min-1.0;
	  }
	  numFound++;
	} else {
	  if ((_params->debug) && (numFound > 0)){
	    fprintf(stderr, "Could not convert %s to a number, resetting after %d entries, this is normal\n", 
		    tok, numFound);
	  }
	  numFound = 0;
	}

	tok = strtok(NULL, " ");

      } while (tok != NULL);

    }


    if (numFound < totalVolumeSize){
      fclose(fp); free(medocData);
      return; // Must be at file's end
    }

    if (_params->debug)
      cerr << numFound << " data points found." << endl;
    
    
    //
    // If we got here, we have a volume.
    //


    //
    //
    // Prepare to write MDV files. Declare a master, field
    // and vlevel header.
    //
    Mdvx::field_header_t fhdr;
    Mdvx::vlevel_header_t vhdr3D;
    Mdvx::vlevel_header_t vhdr2D;
    Mdvx::master_header_t Mhdr;
    //
    // Use 'memset' to set all the bytes in these structures to 0.
    //
    memset(&fhdr, 0, sizeof(fhdr));
    memset(&vhdr2D, 0, sizeof(vhdr2D));
    memset(&vhdr3D, 0, sizeof(vhdr3D));
    memset(&Mhdr, 0, sizeof(Mhdr));
    //
    // Set up the vlevel header. This is pretty simple as its is
    // just surface data.
    //
    vhdr2D.type[0] = Mdvx::VERT_TYPE_SURFACE;
    vhdr2D.level[0] = 0.0;
    //
    //
    switch (_params->output_vert){

    case Params::OUTPUT_VERT_Z :
      vhdr3D.type[0] = Mdvx::VERT_TYPE_Z;
      break;

    case Params::OUTPUT_VERT_PRESSURE :
      vhdr3D.type[0] = Mdvx::VERT_TYPE_PRESSURE;
      break;

    case Params::OUTPUT_VERT_SIGMA_P :
      vhdr3D.type[0] = Mdvx::VERT_TYPE_SIGMA_P;
      break;

    case Params::OUTPUT_VERT_SIGMA_Z :
      vhdr3D.type[0] = Mdvx::VERT_TYPE_SIGMA_Z;
      break;

    default :
      cerr << "Unrecognized vert type : " << _params->output_vert << endl;
      exit(-1);
      break;

    }

    for (int k=0; k < _params->vlevels_n; k++){
      vhdr3D.level[k] = _params->_vlevels[k];
    }

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
    fhdr.native_vlevel_type = vhdr3D.type[0];
    fhdr.vlevel_type = vhdr3D.type[0];
    //
    // Set up some things in the master header.
    //
    Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
    Mhdr.num_data_times = 1;
    Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
    Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;

    if (threeDfieldPresent)
      Mhdr.native_vlevel_type = vhdr3D.type[0];
    else
      Mhdr.native_vlevel_type = vhdr2D.type[0];

    Mhdr.vlevel_included = 1;
    //
    char dataName[128];
    char dataSource[128];
    sprintf(dataName, "medoc2mdv");
    sprintf(dataSource, "medoc2mdv");
    //

    if (_params->metaDataFromFile.metaDataFromFile){

      char *fileNameToUse = _params->metaDataFromFile.filename;
      if (  _metaDataFileSet )
	fileNameToUse =  _metaDataFile;

      FILE *mfp = fopen(fileNameToUse, "r");
      if (mfp == NULL){
	fprintf(stderr, "WARNING : Failed to open %s\n", 
		fileNameToUse );
      } else {
	if (NULL == fgets(dataName, 128, mfp))
	  fprintf(stderr, "WARNING : Failed to read dataset name from %s\n", 
		  fileNameToUse);

	if (NULL == fgets(dataSource, 128, mfp))
	  fprintf(stderr, "WARNING : Failed to read dataset source from %s\n", 
		  fileNameToUse);

	fclose(mfp);


	// Strip trailing line return from fgets()

	if (dataName[strlen(dataName)-1] == char(10))
	  dataName[strlen(dataName)-1] = char(0);

	if (dataSource[strlen(dataSource)-1] == char(10))
	  dataSource[strlen(dataSource)-1] = char(0);
      }
    }

    sprintf(Mhdr.data_set_info,"%s","medoc2mdv");
    sprintf(Mhdr.data_set_name,"%s",dataName);
    sprintf(Mhdr.data_set_source,"%s", dataSource);
    //
    // Set the times in the master and field headers.
    //

    Mhdr.time_gen = dataTime.unix_time;
    Mhdr.time_begin = dataTime.unix_time;
    Mhdr.time_end = dataTime.unix_time;
    Mhdr.time_expire = dataTime.unix_time;
    Mhdr.time_centroid = dataTime.unix_time;
    //
    fhdr.forecast_time = Mhdr.time_centroid;
    //
    fhdr.forecast_delta = 0;

    switch ( _params->projection ){

    case Params::OUTPUT_PROJ_FLAT :
      fhdr.proj_type = Mdvx::PROJ_FLAT;
      fhdr.proj_param[0] = _params->grid.flatRotation;
      break;

    case Params::OUTPUT_PROJ_LATLON :
      fhdr.proj_type = Mdvx::PROJ_LATLON;
      break;

    case Params::OUTPUT_PROJ_LAMBERT :
      fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      fhdr.proj_param[0] = _params->grid.lambertLat1;
      fhdr.proj_param[1] = _params->grid.lambertLat2;
      break;

    default :
      cerr << "Unrecognized projection : " << _params->projection << endl;
      exit(-1);
      break;

    }

    fhdr.bad_data_value = _params->range.min-1.0;
    fhdr.missing_data_value = fhdr.bad_data_value;

    //
    //
    fhdr.nx = _params->grid.nx;
    fhdr.ny = _params->grid.ny;
    fhdr.nz = 1;
    //
    fhdr.grid_dx = _params->grid.dx;
    fhdr.grid_dy = _params->grid.dy;
    //
    //
    fhdr.proj_origin_lat =  _params->grid.latOrig;
    fhdr.proj_origin_lon =  _params->grid.lonOrig;
    //
    //
    fhdr.grid_minx = _params->grid.minx;
    fhdr.grid_miny = _params->grid.miny;
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
    long iptr = 0L;
    for (int ifld=0; ifld < _params->fields_n; ifld++){
      
      //
      // If we are saving this out in MDV file, add thie field.
      //
      if ( _params->_fields[ifld].outputInMdv ){

	if (_params->_fields[ifld].is3D){
	  fhdr.nz = _params->vlevels_n;
	  fhdr.native_vlevel_type = vhdr3D.type[0];
	  fhdr.vlevel_type = vhdr3D.type[0];
	} else {
	  fhdr.nz = 1;
	  fhdr.native_vlevel_type = vhdr2D.type[0];
	  fhdr.vlevel_type = vhdr2D.type[0];
	}
	
	if (_params->debug){
	  cerr <<"  For field " << _params->_fields[ifld].name;
	  cerr << " nz is " << fhdr.nz << endl;
	}
	
	fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
	
	sprintf( fhdr.field_name_long,"%s", _params->_fields[ifld].longName);
	sprintf( fhdr.field_name,"%s", _params->_fields[ifld].name);
	sprintf( fhdr.units,"%s", _params->_fields[ifld].units);    
	sprintf( fhdr.transform,"%s","none");
	//
	MdvxField *Field;
	
	if (_params->_fields[ifld].is3D){
	  Field = new MdvxField(fhdr, vhdr3D, medocData + iptr );
	} else {
	  Field = new MdvxField(fhdr, vhdr2D, medocData + iptr );
	}
	
	if (Field->convertRounded(Mdvx::ENCODING_INT16,
				  Mdvx::COMPRESSION_ZLIB)){
	  cerr << "Conversion of field failed - I cannot go on." << endl;
	  exit(-1);
	}
	outMdvx.addField(Field);

      } // End of if we are saving out as MDV

      //
      // If we have saved the field or not, increment the pointer past
      // this field.
      //
      if (_params->_fields[ifld].is3D){
	iptr += fhdr.nx * fhdr.ny * fhdr.nz;
      } else {
	iptr += fhdr.nx * fhdr.ny;
      }
   
    }
    //
    // Finally, now that the DsMdvx object is all loaded up, use it
    // to write out the data.
    //
    if (outMdvx.writeToDir(_params->outUrl)) {
      cerr << "ERROR - Output::write" << endl;
      cerr << "  Cannot write to url: " << _params->outUrl << endl;
      cerr << outMdvx.getErrStr() << endl;
    }

    if (_params->debug) cerr << "Wrote to " << _params->outUrl << endl;

  } while(1);
  //
  // Will in fact never get here.
  //

}


// See if a string represents a number.

int medoc2mdv::_isNum(char *str){
  
  for (unsigned i=0; i < strlen(str); i++){
    if (str[i] == 'E') str[i]='e';

    if (
	(str[i] != ' ') &&
	(str[i] != '.') &&
	(str[i] != '-') &&
	(str[i] != '+') &&
	(str[i] != 'e') &&
	(!(isdigit((int)str[i])))
	){
      return 0;
    }
  }

  return 1;

}





