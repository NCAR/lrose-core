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
// Read netCDF radar grids from WDT, write mdv data.
//
#include "WDTncdf2Mdv.hh"

#include <toolsa/umisc.h>
#include <netcdf.h> 

#include <cstdlib>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <string.h>
using namespace std;

////////////////////////////////////////////////////////////
//
// constructor. That which there is to do, is done herein.
//
WDTncdf2Mdv::WDTncdf2Mdv (char *FilePath,Params *P){

  if (P->debug){
    cerr << "Processing file " << FilePath << endl;
  }
  
  int ncID;
  if (NC_NOERR != nc_open(FilePath, 0, &ncID)){
    cerr << "Failed to open " << FilePath << endl;
    return;
  }
  //
  // Get the dimensional IDs.
  //
  int xID,yID,recordID,n_valtimesID,data_variablesID,namelenID,charsPerLevelID,cref_levelsID;

  if (
      (NC_NOERR != nc_inq_dimid(ncID, "x", &xID)) ||
      (NC_NOERR != nc_inq_dimid(ncID, "y", &yID)) ||
      (NC_NOERR != nc_inq_dimid(ncID, "record", &recordID)) ||
      (NC_NOERR != nc_inq_dimid(ncID, "cref_smooth_levels", &cref_levelsID)) 
      ){
    cerr << "Dimensional variables x,y,record,cref_smooth_levels ";
    cerr << "must be present - I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  //
  // Get the variable IDs.
  //
  int crefID,crefLevelsID,crefInventoryID,valtimeMINUSreftimeID,valtimeID,reftimeID;

  if (
      (NC_NOERR != nc_inq_varid(ncID, "reftime", &reftimeID)) ||
      (NC_NOERR != nc_inq_varid(ncID, "valtime", &valtimeID)) 
      ){
    cerr << "Variables cref and valtime ";
    cerr << "must be present - I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  //
  // OK - now have variable and dimension IDs 
  //
  //
  unsigned  x,y,record,cref_levels;

  if (
      (NC_NOERR != nc_inq_dimlen(ncID, xID, &x)) ||
      (NC_NOERR != nc_inq_dimlen(ncID, yID, &y)) ||
      (NC_NOERR != nc_inq_dimlen(ncID, recordID, &record )) ||
      (NC_NOERR != nc_inq_dimlen(ncID, cref_levelsID, &cref_levels ))
      ){
    cerr << "Lengths unavailable for x, y, record,cref_levels ";
    cerr << "I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  
  if (P->debug){
    cerr << "Dimensions for " << FilePath << endl;
    cerr << "x: " << x  << endl;
    cerr << "y: " << y << endl;
    cerr << "record: " << record << endl;
    cerr << "cref_levels: " << cref_levels << endl;
  }

  //
  // Read in Global Attributes .
  //
  //
  float xMin,xMax,yMin,yMax,dx,dy;

  if ((NC_NOERR != nc_get_att_float(ncID, NC_GLOBAL, "xMin", &xMin)) ||
     (NC_NOERR != nc_get_att_float(ncID, NC_GLOBAL, "yMin", &yMin)) ||
     (NC_NOERR != nc_get_att_float(ncID, NC_GLOBAL, "xMax", &xMax)) ||
     (NC_NOERR != nc_get_att_float(ncID, NC_GLOBAL, "yMax", &yMax)) ||
     (NC_NOERR != nc_get_att_float(ncID, NC_GLOBAL, "dx", &dx)) ||
     (NC_NOERR != nc_get_att_float(ncID, NC_GLOBAL, "dy", &dy)) 
	 )
  {
    cerr << "Couldn't read xMin,xMax,yMin,yMax,dx,dy Attributes ";
    cerr << "I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }

  //
  // Read in Data Variables
  //
  int reftime,crefInventory,valtimeMINUSreftime,valtime;
  if (
      (NC_NOERR != nc_get_var_int(ncID, reftimeID, &reftime))
      ){
    cerr << "Couldn't read reftime, cref ";
    cerr << "I cannot process " << FilePath << endl;
    nc_close(ncID);
    return;
  }
  double dz = 0.0;

  //
  //
  // Set up a generic master and field headers.
  //

  //
  // Master header.
  //
  Mdvx::master_header_t Mhdr;
  memset(&Mhdr, 0, sizeof(Mhdr));

  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.data_dimension = 2;

  Mhdr.time_begin = Mhdr.time_end = Mhdr.time_centroid = (long) reftime;
  Mhdr.time_expire = reftime + 3600;
  Mhdr.time_gen = time(NULL);

  Mhdr.num_data_times = 1;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  Mhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  Mhdr.vlevel_included = 0;

  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;
  Mhdr.sensor_lon = Mhdr.sensor_lat = Mhdr.sensor_alt = 0.0;
  //
  strncpy(Mhdr.data_set_info,"WDT Radar mosaic",MDV_INFO_LEN);
  strncpy(Mhdr.data_set_name,"CREF mosaic", MDV_NAME_LEN);
  strncpy(Mhdr.data_set_source, FilePath, MDV_NAME_LEN);
  //

  //
  // Field header.
  //
  Mdvx::field_header_t Fhdr;
  memset(&Fhdr, 0, sizeof(Fhdr));

  //
  Fhdr.nx = x;        Fhdr.ny = y;        Fhdr.nz = cref_levels;
  Fhdr.grid_minx = xMin; Fhdr.grid_miny = yMin; Fhdr.grid_minz = 0.0; 
  Fhdr.grid_dx = dx;       Fhdr.grid_dy = dy;       Fhdr.grid_dz = 0.0; 

  Fhdr.proj_origin_lat = Fhdr.grid_miny;  Fhdr.proj_origin_lon = Fhdr.grid_minx;
  Fhdr.proj_type = Mdvx::PROJ_LATLON;
  Fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  Fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  Fhdr.data_element_nbytes = sizeof(float);
  Fhdr.volume_size = Fhdr.nx*Fhdr.ny*Fhdr.nz*Fhdr.data_element_nbytes;
  Fhdr.forecast_delta = 0;
  Fhdr.forecast_time = Mhdr.time_centroid;
  Fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;

  Fhdr.field_code = 0;
  Fhdr.user_time1 = Fhdr.user_time2 = Fhdr.user_time3 = 0;

  Fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  Fhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  Fhdr.dz_constant = 1;

  sprintf(Fhdr.transform, "%s","Assembled Mosaic");


  //
  // name, missing_data_value and units set for each var.
  //
  // Allocate space to read into.
  //
  float *data = (float *)calloc(sizeof(float),x*y*record*cref_levels);
  //
  // Set up DsMdvx object to write to.
  //
  DsMdvx Out;
  Out.setMasterHeader(Mhdr);
  Out.clearFields();     
  //
  // Loop through the fields.
  //
  MdvxRemapLut lut; // Used in the remapping of fields.
  //
  for(int ifield=0; ifield < P->InFields_n; ifield++){
    //
    if (P->debug){
      cerr << "Retrieving field " << P->_InFields[ifield] << endl;
    }
    //
    // Try to read in the data, skip the field if we fail.
    //
    int varID;
    if  (
	 (NC_NOERR != nc_inq_varid(ncID, P->_InFields[ifield], &varID)) ||
	 (NC_NOERR != nc_get_var_float(ncID, varID, data))
	 ){
      cerr << "ERROR : Could not find field " << P->_InFields[ifield] << endl;
      cerr << "This field will not be added." << endl;
      continue;
    }
    //
    // Get the units and missing_value attributes.
    //
    char units[64];
    if (NC_NOERR != nc_get_att_text(ncID, varID, "units", units)){
      cerr << "WARNING : units not found for field " << P->_InFields[ifield];
      cerr << " - setting units to \"none\"" << endl;
      sprintf(units,"%s","none");
    }
    
    float missing_value;
    if (NC_NOERR != nc_get_att_float(ncID, varID, "_FillValue", &missing_value)){
      missing_value = -99999.0;
      cerr << "WARNING : missing_value not found for field " << P->_InFields[ifield];
      cerr << " - setting missing value to " << missing_value << endl;
	}

    float scale;
    if (NC_NOERR != nc_get_att_float(ncID, varID, "_scale", &scale)){
      scale  = 1.0;
      cerr << "WARNING : _scale not found for field " << P->_InFields[ifield];
      cerr << " - setting scale value to " << scale << endl;
	}

    strncpy(Fhdr.units,units,MDV_UNITS_LEN);

    Fhdr.bad_data_value = missing_value;
    Fhdr.missing_data_value = missing_value;
    strncpy(Fhdr.field_name,P->_InFields[ifield],MDV_SHORT_FIELD_LEN);
    //
    // For Fhdr.field_name_long, try to use the long_name attribute, but if can't find it, use
    // the short name. Rose by any other nomenclature.
    //
    char long_name[64];
    if (NC_NOERR != nc_get_att_text(ncID, varID, "long_name", long_name)){
      strncpy(Fhdr.field_name_long, P->_InFields[ifield],MDV_LONG_FIELD_LEN);
    } else {
      strncpy(Fhdr.field_name_long,long_name,MDV_LONG_FIELD_LEN);
    }


    if (P->debug){
      cerr << "Units " << units << endl;
      cerr << "Missing_value " << missing_value << endl;
      cerr << "Long name : " << Fhdr.field_name_long << endl;
    }

	int numpts = x*y*record*cref_levels;
	float *ptr = data;

	for(int i=0; i < numpts; i++) *ptr++ /= scale;

    //
    // Create field.
    //
    MdvxField *fld = new MdvxField();
    fld->setFieldHeader(Fhdr);
	fld->setVolData((void *) data, Fhdr.volume_size,Mdvx::ENCODING_FLOAT32,Mdvx::SCALING_ROUNDED);
    //
    // Remap data, if desired.
    //
    if (P->RemapGrid){

      switch ( P->grid_projection){
	
      case Params::FLAT:

	if (fld->remap2Flat(lut, P->grid_nx, P->grid_ny,
			    P->grid_minx, P->grid_miny,
			    P->grid_dx, P->grid_dy,
			    P->grid_origin_lat, P->grid_origin_lon,
			    P->grid_rotation)){
	  cerr << "Re-map failed." << endl;
	  exit(-1);
	}
	
	break;
	
      case Params::LATLON:

	if (fld->remap2Latlon(lut, P->grid_nx, P->grid_ny,
			      P->grid_minx, P->grid_miny,
			      P->grid_dx, P->grid_dy)){
	  cerr << "Re-map failed." << endl;
	  exit(-1);
	}
	
	break;            
	
      case Params::LAMBERT:

  
	if (fld->remap2Lc2(lut, P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy,
			   P->grid_origin_lat, 
			   P->grid_origin_lon,
			   P->grid_lat1,  P->grid_lat2)){
	  cerr << "Re-map failed." << endl;
	  exit(-1);
	}
	
	break;
      
      default:
	cerr << "Unsupported projection." << endl;
	exit(-1);
	break;
      
      }   
    }            
    //
    // Write to MDV object.
    //

    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
  } // End of loop through fields.
  //
  // Free up the data and dimensional variables.
  //
  if(data) free(data);

  if (NC_NOERR != nc_close(ncID)){
    cerr << "Failed to close " <<  FilePath << endl;
  }
  //
  // Write MDV output.
  //  
  Mdvx::master_header_t OMhdr = Out.getMasterHeader();

  if (OMhdr.n_fields > 0){
    if (Out.writeToDir(P->OutUrl)) {
      cerr << "Failed to wite to " << P->OutUrl << endl;
      exit(-1);
    }      
  }

  return;

}

////////////////////////////////////////////////////////////
//
// destructor
//
WDTncdf2Mdv::~WDTncdf2Mdv (){



}
