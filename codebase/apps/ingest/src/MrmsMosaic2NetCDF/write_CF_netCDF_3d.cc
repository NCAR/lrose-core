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
#include <iostream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <netcdf.h>

#include "func_prototype.h"

using namespace std;


// C O N S T A N T S
// none


// F U N C T I O N S 

/*------------------------------------------------------------------

	Method:		write_CF_netCDF_3d
	
	
	Purpose:	This method will write out to file a single
	            variable CF-compliant netCDF containing 3D data. Data 
	            definition will be
	             [number of levels][number of rows][number of columns] 
	             
	            All required data and info are based on input 
	            variables.  The final file is gzip'd
	
	Input:      outputfile = string storing full file path and name
				dataType = type of field (e.g., LatLonGrid)
				longName = long data field name
				varName = short data field name
				varUnit = data field unit
				nx, ny = number of columns and rows in data field
				dx, dy = resolution of field in longitude and latitude (degrees)
				nw_lat = latitude of center of Northwest grid cell
				nw_lon = longitude of center of Northwest grid cell
				height = height of field (meter MSL)
				epoch_time = valid time of field in epoch seconds
				fractional_time = sub-second valid time of field
				missing_value = missing data flag
				range_folded_value = range folded data flag
				data_1D = set of 2D data fields stored as a row-major 1D array
				gzip_flag = set to 1 and function will gzip output if NETCDF_4_OUT is not defined
				            or activate the native compression if NETCDF_4_OUT is defined.
	
	Output:		Single variable CF-compliant netCDF
				int indicating success or failure
	
------------------------------------------------------------------*/

int write_CF_netCDF_3d( string outputfile, string dataType, 
                   string longName, string varName, string varUnit,
                   int nx, int ny, int nz, float dx, float dy, 
                   float nw_lat, float nw_lon, float heights[],
                   long epoch_time, float fractional_time,
                   float missing_value, float range_folded_value,
                   float* data_1D, int gzip_flag )
{
    /*-----------------------------*/
    /*** 0. Handle trivial cases ***/
    /*-----------------------------*/

    //No data field
    if(data_1D == 0) return 0;
    

    //Try to create and open the NetCDF output file 
    int file_handle;
    //int stat = nc_create(outputfile.c_str(), NC_CLOBBER, &file_handle);
#ifdef NETCDF_4_OUT
    int stat = nc_create(outputfile.c_str(), NC_NETCDF4, &file_handle);
#else
    int stat = nc_create(outputfile.c_str(), NC_64BIT_OFFSET, &file_handle);
#endif
    //Need to use NC_64BIT_OFFSET because the resulting file is likely huge!
    //http://www.unidata.ucar.edu/software/netcdf/docs/netcdf/Large-File-Support.html
    //http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-c/nc_005fcreate.html
    check_err(stat,__LINE__,__FILE__); //exit if fail



    /*--------------------------*/
    /*** 1. Declare variables ***/
    /*--------------------------*/
   
    /*** NetCDF related variables ***/  
    bool write_error = false;

    // dimension ids 
    int lat_dim_ID;
    int lon_dim_ID;
    int z_dim_ID;
    int time_dim_ID;
       
    // dimension lengths 
    size_t lat_len;
    size_t lon_len;
    size_t time_len;
    size_t z_len;
   
    // variable ids 
    int varID, latID, lonID, zID, timeID;
    int var_dims[3], lat_dims[1], lon_dims[1], z_dims[1], time_dims[1];
    
   
    /*** Data array variables ***/
    float *lat_1d = 0;
    float *lon_1d = 0;
    float *z_1d = 0;
    double *time_1d = 0;
   
   
    /*** Misc. variables ***/
    float fltArray[1];
    long  longArray[1];
    char charArray[NC_MAX_NAME];
   
   
   
    /*------------------------------------------------------*/
    /*** 2. Fill 1D array with values from the input data ***/
    /*------------------------------------------------------*/
    
    //For Ht
    // [0] = lowest; [last] = highest
    z_1d = new float [nz];
    for(int k = 0; k < nz; k++)
      z_1d[k] = heights[k];
      
    //For Longitude
    // [0] = West lon; [last] = East lon
    lon_1d = new float [nx];
    for(int i = 0; i < nx; i++)
      lon_1d[i] = nw_lon + dx*i;
    
    //For Latitude
    // [0] = North lat; [last] = South lat
    lat_1d = new float [ny];
    for(int j = 0; j < ny; j++)
      lat_1d[j] = nw_lat - dy*j;
      
    //Set time
    time_1d = new double [1];
    time_1d[0] = (double)epoch_time;



    /*-------------------------------------------------------------*/
    /*** 3. Enter define mode and build variables and attributes ***/
    /*-------------------------------------------------------------*/     

    /*** 3A. Define dimensions of variable. ***/ 

    z_len = nz;
    stat = nc_def_dim(file_handle, "Ht", z_len, &z_dim_ID);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    lat_len = ny;
    stat = nc_def_dim(file_handle, "Lat", lat_len, &lat_dim_ID);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    lon_len = nx;
    stat = nc_def_dim(file_handle, "Lon", lon_len, &lon_dim_ID);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    time_len = 1;
    stat = nc_def_dim(file_handle, "time", time_len, &time_dim_ID);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
      
    if(!write_error)
    {
      //Write out uncompressed data
      //Define variable size
      var_dims[0] = z_dim_ID;
      var_dims[1] = lat_dim_ID;
      var_dims[2] = lon_dim_ID;

      strcpy(charArray, varName.c_str());
      stat = nc_def_var(file_handle, charArray, NC_FLOAT, 3, var_dims, &varID);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

#ifdef NETCDF_4_OUT
      if(gzip_flag)
      {
        int shuffle, deflate, deflate_level;
        /* Set chunking, shuffle, and deflate. */
        shuffle = NC_SHUFFLE;
        deflate = 1;
        deflate_level = 1;
  
        stat = nc_def_var_deflate(file_handle, varID, shuffle, deflate,deflate_level);
        if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
      }
#endif
    }

    //height
    if(!write_error)
    {
      //Write out uncompressed data
      //Define variable size
      z_dims[0] = z_dim_ID;

      strcpy(charArray, "Ht");
      stat = nc_def_var(file_handle, charArray, NC_FLOAT, 1, z_dims, &zID);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }

    //latitude
    if(!write_error)
    {
      //Write out uncompressed data
      //Define variable size
      lat_dims[0] = lat_dim_ID;

      strcpy(charArray, "Lat");
      stat = nc_def_var(file_handle, charArray, NC_FLOAT, 1, lat_dims, &latID);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }

    //longitude
    if(!write_error)
    {
      //Write out uncompressed data
      //Define variable size
      lon_dims[0] = lon_dim_ID;

      strcpy(charArray, "Lon");
      stat = nc_def_var(file_handle, charArray, NC_FLOAT, 1, lon_dims, &lonID);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }

    //time
    if(!write_error)
    {
      //Write out uncompressed data
      //Define variable size
      time_dims[0] = time_dim_ID;

      strcpy(charArray, "time");
      stat = nc_def_var(file_handle, charArray, NC_DOUBLE, 1, time_dims, &timeID);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }

    //Error check
    if(write_error)
    {
      delete [] z_1d;
      delete [] lat_1d;
      delete [] lon_1d;
      delete [] time_1d;
      return -1;
      
    }//end write_error if-blk


    /*** 3B. Write out attributes of main variable ***/ 
    /***     and reference variables               ***/

    strcpy(charArray, varUnit.c_str());
    stat = nc_put_att_text(file_handle, varID, "units", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
   
    strcpy(charArray, longName.c_str());
    stat = nc_put_att_text(file_handle, varID, "long_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    fltArray[0] = missing_value;
    stat = nc_put_att_float(file_handle, varID, "_FillValue", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    
    //For height
    strcpy(charArray, "height of mosaic levels (MSL)");
    stat = nc_put_att_text(file_handle, zID, "long_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    strcpy(charArray, "meters");
    stat = nc_put_att_text(file_handle, zID, "units", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    strcpy(charArray, "up");
    stat = nc_put_att_text(file_handle, zID, "positive", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    

    //For latitude
    strcpy(charArray, "latitude");
    stat = nc_put_att_text(file_handle, latID, "long_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    
    strcpy(charArray, "degrees_north");
    stat = nc_put_att_text(file_handle, latID, "units", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    strcpy(charArray, "latitude");
    stat = nc_put_att_text(file_handle, latID, "standard_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    

    //For longitude
    strcpy(charArray, "longitude");
    stat = nc_put_att_text(file_handle, lonID, "long_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    
    strcpy(charArray, "degrees_east");
    stat = nc_put_att_text(file_handle, lonID, "units", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    strcpy(charArray, "longitude");
    stat = nc_put_att_text(file_handle, lonID, "standard_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;


    //For time
    strcpy(charArray, "time");
    stat = nc_put_att_text(file_handle, timeID, "long_name", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    strcpy(charArray, "seconds since 1970-1-1 0:0:0");
    stat = nc_put_att_text(file_handle, timeID, "units", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
   
    strcpy(charArray, "Time");
    stat = nc_put_att_text(file_handle, timeID, "_CoordinateAxisType", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;



    /*** 3C. Write out global attributes.  Note        ***/ 
    /***     most are carried over from WDSS-II netCDF ***/
    
    strcpy(charArray, varName.c_str());
    stat = nc_put_att_text(file_handle, NC_GLOBAL, "TypeName", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    strcpy(charArray, "LatLonHeightGrid");
    stat = nc_put_att_text(file_handle, NC_GLOBAL, "DataType", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
        
    fltArray[0] = nw_lat;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "Latitude", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    fltArray[0] = nw_lon;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "Longitude", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    longArray[0] = epoch_time;
    stat = nc_put_att_long(file_handle, NC_GLOBAL, "Time", NC_LONG, 1, longArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    fltArray[0] = fractional_time;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "FractionalTime", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;

    fltArray[0] = dy;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "LatGridSpacing", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
   
    fltArray[0] = dx;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "LonGridSpacing", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
   
    fltArray[0] = missing_value;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "MissingData", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
   
    fltArray[0] = range_folded_value;
    stat = nc_put_att_float(file_handle, NC_GLOBAL, "RangeFolded", NC_FLOAT, 1, fltArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    
    //write empty extra attributes (WDSS-II thing)
    strcpy(charArray, "");
    stat = nc_put_att_text(file_handle, varID, "attributes", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;    

    strcpy(charArray, "NMQ Product");
    stat = nc_put_att_text(file_handle, NC_GLOBAL, "title", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;

    strcpy(charArray, "NSSL");
    stat = nc_put_att_text(file_handle, NC_GLOBAL, "institution", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;

    strcpy(charArray, "CF-1.4");
    stat = nc_put_att_text(file_handle, NC_GLOBAL, "Conventions", strlen(charArray), charArray);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) return -1;

    
   
    /*** 3D. Leave define mode ***/ 
    
    stat = nc_enddef (file_handle);
    if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
     
   
   
    /*--------------------------------------*/
    /*** 4. Write variable values to file ***/
    /*--------------------------------------*/

    if(!write_error)
    {
      //Write out main variable data
      stat = nc_put_var_float(file_handle, varID, data_1D);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
            
    }
    
    if(!write_error)
    {  
      stat = nc_put_var_float(file_handle, zID, z_1d);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }

    if(!write_error)
    {  
      stat = nc_put_var_float(file_handle, latID, lat_1d);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }
    
    if(!write_error)
    {
      stat = nc_put_var_float(file_handle, lonID, lon_1d);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }
    
    if(!write_error)
    {
      stat = nc_put_var_double(file_handle, timeID, time_1d);
      if(soft_check_err_wrt(stat,__LINE__,__FILE__) < 0) write_error = true;
    }
    //end write_error if-blks 
    
	    
    /*----------------------------*/   
    /*** 5. Close file and gzip ***/
    /*----------------------------*/  
    
    //Closing NetCDF file
    nc_close(file_handle);
    
    
#ifndef NETCDF_4_OUT
    //gzip file
    if(gzip_flag)
    {
      int length = outputfile.length() + 25;
      char command[ length ];
    
      sprintf(command, "gzip -f -q %s", outputfile.c_str());       
      system( command );
    }
#endif


    /*----------------------------------*/   
    /*** 6. Free-up memory and return ***/
    /*----------------------------------*/  
    
    delete [] z_1d;
    delete [] lat_1d;
    delete [] lon_1d;
    delete [] time_1d;
    
    if(write_error) return -1;
    else return 1;
    
}//end function write_CF_netCDF_3d
