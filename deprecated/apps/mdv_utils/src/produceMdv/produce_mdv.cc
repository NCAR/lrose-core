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

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <cstdio>
#include <toolsa/umisc.h>

extern "C" {

//
// Routine to write data in MDV format, designed to be callable
// by Fortran. Niles Oien March 2005.
//


// Small file-scoped routine.
void parseNthItem(int n, char *inString, char *outString, int strLen);



// Allow for the fact that we may be using the Portland group
// compiler, which puts a different number of underscores after
// the routine name when compiling.
//
#ifdef PGI_IN_USE
  void produce_mdv_(
		    char *fieldNames,                          // Field names, separated by '^'
		    char *url,                                 // URL to write to, terminated with a '^'
		    char *units,                               // Units to use, separated by '^'
		    int *debug,                                // 0 => quiet, 1 => print some debugging, 2 => more debugging.
		    int *nx, int *ny, int *nz,                 // Array dimensions.
		    float *dx, float *dy,                      // Grid cell sizes in x,y.
		    float *minx, float *miny,                  // Grid minimums. See note below.
		    float *originLat, float *originLon,        // Origin of grid.
		    int *vlevelType,                           // Vertical level type. See note below.
		    float *vlevels,                            // Array of nz heights for data planes.
		    int *gridType,                             // grid type. See note below.
		    float *gridRotation,                       // Rotation for flat earth projections.
		    float *lambertLat1, 
		    float *lambertLat2,                        // True latitudes for lambert conformal projection.
		    float *badVal,                             // Value to use for bad or missing data.
		    int *year, int *month, int *day,
		    int *hour, int *minute, int *second,       // Time values used only if unixtime is 0.
		    time_t *unixtime,                          // Time.
		    int *expiry,                               // How long data are valid for, seconds.
		    float *fieldData,                          // Actual field data. Array size is nx*ny*nz
		    int *forecastLeadTime,                     // in seconds.
		    int *writeAsForecast,                      // 0 => Do not write as forecast, else do
		    int *numFields,                            // Number of fields to write.
		    int *encodingType,                         // See note below.
		    int *wentOK,                               // 0 => did not go OK, else it did.
		    int lenFieldStr,
		    int lenUrlStr,
		    int lenUnitsStr){
#else
    void produce_mdv__(
		     char *fieldNames,                          // Field names, separated by '^'
		     char *url,                                 // URL to write to, terminated with a '^'
		     char *units,                               // Units to use for each field, separated by '^'
		     int *debug,                                // 0 => quiet, 1 => print some debugging, 2 => more debugging.
		     int *nx, int *ny, int *nz,                 // Array dimensions.
		     float *dx, float *dy,                      // Grid cell sizes in x,y.
		     float *minx, float *miny,                  // Grid minimums. See note below.
		     float *originLat, float *originLon,        // Origin of grid.
		     int *vlevelType,                           // Vertical level type. See note below.
		     float *vlevels,                            // Array of nz heights for data planes.
		     int *gridType,                             // grid type. See note below.
		     float *gridRotation,                       // Rotation for flat earth projections.
		     float *lambertLat1, 
		     float *lambertLat2,                        // True latitudes for lambert conformal projection.
		     float *badVal,                             // Value to use for bad or missing data.
		     int *year, int *month, int *day,
		     int *hour, int *minute, int *second,       // Time values used only if unixtime is 0.
		     time_t *unixtime,                          // Time.
		     int *expiry,                               // How long data are valid for, seconds.
		     float *fieldData,                          // Actual field data. Array size is nx*ny*nz
		     int *forecastLeadTime,                     // in seconds.
		     int *writeAsForecast,                      // 0 => Do not write as forecast, else do
		     int *numFields,                            // Number of fields to write.
		     int *encodingType,                         // See note below.
		     int *wentOK,                               // 0 => did not go OK, else it did.
		     int lenFieldStr,
		     int lenUrlStr,
		     int lenUnitsStr){
#endif  

      //
      // Input arguments :
      //
      //  char *fieldNames - list of field names to output, separated by the '^' character.
      //                     For example, setting this to 'uwind^vwind^temp^' would output
      //                     fields name uwind, vwind and temp.
      //
      //  char *url - the MDV URL to write to, terminated by a '^' character. Setting this
      //              to 'mdvp:://localhost::./mdv/adjoint^' would write to the URL
      //              mdvp:://localhost::./mdv/adjoint
      //
      //  char *units - list of units for each field, separated by the '^' character. Setting
      //                this to 'm/s^m/s^Celcius^' would create an mdv file with units
      //                three fields having units m/s, m/s and Celcius.
      //
      //  int *debug - 0 => Only report failure to write 1 => Some debugging 2 or more => more debugging.
      //
      //  int *nx, int *ny, int *nz - Grid size.
      //
      //  float *dx, float *dy -  Grid resolution. Usually in Km but for lat/lon projection it's in degrees.
      //
      //  float *minx, float *miny - For latlon projection, lat/lon of center of lower left grid point.
      //                            For other projections, distance from origin to center of lower left grid point.
      //
      //  float *originLat, float *originLon - origin for grid.
      //
      // int *vlevelType - type of vertical levels :
      //                     
      //                     1 => VERT_TYPE_SURFACE (NZ = 1)
      //                     2 => VERT_TYPE_SIGMA_P
      //                     3 => VERT_TYPE_PRESSURE
      //                     4 => VERT_TYPE_Z (in Km)
      //                     5 => VERT_TYPE_SIGMA_Z
      //                     6 => VERT_TYPE_ETA
      //                     7 => VERT_TYPE_THETA
      //                     8 => VERT_TYPE_MIXED
      //                     9 => VERT_TYPE_ELEV (Degrees of elevation)
      //
      //  float *vlevels - Array of NZ vertical levels.
      //
      // int *gridType - projection to use :
      //
      //                    1 => PROJ_LAMBERT_CONF
      //                    2 => PROJ_FLAT
      //                    3 => PROJ_LATLON
      //
      // float *gridRotation - Used in flat earth projection only. Generally 0.0
      //
      // float *lambertLat1, float *lambertLat2 - Lambert conformal true latitudes.
      //                                          Used for lambert projection only.
      //
      // float *badVal - value for bad or missing data. Set to -9999.0 or some such value.
      //
      // int *year, int *month, int *day, int *hour, int *minute, int *second, time_t *unixtime :
      //   If these determine the generation time of the data. If unixtime is non-zero,
      //   it is used, otherwise the year, month, day, hour, minute, second are used.
      //
      //  int *expiry - How long data are considered valid, seconds. Depends on data
      //                frequency. If data come in every hour, set to one hour (3600 seconds).
      //                Used primarily for display of data.
      //
      // float *fieldData - One dimensional array of data to write to the fields. Index as :
      //                     INDEX = (IFIELD-1) *NX*NY*NZ + (IZ-1)*NX*NY + (IY-1)*NX + (IX-1) + 1
      //                    Data start at lower left corner, X changes fastest in memory, then Y,
      //                    then Z.
      //
      // int *forecastLeadTime - Forecast lead time, in seconds. Set to 0 for non-forecast data.
      //
      // int *writeAsForecast - 0 => Use file names like mdv/non_fcst/20050212/013000.mdv
      //                     else => Use file names like mdv/pls_fcst/20050212/g_013000/f_00003600.mdv
      //
      // int *numFields - How many fields to write.
      //
      // int *encodingType - How data in the mdv file are to be encoded.
      //                     1 => encode as bytes with a scale and a bias. This
      //                          is the most compact although it rounds the data off
      //                          a little by quatizing it into 255 discrete steps.
      //                          This is used for storing measured data like radar reflectivity
      //                          and it seems to work well.
      //
      //                     2 => encode as 16 bit integers with scale and bias. This is a nice
      //                          tradeoff between file size and data precision, in my opinion.
      //
      //                     3 => Store the floating point values directly with no loss of
      //                          precision. File sizes increase with this option. Used for storing
      //                          model run data which may be used to initialize the model - in
      //                          this case small roundoff errors in the initial state can take
      //                          the chaotic model system to radically different output states.
      //                         
      //
      // int *wentOK - passed back to caller. 1 => Write went well, 0 => write failed.
      //
      // int lenFieldStr, int lenUrlStr, int lenUnitsStr - These are added automatically if this routine is
      //                                                   called from Fortran. Ignored.
      //

      *wentOK = 0; // start off pessimisstic.
    
      //
      // First, get the time for these data. if unixtime is 0, then
      // user the year, month, day, hour, minute and second values,
      // otherwise use the unixtime.
      //
      date_time_t dataTime;
      if (*unixtime == 0){
	dataTime.year = *year; dataTime.month = *month; dataTime.day = *day;
	dataTime.hour = *hour; dataTime.min = *minute; dataTime.sec = *second;
	uconvert_to_utime( &dataTime );
      } else {
	dataTime.unix_time = *unixtime;
	uconvert_from_utime( &dataTime );
      }

      //
      // Get the URL string.
      //
      const int strLen = 256;
      char url_out[strLen];

      int exitOK = 0;
      for (int in=0; in < strLen; in++){
	if (url[in] == '^'){
	  url_out[in] = char(0);
	  exitOK = 1;
	  break;
	} else {
	  url_out[in] = url[in];
	}
      }

      if (!(exitOK)){
	fprintf(stderr, "Failed to parse URL from %s\n", url);
	return;
      }

      if (*debug){
	fprintf(stderr, "Writing mdv data at time %s\n", utimstr(dataTime.unix_time));
	fprintf(stderr, " to %s\n", url_out);
      }

      //
      // Set up the structures we need to produce MDV data.
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
      switch ( *vlevelType ) {

      case 1 :
	vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
	break;

      case 2 :
	vhdr.type[0] = Mdvx::VERT_TYPE_SIGMA_P;
	break;

      case 3 :
	vhdr.type[0] = Mdvx::VERT_TYPE_PRESSURE;
	break;

      case 4 :
	vhdr.type[0] = Mdvx::VERT_TYPE_Z;
	break;

      case 5 :
	vhdr.type[0] = Mdvx::VERT_TYPE_SIGMA_Z;
	break;

      case 6 :
	vhdr.type[0] = Mdvx::VERT_TYPE_ETA;
	break;

      case 7 :
	vhdr.type[0] = Mdvx::VERT_TYPE_THETA;
	break;

      case 8 :
	vhdr.type[0] = Mdvx::VERT_TYPE_MIXED;
	break;

      case 9 :
	vhdr.type[0] = Mdvx::VERT_TYPE_ELEV;
	break;

      default :
	fprintf(stderr,"Unknown vertical type.\n");
	return;
	break;

      }

      for (int iz = 0; iz < *nz; iz++){
	vhdr.level[iz] = vlevels[iz];
      }

      //
      // Now, set up the field header.
      //
      fhdr.nx = *nx;
      fhdr.ny = *ny;
      fhdr.nz = *nz;
      //
      fhdr.grid_dx = *dx;
      fhdr.grid_dy = *dy;
      if (fhdr.nz < 2) {
	fhdr.grid_dz = 1.0;
      } else {
	fhdr.grid_dz = fabs( vhdr.level[1] - vhdr.level[0] );
      }
      //
      switch ( *gridType ) {

      case 1 :
	fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
	break;

      case 2 :
	fhdr.proj_type = Mdvx::PROJ_FLAT;
	break;

      case 3 :
	fhdr.proj_type = Mdvx::PROJ_LATLON;
	break;

      default :
	fprintf(stderr, "Unknown grid type for MDV write!\n");
	return;
	break;

      }
      //
      fhdr.proj_origin_lat =  *originLat;
      fhdr.proj_origin_lon =  *originLon;
      //
      //
      fhdr.grid_minx = *minx;
      fhdr.grid_miny = *miny;
      fhdr.grid_minz = vhdr.level[0];
      //
      // For Lambert Conformal, the first two proj params are set to the
      // lambert conformal latitudes.
      //
      if ( fhdr.proj_type == Mdvx::PROJ_LAMBERT_CONF){
	fhdr.proj_param[0] = *lambertLat1;
	fhdr.proj_param[1] = *lambertLat2;
      }
      //
      if ( fhdr.proj_type == Mdvx::PROJ_FLAT){
	fhdr.proj_param[0] = *gridRotation;
	fhdr.proj_rotation = *gridRotation;
      }
      //
      // Set up an uncompressed grid of floating point values.
      //
      fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
      fhdr.data_element_nbytes = sizeof(fl32);
      fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
      fhdr.compression_type = Mdvx::COMPRESSION_NONE;
      fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
      fhdr.scaling_type = Mdvx::SCALING_NONE;     
      //
      // State what vlevel type we have.
      //
      fhdr.native_vlevel_type = vhdr.type[0];
      fhdr.vlevel_type = vhdr.type[0];
      //
      //
      // Set up some things in the master header.
      //
      Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
      Mhdr.num_data_times = 1;
      Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
      Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
      Mhdr.native_vlevel_type = vhdr.type[0];
      Mhdr.vlevel_included = 1;
      //
      //
      sprintf(Mhdr.data_set_info,"%s","produceMdv");
      sprintf(Mhdr.data_set_name,"%s","produceMdv");
      sprintf(Mhdr.data_set_source,"%s", "produceMdv");
      //
      // Set the times in the master and field headers.
      //
      Mhdr.time_gen = dataTime.unix_time;
      Mhdr.time_begin = dataTime.unix_time + *forecastLeadTime;
      Mhdr.time_end = dataTime.unix_time + *forecastLeadTime;
      Mhdr.time_expire = dataTime.unix_time + *expiry + *forecastLeadTime;
      Mhdr.time_centroid = dataTime.unix_time + *forecastLeadTime;
      //
      fhdr.forecast_time = Mhdr.time_centroid + *forecastLeadTime;
      //
      fhdr.forecast_delta = *forecastLeadTime;

      fhdr.bad_data_value = *badVal;   fhdr.missing_data_value = *badVal;
      //

      //
      // Declare a DsMdvx object so we can load it up with fields.
      //
      DsMdvx outMdvx;
      //
      outMdvx.setMasterHeader( Mhdr ); 
      outMdvx.clearFields();
      //
      // Start looping through the fields.
      //
      for (int ifld=0; ifld < *numFields; ifld++){

	char field_name[strLen];
	char units_out[strLen];

	parseNthItem(ifld, fieldNames, field_name, strLen);
	parseNthItem(ifld, units,      units_out, strLen);

	sprintf( fhdr.field_name_long,"%s", field_name);
	sprintf( fhdr.field_name,"%s", field_name);
	sprintf( fhdr.units,"%s", units_out);
	sprintf( fhdr.transform,"%s","none");

	if (*debug){
	  fprintf(stderr,"  Adding field %d %s units %s\n", ifld+1, field_name, units_out);
	}

	float *currentFieldData = fieldData + ifld * *nx * *ny * *nz;

	//
	// Do much more debugging if requested - actually look at the data.
	//
	if (*debug > 1){
	  float min=0.0, max=0.0; 
	  float total = 0.0;
	  int numGood = 0;
	  int first = 1;
	  //
	  for (int il = 0; il < *nx * *ny * *nz; il++){
	    if (currentFieldData[il] != *badVal){
	      numGood++;
	      total += currentFieldData[il];
	      if (first){
		first = 0;
		min = currentFieldData[il]; max = currentFieldData[il];
	      } else {
		if (currentFieldData[il] < min) min = currentFieldData[il];
		if (currentFieldData[il] > max) max = currentFieldData[il];
	      }
	    }
	  }

	  fprintf(stderr,"  Percent of data that are not set to the missing value : ");
	  fprintf(stderr, "%f\n", 100.0*double(numGood)/double( *nx * *ny * *nz ));
	  if (numGood){
	    fprintf(stderr, "  Data range from %f to %f\n", min, max);
	    fprintf(stderr,"   Mean : %f\n", total / double(numGood));
	  }
	}

	Mdvx::encoding_type_t Encoding = Mdvx::ENCODING_INT16;

	switch ( *encodingType ){

	case 1 :
	  Encoding = Mdvx::ENCODING_INT8;
	  break;
	
	case 2 :
	  Encoding = Mdvx::ENCODING_INT16;
	  break;

	case 3 :
	  Encoding = Mdvx::ENCODING_FLOAT32;
	  break;

	default :
	  fprintf(stderr,"Unrecognized encoding type!\n");
	  return;
	  break;

	}

	//
	MdvxField *field;
	//
	field = new MdvxField(fhdr, vhdr, currentFieldData );
	//
	if (field->convertRounded(Encoding,
				  Mdvx::COMPRESSION_ZLIB)){
	  fprintf(stderr, "conversion of field failed - I cannot go on.\n");
	  exit(-1);
	}
      
	outMdvx.addField(field);
      
      } // End of loop through the fields.

      if (*writeAsForecast){
	outMdvx.setWriteAsForecast();
      }
    
      if (*debug){
	fprintf(stderr,"Writing to %s\n", url_out);
      }


      if (outMdvx.writeToDir(url_out)) {
	fprintf(stderr,"MDV write failed!\n");
	return;
      }

      *wentOK = 1;

      if (*debug){
	fprintf(stderr,"MDV write successful.\n");
      }

      return;

}


void parseNthItem(int n, char *inString, char *outString, int strLen){
  //
  // Small routine to get the nth item from a carat separated string.
  // n=0 returns first one. Returns empty string if no luck.
  //
  outString[0] = char(0);

  int ip = 0;
  int numSeparatorsFound = 0;

  while (numSeparatorsFound != n){
    ip++;
    if (inString[ip] == '^') numSeparatorsFound++;
    if (ip == strLen) return;
  }

  if (inString[ip] == '^') ip++;

  int ij = 0;
  int go = 1;
  do {
    outString[ij] = inString[ip];
    ip++; ij++;
    if (ip == strLen){
      outString[0] = char(0);
      return;
    }
    if (inString[ip] == '^'){
      outString[ij] = char(0);
      go = 0;
    }

  } while(go);


  return;

}
  
}
