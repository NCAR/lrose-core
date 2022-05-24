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
// Mdvx_read.hh
//
// Read functions for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

//////////////////////////////
// Setting up to read

// clear all read requests, set defaults

virtual void clearRead();

////////////////////////////////////////////////////////////////////////
// set read time
//
// You must either set the readTime or readPath.
//
// If you specify read time, the path is determined from the dir,
// the search_time and the search mode.
//
// Search modes are as follows:
//
//   READ_LAST: read last available data set
//   READ_CLOSEST: read closest data within search_margin of search_time
//   READ_FIRST_BEFORE: read first data at or before search_time,
//                      within search_margin
//   READ_FIRST_AFTER:  read first data at or after search_time,
//                      within search_margin
//   READ_BEST_FORECAST: read best forecast within 
//                       search_margin of search_time.
//                       Takes data from latest available gen time.
//   READ_SPECIFIED_FORECAST: read forecast generated at search time,
//                            closest to forecast_lead_time,
//                            within search_margin

virtual void setReadTime(const read_search_mode_t mode,
                         const string &read_dir,
                         const int search_margin = 0,
                         const time_t search_time = 0,
                         const int forecast_lead_time = 0);

virtual void clearReadTime();

/////////////////////////////////////////////////////////////////
// Constrain the lead times to be considered in the time search,
// if data is stored in forecast format.
//
// Does not apply to READ_SPECIFIED_FORECAST mode.
//
// Only forecast lead times within the specified limits will
// be considered.
//
// Also, you can specify that the search_time is the generate time
// rather than valid time. The valid time will be computed as
// the request_time plus the mean of the min and max lead times.
//
// If request_by_gen_time is true, the gen time list will be returned
// in both the valid and gen time list.

void setConstrainFcastLeadTimes(int min_lead_time,
                                int max_lead_time,
                                bool request_by_gen_time = false);

void clearConstrainFcastLeadTimes();
  
////////////////////////////////////////////////////////////////////////
// set read path
//
// You must either set the readTime or readPath.
// If you specify the path, that exact path will be used.

virtual void setReadPath(const string &read_path);
virtual void clearReadPath();

//////////////////////////////////////////////////////////////////
// set field requests
// If fields are specified, they will be read in the order they are
// added and will exist in the Mdvx object in that order

void addReadField(const int field_num);
void addReadField(const string &field_name);
void clearReadFields();

/////////////////////
// set chunk requests

// clear chunk list
void clearReadChunks();

// add a chunk num to the list
void addReadChunk(const int chunk_num);

// set no chunks to be read
void setReadNoChunks();

/////////////////////////////////
// set or clear horizontal limits

void setReadHorizLimits(const double min_lat,
                        const double min_lon,
                        const double max_lat,
                        const double max_lon);

void clearReadHorizLimits();

///////////////////////////////
// set or clear vertical limits

// vlevel limits - min and max vlevel values in floating point

void setReadVlevelLimits(const double min_vlevel,
                         const double max_vlevel);

// plane num limits - min and max plane nums as integers

void setReadPlaneNumLimits(const int min_plane_num,
                           const int max_plane_num);

void clearReadVertLimits();

/////////////////
// set composite

void setReadComposite();   // set composite on
void clearReadComposite(); // set composite off

///////////////////
// set fill missing
//
// Set to fill missing data in planes (horiz and vert) with the
// surrounding missing values.

void setReadFillMissing();   // set fill on
void clearReadFillMissing(); // set fill off

//////////////////////////////////////////////
// set encoding, compression and scaling types

void setReadEncodingType(const encoding_type_t encoding_type);
void clearReadEncodingType();

void setReadCompressionType(const compression_type_t compression_type);
void clearReadCompressionType();

void setReadScalingType(const scaling_type_t scaling_type,
                        const double scale = 1.0,
                        const double bias = 0.0);
void clearReadScalingType();

/////////////////////////////////////////////
// number of sample points for vert section
//
// If nsamples is not set it will be computed based on
// the resolution of the grid data.

void setReadNVsectSamples(int n_samples);
void clearReadNVsectSamples();

////////////////////////////////////////////////
// max number of sample points for vert section
// Default is 500.

void setReadMaxVsectSamples(int max_samples);
void clearReadMaxVsectSamples();

/////////////////////////////////////////////
// Read vert section as RHI
// Compute azimuth from radar to mid-point of vsection.
// If error is within limits, return RHI. Otherwise
// return error.

void setReadVsectAsRhi(bool as_polar = true,
		       double max_az_error = 2.0,
		       bool respectUserDistance = false );

void clearReadVsectAsRhi();

///////////////////////////////////////////////
// set way points for reading vertical section

void addReadWayPt(const double lat, const double lon);
void clearReadWayPts();

///////////////////////////////////////////////////////////
// Disable interpolation for vertical section
// If interp is disabled, nearest neighbor sampling is used.

void setReadVsectDisableInterp();
void clearReadVsectDisableInterp();

/////////////////////////////////////////////
// remapping grid on read.
// setReadRemap() returns 0 on success, -1 on failure.

// Radar FLAT (Azimuthal Equidistant)

void setReadRemapFlat(int nx, int ny,
		      double minx, double miny,
		      double dx, double dy,
		      double origin_lat, double origin_lon,
		      double rotation);

void setReadRemapAzimEquidist(int nx, int ny,
			      double minx, double miny,
			      double dx, double dy,
			      double origin_lat, double origin_lon,
			      double rotation) {
  setReadRemapFlat(nx, ny, minx, miny, dx, dy,
		   origin_lat, origin_lon, rotation);
}

// latlon (cylindrical equidistant)

void setReadRemapLatlon(int nx, int ny,
			double minx, double miny,
			double dx, double dy);

// Lambert Conformal conic

void setReadRemapLambertConf(int nx, int ny,
			     double minx, double miny,
			     double dx, double dy,
			     double origin_lat, double origin_lon,
			     double lat1, double lat2);

void setReadRemapLc2(int nx, int ny,
		     double minx, double miny,
		     double dx, double dy,
		     double origin_lat, double origin_lon,
		     double lat1, double lat2) {
  setReadRemapLambertConf(nx, ny, minx, miny, dx, dy,
			  origin_lat, origin_lon, lat1, lat2);
}

// Stereographic - polar aspect

void setReadRemapPolarStereo(int nx, int ny,
			     double minx, double miny,
			     double dx, double dy,
			     double origin_lat, double origin_lon,
			     double tangent_lon, 
			     Mdvx::pole_type_t poleType,
			     double central_scale,
			     double lad = 90.0);

// Stereographic - oblique aspect

void setReadRemapObliqueStereo(int nx, int ny,
			       double minx, double miny,
			       double dx, double dy,
			       double origin_lat, double origin_lon,
			       double tangent_lat, double tangent_lon,
                               double central_scale);

// Mercator

void setReadRemapMercator(int nx, int ny,
			  double minx, double miny,
			  double dx, double dy,
			  double origin_lat, double origin_lon);

// Transverse mercator

void setReadRemapTransverseMercator(int nx, int ny,
				    double minx, double miny,
				    double dx, double dy,
				    double origin_lat, double origin_lon,
				    double central_scale);

// Albers equal area conic

void setReadRemapAlbers(int nx, int ny,
			double minx, double miny,
			double dx, double dy,
			double origin_lat, double origin_lon,
			double lat1, double lat2);

// Lambert azimuthal equal area
  
void setReadRemapLambertAzimuthal(int nx, int ny,
				  double minx, double miny,
				  double dx, double dy,
				  double origin_lat, double origin_lon);

// Vertical perspective

void setReadRemapVertPersp(int nx, int ny,
                           double minx, double miny,
                           double dx, double dy,
                           double origin_lat, double origin_lon,
                           double persp_radius);

// set false coord correction for remapping
// Normally these are not set.
// The false values are added to the real coords
// to keep the values positive.

void setReadFalseCoords(double false_northing,
                        double false_easting);

// set remapping from projection object
 
int setReadRemap(const MdvxProj &projection);
int setReadRemap(const MdvxPjg &projection);

void clearReadRemap();

/////////////////////////////////////////////
// Auto remap to lat-lon
//
// Selects grid appropriately from data

void setReadAutoRemap2LatLon();
void clearReadAutoRemap2LatLon();

/////////////////////////////////////////////
// decimation on read
// Set the maximum number of grid points to be 
// returned from a read.

void setReadDecimate(size_t max_nxy);
void clearReadDecimate();

/////////////////////////////////////////////
// vlevels from read
// Set the desired vlevel type to be returned from the read.
// If the request does not make sense, it will be ignored.
//
// Conversion between the following are supported:
//
//    VERT_TYPE_Z
//    VERT_TYPE_PRESSURE
//    VERT_FLIGHT_LEVEL

void setReadVlevelType(vlevel_type_t vlevel_type);
void clearReadVlevelType();

/////////////////////////////
// Returning FieldFileHeaders
//
// If this option is set, the field objects will contain 
// copies of the actual file headers, in addition to the
// headers modified on the read.

void setReadFieldFileHeaders();
void clearReadFieldFileHeaders();

/////////////////////////////////////////////
// Read time list when reading volume etc.
// You must call setTimeListModeXxxx() as well.
// Time list data will be returned.
// Use compileTimeList() if only time list is required.

void setReadTimeListAlso();
void clearReadTimeListAlso();

/////////////////////////////////////////////////
// Set latest valid file mod time.
// Option to check file modify times and only use
// files with mod times before a given time.

void setCheckLatestValidModTime(time_t latest_valid_mod_time);
void clearCheckLatestValidModTime();

// set/clear readAsSinglePart option
//
// If this is set when a request is via a server, the server passes the data
// back as a single buffer part instead of multiple parts.
// 
// The buffer contains an Mdvx object just as it would be written to a file.
// 
// NOTE: this is only applicable if the request is made to a server.
// If the read is local, this option has no effect.
// If you are doing a local read and need a buffer representation,
// see writeToBuffer() in Mdvx_write.hh.

void setReadAsSingleBuffer();
void clearReadAsSingleBuffer();

// read using 32-bit headers

void setRead32BitHeaders(bool val) { _read32BitHeaders = val; }

// deprecated

void setReadAsSinglePart();
void clearReadAsSinglePart();

// set/clear read format option
//
// (a) XML
//
// If the request is local, fill out the XML headers and
// the XML data buffer
//
// If request is via a server, the server passes back the
// xml header and buffer, which are set in the object
//
// See _xmlHdr and _xmlBuf;
//
// (b) netCDF
//
// If the request is local, fill out the netcdf buffer
// and the times in the master header
//
// If request is via a server, the server passes the data
// back as a netcdf buffer

void setReadFormat(mdv_format_t read_format);
void clearReadFormat(); // resets to normal

//////////////////////
// print read request

virtual void printReadRequest(ostream &out);

////////////////////////////////////////////////////////////////////
// verify()
//
// Verify that a file is in MDV format, by checking the magic cookie
//
// Returns true or false
//
 
bool verify(const string &file_path);

////////////////////////////////////////////////////////////////////
// checkIs64bit()
//
// Check if an MDV file is a 64-bit version
//
// Returns 0 on success, -1 on failure
// Sets the _is64Bit flag appropriately.
// Use getIs64Bit() after this call.
 
int checkIs64Bit(const string &file_path);

//////////////////////////////////////////////////////////////////////
// Read all of the headers in a file. The headers are read in exaclty
// as they exist in the file.
//
// You must call either setReadTime() or setReadPath() before calling
// this function.
//
// returns 0 on success, -1 on failure
//
// If successful, you can access the headers using:
//
//  getMasterHeaderFile()
//  getFieldHeaderFile()
//  getVlevelHeaderFile()
//  getChunkHeaderFile()

virtual int readAllHeaders();

////////////////////////////////////////////////////////////////////
// Read volume, according to the read settings.
//
// To specify the file path, you must either
//   setReadTime() or
//   setReadPath().
//
// The following calls are optional prior to calling read():
//
//   addReadField(): the specified fields are read.
//                   If not called, all fields are read.
//
//   addReadChunk(): the specified chunks are read.
//                   If not called, all chunks are read.
//
//   setReadHorizLimits(): horizontal limits apply.
//
//   setReadVlevelLimits(): vlevel limits apply.
//     or
//   setReadPlaneNumLimits(): plane num limits apply.
//                       
//   setReadEncodingType(): specify encoding type
//                          default is ENCODING_ASIS
//
//   setReadCompressionType(): specify compression type
//                             default is COMPRESSION_ASIS
//
//   setReadScalingType(): specify scaling type for data conversions
//                         default is SCALING_ROUNDED
//
//   setReadComposite(): compute the composite.
//                       This uses vlevel or plane num limits if set.
//
// returns 0 on success, -1 on failure

virtual int readVolume();

//////////////////////////////////////////////////////////
// Read vertical section, according to the read settings.
//
// nsamples is the number of points to be sampled along the
// vertical section.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

virtual int readVsection();

///////////////////////////////////////////////////////////
// Read from buffer
//
// Read an Mdvx object from a buffer as if it were a file.
//
// Returns 0 on success, -1 on failure

int readFromBuffer(const MemBuf &buf);

///////////////////////////////////////////
// Read using the buffer routines.
//
// This is intended for testing only.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int readUsingBuf();

///////////////////////////////////////////////////////////
// Set the weight given to valid time differences, over gen
// time differences, when searching for the best forecast
// for a specified time
//
// Finding the best forecast for a given time is tricky. Do
// you care more about differences between the valid time
// and the requested time, or do you want to give more weight
// to the closest gen time.
//
// The default value is 2.5. This works well for most situations.
// 
// If the time between model runs is long (say 6 hours) as compared
// to the time between model output times (say 30 mins) then you
// need to increase the weight to say 25. Setting it to 100
// will weight the decision very heavily in favor of the diff
// between the valid and requested time, and put very little
// weight on which model run to use.

void setValidTimeSearchWt(double wt);
void clearValidTimeSearchWt();
double getValidTimeSearchWt() const;

//////////////////////////////////////////////////
// test whether read qualifiers are active
// Returns true if qualifiers set, false otherwise

bool readQualifiersAreActive() { return _readQualifiersActive; }

////////////////////////////////////////////////////////////////
// for each field, constrain the data in the vertical,
// based on read limits set the mdvx object

void constrainVertical();

////////////////////////////////////////////////////////////////
// for each field, constrain the data in the horizontal
// based on read limits set the mdvx object

void constrainHorizontal();


#endif

    
