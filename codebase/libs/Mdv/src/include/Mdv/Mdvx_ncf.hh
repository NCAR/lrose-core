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

///////////////////////////////////////////////////////////////
// converting MDV to netCDF CF

// data packing
  
typedef enum {
  NCF_PACK_FLOAT = 0, // FLOAT: 4-byte float
  NCF_PACK_SHORT = 1, // SHORT: 2-byte signed int
  NCF_PACK_BYTE = 2,  // BYTE: 1-byte signed int
  NCF_PACK_ASIS = 3   // ASIS: use packing as in MDV file
} ncf_pack_t;
  
// field translation specification

class Mdv2NcfFieldTrans {
public:
  string mdvFieldName;
  string ncfFieldName;
  string ncfStandardName;
  string ncfLongName;
  string ncfUnits;
  bool doLinearTransform;
  double linearMult;
  double linearOffset;
  ncf_pack_t packing;
};

// format of the netcdf file

typedef enum {
  NCF_FORMAT_CLASSIC = 0,         // Nc3File::Classic
  NCF_FORMAT_OFFSET64BITS = 1,    // Nc3File::Offset64Bits
  NCF_FORMAT_NETCFD4_CLASSIC = 2, // Nc3File::Netcdf4Classic
  NCF_FORMAT_NETCDF4 = 3          // Nc3File::Netcdf4
} nc_file_format_t;
  
// set NCF attributes for MDV to NetCDF CF conversion
  
void setMdv2NcfAttr(const string &institution,
                    const string &references,
                    const string &comment);
  

// replace comment attribute with input string.

void setMdv2NcfCommentAttr(const string &comment);
  

// set compression - uses HDF5

void setMdv2NcfCompression(bool compress,
                           int compressionLevel);
  
// set the output format of the netCDF file

void setMdv2NcfFormat(nc_file_format_t fileFormat);

// set radial data file types - radar and lidar
  
typedef enum {
  RADIAL_TYPE_CF = 0, // normal CF output
  RADIAL_TYPE_CF_RADIAL = 1, // CF radial output (radar and lidar)
  RADIAL_TYPE_DORADE = 2, // CF radial output (radar and lidar)
  RADIAL_TYPE_UF = 3 // CF radial output (radar and lidar)
} radial_file_type_t;
  
void setRadialFileType(radial_file_type_t fileType);
  
// set output parameters - what should be included
  
void setMdv2NcfOutput(bool outputLatlonArrays,
                      bool outputMdvAttr,
                      bool outputMdvChunks,
                      bool outputStartEndTimes = true);
  
// add field translation info for MDV to NetCDF CF conversion
  
void addMdv2NcfTrans(string mdvFieldName,
                     string ncfFieldName,
                     string ncfStandardName,
                     string ncfLongName,
                     string ncfUnits,
                     bool doLinearTransform,
                     double linearMult,
                     double linearOffset,
                     ncf_pack_t packing);
  
// clear MDV to NetCDF parameters
  
void clearMdv2Ncf();
  
// return string representation of packing type
  
static string ncfPack2Str(const ncf_pack_t packing);
  
// return enum representation of packing type
  
static ncf_pack_t ncfPack2Enum(const string &packing);

// return string representation of file format
  
static string ncFormat2Str(const nc_file_format_t format);
  
// return enum representation of file format
  
static nc_file_format_t ncFormat2Enum(const string &format);

// return string representation of file type

static string radialFileType2Str(const radial_file_type_t ftype);

// return enum representation of file type

static radial_file_type_t radialFileType2Enum(const string &ftype);

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

// clear NC format representation

void clearNcf();

//////////////////////////////////////////
// is specified format netCDF CF

bool isNcf(mdv_format_t format) const;

// print ncf info

void printNcfInfo(ostream &out) const;

// print mdv to ncf convert request

void printConvertMdv2NcfRequest(ostream &out) const;

#endif

    
