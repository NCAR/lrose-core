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
// This is the module of the adjointDump2mdv application that does most
// of the work, reading the data and writing the MDV files. Include
// MDV header files.
//
#include "adjointDump2mdv.hh"

#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

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
adjointDump2mdv::adjointDump2mdv(Params *P){
  _params = P;
  return;
}
//
// Destructor - does nothing but avoids default destructor.
//
adjointDump2mdv::~adjointDump2mdv(){
  return;
}
//
// Main method 
//
void adjointDump2mdv::adjointDump2mdvFile( char *FilePath ){

  //
  // See if we can parse the file name from the filename.
  //
  if (strlen(FilePath) < strlen("20080328_083000.dump")){
    cerr << FilePath << " : filename does not follow 20080328_083000.dump naming convention." << endl;
    return;
  }

  bool gotTime = false;
  bool isForecast = false;
  long leadTime = 0L;
  date_time_t dataTime, genTime;

  //
  // Do we have a generation time in the filename?
  //
  if (strlen(FilePath) > strlen("20080328_030000_20080328_083000.dump")){

    char *q = FilePath + strlen(FilePath) - strlen("20080328_030000_20080328_083000.dump");

    if (12 == sscanf(q, "%4d%2d%2d_%2d%2d%2d_%4d%2d%2d_%2d%2d%2d",
		     &genTime.year,  &genTime.month,  &genTime.day,
		     &genTime.hour,  &genTime.min,    &genTime.sec,
		     &dataTime.year, &dataTime.month, &dataTime.day,
		     &dataTime.hour, &dataTime.min,   &dataTime.sec)){
      gotTime = true; isForecast = true;
      uconvert_to_utime( &dataTime ); uconvert_to_utime ( &genTime );
      leadTime = dataTime.unix_time - genTime.unix_time;
    }
  }

  // Still no time? Try for a normal filename, no gentime -

  if (!(gotTime)){

    char *p = FilePath + strlen(FilePath) - strlen("20080328_083000.dump");

    if (6 != sscanf(p, "%4d%2d%2d_%2d%2d%2d",
		    &dataTime.year, &dataTime.month, &dataTime.day,
		    &dataTime.hour, &dataTime.min, &dataTime.sec)){
      cerr << "Failed to parse time from " << FilePath << endl;
      return;
    }
    uconvert_to_utime( &dataTime );
  }


  FILE *fp = fopen(FilePath, "r");
  if (fp == NULL){
    cerr << "Unable to open " << FilePath << endl;
    return;
  }

  long totalVolumeSize = 0L;
  bool threeDfieldPresent = false;
  long numExtraBytes = 0L;

  for (int i=0; i < _params->fields_n; i++){
    totalVolumeSize += _params->grid.nx * _params->grid.ny *  _params->_fields[i].nz;
    numExtraBytes += _params->_fields[i].numTrailingBytes;
    if (_params->_fields[i].nz > 1) {
      threeDfieldPresent = true;
    }
  }


  unsigned char *outData = (unsigned char *) malloc(sizeof(fl32)*totalVolumeSize + numExtraBytes);
  if (outData == NULL){
    cerr << "Malloc failed, size " << totalVolumeSize << endl;
    exit(0);
  }

  long expectedFileSize = sizeof(fl32)*totalVolumeSize + numExtraBytes;

  cerr << "Expected file size : " << expectedFileSize << endl;

  if (fseek(fp, _params->skipBytesAtStart, SEEK_SET)){
    cerr << "fseek at start of file failed!" << endl;
    exit(-1);
  }

  int numFound = fread(outData, sizeof(unsigned char), expectedFileSize, fp);

  if (numFound < expectedFileSize){
    cerr << "Only found " << numFound << " bytes in " << FilePath << endl;
    free(outData);
    return; 
  }


  fclose(fp);


  //
  // Swap the bytes, if requested.
  //
  if (_params->byteSwap){

    long bytePtr = 0L;

    for (int ifld=0; ifld < _params->fields_n; ifld++){

      for (int ib=0; ib <  _params->grid.nx * _params->grid.ny *  _params->_fields[ifld].nz; ib++){

	unsigned char tmp = outData[bytePtr];
	outData[bytePtr] = outData[bytePtr+1];
	outData[bytePtr+1] = tmp;
	bytePtr += 2;

	tmp = outData[bytePtr];
	outData[bytePtr] = outData[bytePtr+1];
	outData[bytePtr+1] = tmp;
	bytePtr += 2;
      }
      bytePtr += _params->_fields[ifld].numTrailingBytes;
    }
  }

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
  //
  sprintf(Mhdr.data_set_info,"%s","adjointDump2mdv");
  sprintf(Mhdr.data_set_name,"%s","adjointDump2mdv");
  sprintf(Mhdr.data_set_source,"%s", "adjointDump2mdv");
  //
  // Set the times in the master and field headers.
  //
  
  Mhdr.time_gen = dataTime.unix_time - leadTime;
  Mhdr.time_begin = dataTime.unix_time;
  Mhdr.time_end = dataTime.unix_time;
  Mhdr.time_expire = dataTime.unix_time;
  Mhdr.time_centroid = dataTime.unix_time;
  //
  fhdr.forecast_time = Mhdr.time_centroid;
  //
  fhdr.forecast_delta = leadTime;
  
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
  // Replace out of range values, NaNs, with MDV bd data value.
  //

  long bytePtr = 0L;

  for (int ifld=0; ifld < _params->fields_n; ifld++){
    
    for (int ib=0; ib <  _params->grid.nx * _params->grid.ny *  _params->_fields[ifld].nz; ib++){
      
      fl32 *val = (fl32 *) &outData[bytePtr];
	
      if (
	  (*val < _params->range.min) ||
	  (*val > _params->range.max) ||
	  (std::isnan(*val))
	  ){
	*val = fhdr.missing_data_value;
      }
      
      bytePtr += sizeof(fl32);
      
    }
    bytePtr += _params->_fields[ifld].numTrailingBytes;
  }
  
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
  unsigned long iptr = 0L;
  for (int ifld=0; ifld < _params->fields_n; ifld++){
    
    //
    // If we are saving this out in MDV file, add thie field.
    //
    if ( _params->_fields[ifld].outputInMdv ){
      
      fhdr.nz = _params->_fields[ifld].nz;
      if (fhdr.nz > 1){
	fhdr.native_vlevel_type = vhdr3D.type[0];
	fhdr.vlevel_type = vhdr3D.type[0];
      } else {
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
	
      if (fhdr.nz > 1){
	Field = new MdvxField(fhdr, vhdr3D, outData + iptr );
      } else {
	Field = new MdvxField(fhdr, vhdr2D, outData + iptr );
      }
	
      if (Field->convertRounded(Mdvx::ENCODING_INT16,
				Mdvx::COMPRESSION_ZLIB)){
	cerr << "Conversion of field failed - I cannot go on." << endl;
	exit(-1);
      }
      outMdvx.addField(Field);

    }

    //
    // If we have saved the field or not, increment the pointer past
    // this field.
    //
    iptr += _params->grid.nx * _params->grid.ny * _params->_fields[ifld].nz * sizeof(fl32) + _params->_fields[ifld].numTrailingBytes;
   
  }
  //
  // Finally, now that the DsMdvx object is all loaded up, use it
  // to write out the data.
  //

  if (isForecast) outMdvx.setWriteAsForecast();

  if (outMdvx.writeToDir(_params->outUrl)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << _params->outUrl << endl;
    cerr << outMdvx.getErrStr() << endl;
  }
  
  if (_params->debug) cerr << "Wrote to " << _params->outUrl << endl;

  free(outData);

  if (_params->deleteInput) unlink(FilePath);
  
  return;
  
}


// See if a string represents a number.

int adjointDump2mdv::_isNum(char *str){
  
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





