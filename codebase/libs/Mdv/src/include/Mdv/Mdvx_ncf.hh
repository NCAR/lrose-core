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
////////////////////////////////////////////////
//
// Mdvx_ncf.hh
//
// NetCDF CF functions for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh

#ifdef _in_Mdvx_hh

// set NCF data in MDVX object
//
// For forecast data, the valid time represents the time at which
// the forecast is valid, and forecastLeadSecs is the time from the
// forecast reference time (gen time) to the valid time.

void setNcf(const void *ncBuf,
            int nbytes,
            time_t validTime,
            bool isForecast = false,
            int forecastLeadSecs = 0,
            int epoch = 0);

// set just the NCF header in MDVX object
//
// For forecast data, the valid time represents the time at which
// the forecast is valid, and forecastLeadSecs is the time from the
// forecast reference time (gen time) to the valid time.

void setNcfHeader(time_t validTime,
		  bool isForecast = false,
		  int forecastLeadSecs = 0,
		  int epoch = 0);

// set just the NCF buffer in MDVX object

void setNcfBuffer(const void *ncBuf,
		  int nbytes);

// set the suffix for the netCDF file
// file name will end .mdv.suffix.nc

void setNcfFileSuffix(const string &suffix);

// set whether NCF data is constrained
// i.e. have read constraints been applied?
// When a file is first read, _ncfContrained is set false.
// It is set true after constrainNcf() is called.

void setConstrained(bool state);

// clear NC format representation

void clearNcf();

//////////////////////////////////////////
// is specified format netCDF CF

bool isNcf(mdv_format_t format) const;

// print ncf info

void printNcfInfo(ostream &out) const;

// get ncf file extension
// Adds in suffix if appropriate

string getNcfExt() const;

#endif

    
