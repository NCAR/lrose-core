/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifndef FUNC_PROTOTYPE_H
#define FUNC_PROTOTYPE_H

#include <vector>
#include <string>

#include "ProductInfo.h"
#include "HeaderAttribute.h"

using namespace std;

vector<ProductInfo> setupMRMS_ProductRefData( );
                   
short int* mrms_binary_reader_cart3d(const char *vfname,                     
                     char *varname, char *varunit,
                     int &nradars, vector<string> &radarnam,
                     int &var_scale, int &missing_val,
                     float &nw_lon, float &nw_lat,
                     int &nx, int &ny, float &dx, float &dy,
                     float zhgt[], int &nz, long &epoch_seconds,
                     int swap_flag);
                   
int write_CF_netCDF_2d( string outputfile, string dataType, 
                   string longName, string varName, string varUnit,
                   int nx, int ny, float dx, float dy, 
                   float nw_lat, float nw_lon, float height,
                   long epoch_time, float fractional_time,
                   string cf_time_string, long cf_fcst_length,
                   vector<HeaderAttribute>& attrs, 
                   float missing_value, float range_folded_value,
                   float* data_1D, int gzip_flag);
                   
int write_CF_netCDF_2d_FAA( string outputfile, string dataType, 
                   string longName, string varName, string varUnit,
                   int nx, int ny, float dx, float dy, 
                   float nw_lat, float nw_lon, float height,
                   long epoch_time, float fractional_time,
                   string cf_time_string, long cf_fcst_length,
                   vector<HeaderAttribute>& attrs, 
                   float missing_value, float range_folded_value,
                   float* data_1D, int gzip_flag);
                   
int write_CF_netCDF_3d( string outputfile, string dataType, 
                   string longName, string varName, string varUnit,
                   int nx, int ny, int nz, float dx, float dy, 
                   float nw_lat, float nw_lon, float heights[],
                   long epoch_time, float fractional_time,
                   float missing_value, float range_folded_value,
                   float* data_1D, int gzip_flag );
                                      
int write_CF_netCDF_3d_FAA( string outputfile, string dataType, 
                   string longName, string varName, string varUnit,
                   int nx, int ny, int nz, float dx, float dy, 
                   float nw_lat, float nw_lon, float heights[],
                   long epoch_time, float fractional_time,
                   float missing_value, float range_folded_value,
                   float* data_1D, int gzip_flag );

void check_err(const int stat, const int line, const char *file);
int soft_check_err_wrt(const int stat, const int line, const char *file); 
int write_extra_attributes(int file_handle, int varID, vector<HeaderAttribute>& attrs);

#endif

