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

///////////////////////////////////////
// Mdv/DsMdvxMsg.hh
//
// DsMessage class for DsMdvx class
///////////////////////////////////////

#ifndef DsMdvxMsg_hh
#define DsMdvxMsg_hh

#include <didss/DsDataFile.hh>
#include <dsserver/DsServerMsg.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

class DsMdvxMsg : public DsServerMsg {

  friend class DsMdvx;

public:
  
  typedef enum {
    MDVP_REQUEST_MESSAGE    = 10000,
    MDVP_REPLY_MESSAGE      = 20000
  } msg_type_t;
  
  typedef enum {
    MDVP_READ_ALL_HDRS         = 27100,
    MDVP_READ_VOLUME           = 27110,
    MDVP_READ_VSECTION         = 27120,
    MDVP_WRITE_TO_DIR          = 27130,
    MDVP_WRITE_TO_PATH         = 27140,
    MDVP_COMPILE_TIME_LIST     = 27150,
    MDVP_COMPILE_TIME_HEIGHT   = 27160,
    MDVP_CONVERT_MDV_TO_NCF    = 27170
  } msg_subtype_t;

  // the following are deprecated - will be removed later
  // keep in here for now, so no other enum uses these vals
 
  typedef enum {
    MDVP_CONVERT_NCF_TO_MDV    = 27180,
    MDVP_READ_NCF              = 27200,
    MDVP_READ_RADX             = 27210,
    MDVP_READ_ALL_HDRS_NCF     = 27220,
    MDVP_READ_ALL_HDRS_RADX    = 27230
  } msg_subtype_deprecated_t;

  typedef enum {
    MDVP_INCOMPLETE_REQUEST  = 30000,
    MDVP_NO_DATA             = 30001,
    MDVP_GET_ERROR           = 30002,
    MDVP_PUT_ERROR           = 30004
  } msg_flags_t;

  typedef enum {

    MDVP_READ_URL_PART                   = DsServerMsg::DS_URL,          // 1
    MDVP_ERR_STRING_PART                 = DsServerMsg::DS_ERR_STRING,   // 8
    MDVP_CLIENT_USER_PART                = DsServerMsg::DS_CLIENT_USER,  // 64
    MDVP_CLIENT_HOST_PART                = DsServerMsg::DS_CLIENT_HOST,  // 128
    MDVP_CLIENT_IPADDR_PART              = DsServerMsg::DS_CLIENT_IPADDR,// 256

    MDVP_FILE_SEARCH_PART_32             = 50100,
    MDVP_APP_NAME_PART                   = 50105,

    // current, read and write formats

    MDVP_READ_FORMAT_PART                = 50106,
    MDVP_WRITE_FORMAT_PART               = 50107,
    MDVP_INTERNAL_FORMAT_PART            = 50108,

    // read specs

    MDVP_READ_FIELD_NUM_PART             = 50110,
    MDVP_READ_FIELD_NAME_PART            = 50120,
    MDVP_READ_CHUNK_NUM_PART             = 50130,
    MDVP_READ_HORIZ_LIMITS_PART          = 50140,
    MDVP_READ_VLEVEL_LIMITS_PART         = 50150,
    MDVP_READ_PLANE_NUM_LIMITS_PART      = 50160,
    MDVP_READ_COMPOSITE_PART             = 50170,
    MDVP_READ_FILL_MISSING_PART          = 50175,
    MDVP_READ_ENCODING_PART              = 50180,
    MDVP_READ_REMAP_PART_32              = 50190,
    MDVP_READ_AUTO_REMAP_TO_LATLON_PART  = 50191,
    MDVP_READ_FIELD_FILE_HEADERS_PART    = 50195,
    MDVP_READ_VSECT_WAYPTS_PART_32       = 50200,
    MDVP_READ_VSECT_NSAMPLES_PART        = 50201,
    MDVP_READ_VSECT_MAXSAMPLES_PART      = 50202,
    MDVP_READ_VSECT_DISABLE_INTERP_PART  = 50203,
    MDVP_READ_VSECT_AS_RHI_PART          = 50204,
    MDVP_READ_AS_SINGLE_BUFFER_PART      = 50210,

    // deprecated - keep in here for now, so no other
    // parts use this enum

    MDVP_DEPRECATED_01_PART              = 50211,

    // write specs

    MDVP_WRITE_OPTIONS_PART              = 50300,

    // time list specs

    MDVP_TIME_LIST_OPTIONS_PART_32       = 50400,

    // headers

    MDVP_MASTER_HEADER_PART_32              = 50500,
    MDVP_MASTER_HEADER_FILE_PART_32         = 50501,
    MDVP_FIELD_HEADER_PART_32               = 50510,
    MDVP_FIELD_HEADER_FILE_PART_32          = 50511,
    MDVP_FIELD_HEADER_FILE_FIELD_PART_32    = 50512,
    MDVP_VLEVEL_HEADER_PART_32              = 50520,
    MDVP_VLEVEL_HEADER_FILE_PART_32         = 50521,
    MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32   = 50522,
    MDVP_CHUNK_HEADER_PART_32               = 50530,
    MDVP_CHUNK_HEADER_FILE_PART_32          = 50531,

    MDVP_FIELD_DATA_PART                 = 50610,
    MDVP_CHUNK_DATA_PART                 = 50620,

    // vertical sections

    MDVP_VSECT_SAMPLE_PTS_PART_32        = 50700,
    MDVP_VSECT_SEGMENTS_PART_32          = 50710,

    // times

    MDVP_VALID_TIMES_PART                = 50800,
    MDVP_GEN_TIMES_PART                  = 50810,
    MDVP_FORECAST_TIMES_PART             = 50820,

    // path of file read/written

    MDVP_PATH_IN_USE_PART                = 50900,

    // representation as a single buffer

    MDVP_SINGLE_BUFFER_PART              = 50910,

    // XML format

    MDVP_XML_HEADER_PART                 = 50911,
    MDVP_XML_BUFFER_PART                 = 50912,
    
    // read result

    MDVP_NO_FILES_FOUND_ON_READ_PART     = 50920,

    // read options part 2

    MDVP_READ_DECIMATE_PART              = 50930,
    MDVP_READ_VLEVEL_TYPE_PART           = 50940,
    MDVP_READ_TIME_LIST_ALSO_PART        = 50950,
    MDVP_READ_LATEST_VALID_MOD_TIME_PART = 50960,
    MDVP_CONSTRAIN_LEAD_TIMES_PART       = 50970,

    // NetCDF CF

    MDVP_NCF_HEADER_PART                 = 51000,
    MDVP_NCF_BUFFER_PART                 = 51010,
    MDVP_CONVERT_MDV_TO_NCF_PART         = 51020,

    // Parts used by the DsMdvClimoServer

    MDVP_CLIMO_STATISTIC_TYPE_PART       = 52000,
    MDVP_CLIMO_DATA_RANGE_PART_32        = 52100,
    MDVP_CLIMO_TIME_RANGE_PART           = 52200,

    // 64 bit headers
    
    MDVP_FILE_SEARCH_PART_64             = 53100,
    MDVP_READ_REMAP_PART_64              = 53190,
    MDVP_TIME_LIST_OPTIONS_PART_64       = 53400,
    MDVP_CLIMO_DATA_RANGE_PART_64        = 53410,

    MDVP_MASTER_HEADER_PART_64            = 53500,
    MDVP_MASTER_HEADER_FILE_PART_64       = 53501,
    MDVP_FIELD_HEADER_PART_64             = 53510,
    MDVP_FIELD_HEADER_FILE_PART_64        = 53511,
    MDVP_FIELD_HEADER_FILE_FIELD_PART_64  = 53512,
    MDVP_VLEVEL_HEADER_PART_64            = 53520,
    MDVP_VLEVEL_HEADER_FILE_PART_64       = 53521,
    MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64 = 53522,
    MDVP_CHUNK_HEADER_PART_64             = 53530,
    MDVP_CHUNK_HEADER_FILE_PART_64        = 53531,

    MDVP_READ_VSECT_WAYPTS_PART_64       = 54200,
    MDVP_VSECT_SAMPLE_PTS_PART_64        = 54700,
    MDVP_VSECT_SEGMENTS_PART_64          = 54710

  } msg_part_t;

  typedef enum {
    MDVP_READ_FROM_PATH =           -1, // special case
    MDVP_READ_LAST =                Mdvx::READ_LAST,               // 0
    MDVP_READ_CLOSEST =             Mdvx::READ_CLOSEST,            // 1
    MDVP_READ_FIRST_BEFORE =        Mdvx::READ_FIRST_BEFORE,       // 2
    MDVP_READ_FIRST_AFTER =         Mdvx::READ_FIRST_AFTER,        // 3
    MDVP_READ_BEST_FORECAST =       Mdvx::READ_BEST_FORECAST,      // 4
    MDVP_READ_SPECIFIED_FORECAST =  Mdvx::READ_SPECIFIED_FORECAST  // 5
  } file_search_mode_t;

  typedef struct {
    si32 file_search_mode;
    si32 search_margin_secs;
    ti32 search_time;
    si32 forecast_lead_secs;
    fl32 valid_time_search_wt;
    si32 spare;
  } file_search_32_t;
   
  typedef struct {
    si64 file_search_mode;
    si64 search_margin_secs;
    si64 search_time;
    si64 forecast_lead_secs;
    fl64 valid_time_search_wt;
    si64 spare[3];
  } file_search_64_t;

  typedef file_search_64_t file_search_t;
  
  typedef struct {
    si32 min_lead_time;
    si32 max_lead_time;
    ti32 specify_by_gen_time;
    si32 spare[5];
  } constrain_lead_times_t;
   
  typedef struct {
    fl32 min_lat;
    fl32 min_lon;
    fl32 max_lat;
    fl32 max_lon;
    si32 spare[2];
  } read_horiz_limits_t;

  typedef struct {
    fl32 min_vlevel;
    fl32 max_vlevel;
    si32 spare[2];
  } read_vlevel_limits_t;

  typedef struct {
    si32 min_plane_num;
    si32 max_plane_num;
    si32 spare[2];
  } read_plane_num_limits_t;

  typedef enum {
    MDVP_COMPOSITE_NONE = -1,
    MDVP_COMPOSITE_MAX = 1
  } composite_method_t;

  typedef struct {
    si32 type;
    si32 spare;
  } read_composite_t;

  typedef struct {
    si32 encoding_type;
    si32 compression_type;
    si32 scaling_type;
    fl32 scale;
    fl32 bias;
    si32 spare;
  } read_encoding_t;

  // Note: for remap_t:
  // See Mdvx_typedefs for proj_params members
  // which match those in field_header_t.

  typedef struct {
    si32 proj_type;
    si32 nx;
    si32 ny;
    si32 spare_si32[3];
    fl32 minx;
    fl32 miny;
    fl32 dx;
    fl32 dy;
    fl32 origin_lat;
    fl32 origin_lon;
    fl32 proj_params[MDV32_MAX_PROJ_PARAMS];
    fl32 spare_fl32[6];
  } read_remap_32_t;

  typedef struct {
    si64 proj_type;
    si64 nx;
    si64 ny;
    si64 spare_si64[5];
    fl64 minx;
    fl64 miny;
    fl64 dx;
    fl64 dy;
    fl64 origin_lat;
    fl64 origin_lon;
    fl64 proj_params[MDV64_MAX_PROJ_PARAMS];
    fl64 spare_fl64[6];
  } read_remap_64_t;

  typedef read_remap_64_t read_remap_t;
  
  typedef struct {
    si32 write_as_forecast; // forces forecast write
    si32 write_ldata_info;
    si32 write_using_extended_path;
    si32 if_forecast_write_as_forecast; /* writes as forecast only if
                                         * data_collection_type is
                                         * FORECAST or EXTRAPOLATED */
  } write_options_t;
  
  typedef struct {
    si32 mode;
    si32 start_time;
    si32 end_time;
    si32 gen_time;
    si32 search_time;
    si32 time_margin;
  } time_list_options_32_t;
  
  typedef struct {
    si64 mode;
    si64 start_time;
    si64 end_time;
    si64 gen_time;
    si64 search_time;
    si64 time_margin;
    si64 spare[4];
  } time_list_options_64_t;

  typedef time_list_options_64_t time_list_options_t;
  
  typedef struct {
    si32 ntimes;
    si32 has_forecasts;
    si32 spare[2];
  } time_list_hdr_t;
  
  typedef struct {
    si32 as_polar;
    fl32 max_az_error;
    si32 respect_user_dist;
    si32 spare;
  } rhi_request_t;

  // NOTE: for vertical section parts, see the chunkVsect

  // typedefs in Mdvx_typedefs.hh

  // Structures used for climatology message parts.  The climatology
  // message parts are used by the DsMdvClimoServer to derived
  // climatology information from either raw datasets or from partially
  // precalculated datasets.  The climoDataRange part allows you to
  // specify the range of data to be used in the derived climatology,
  // while the climoTimeRange part allows you to filter the data even
  // more so that only data between the given times of day are used.
  //
  // For example, if you want the average precipitation for the month
  // of June of 2003 between the hours of 15Z and 18Z, you would set
  // the climoDataRange to 6-1-2003 0:00:00 and 7/1/2003 0:00:00 and
  // the climoTimeRange to 150000 and 180000.
  //
  // Note that these queries are meant to be calculated on the fly and
  // so could take a while to satisfy, depending on the amount of data
  // being processed for the query.

  typedef struct
  {
    si32 num_stats;
    si32 spare[3];
  } climoTypePartHdr_t;
  
  
  typedef struct 
  {
    si32 climo_type;          // One of climo_type_t
    si32 divide_by_num_obs;   // 1 = true, 0 = false
    fl32 params[2];           // Parameters specific to the type of
                              // climo requested. Used as follows:
                              //    CLIMO_TYPE_NUM_OBS_GT:
                              //    CLIMO_TYPE_NUM_OBS_GE:
                              //    CLIMO_TYPE_NUM_OBS_LT:
                              //    CLIMO_TYPE_NUM_OBS_LE:
                              //           params[0] = cutoff value
  } climoTypePart_t;
    
  typedef struct {
    ti32 start_time;     // Start time for climo request
    ti32 end_time;       // End time for climo request
    si32 spare[2];
  } climoDataRange_32_t;
  
  typedef struct {
    si64 start_time;     // Start time for climo request
    si64 end_time;       // End time for climo request
    si64 spare[4];
  } climoDataRange_64_t;

  typedef climoDataRange_64_t climoDataRange_t;
  
  typedef struct {
    si32 start_hour;     // Start TOD for climo request
    si32 start_minute;
    si32 start_second;
    si32 end_hour;       // End TOD for climo request
    si32 end_minute;
    si32 end_second;
    si32 spare[2];
  } climoTimeRange_t;
  
  DsMdvxMsg(memModel_t mem_model = CopyMem);

  virtual ~DsMdvxMsg();

  // Assemble readAllHeaders message.
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error string.
  
  void *assembleReadAllHdrs(const DsMdvx &mdvx);

  // assemble readAllHeadersReturn message
  // Returns assembled message.

  void *assembleReadAllHdrsReturn(const DsMdvx &mdvx);
  
  // Assemble readVolume message.
  // If _readAsSinglePart is true, the mdvx data will be returned
  // as a buffer in a single part rather than in separate parts.
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error string.
  
  void *assembleReadVolume(const DsMdvx &mdvx);
  
  // assemble readVolumeReturn message
  // Returns assembled message.
  
  void *assembleReadVolumeReturn(const DsMdvx &mdvx);
  
  // assemble readVsection message
  // If _readAsSinglePart is true, the mdvx data will be returned
  // as a buffer in a single part rather than in separate parts.
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error string.

  void *assembleReadVsection(const DsMdvx &mdvx);
  
  // assemble readVsectionReturn message
  // Returns assembled message.
  
  void *assembleReadVsectionReturn(const DsMdvx &mdvx);
  
  // assemble write message
  // Returns assembled message.
  
  void *assembleWrite(msg_subtype_t subtype,
		      const DsMdvx &mdvx,
		      const string &output_url);
  
  // assemble write return message
  // Returns assembled message.
  
  void *assembleWriteReturn(msg_subtype_t subtype,
			    const DsMdvx &mdvx);
  
  // Assemble compileTimeList message.
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error string.
  
  void *assembleCompileTimeList(const DsMdvx &mdvx);
  
  // assemble compileTimeListReturn message
  // Returns assembled message.

  void *assembleCompileTimeListReturn(const DsMdvx &mdvx);
  
  // assemble compileTimeHeight message
  //
  // If _readAsSinglePart is true, the mdvx data will be returned
  // as a buffer in a single part rather than in separate parts.
  //
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error str for this call.
  
  void *assembleCompileTimeHeight(const DsMdvx &mdvx);

  // assemble compileTimeHeightReturn message
  // Returns assembled message.
  
  void *assembleCompileTimeHeightReturn(const DsMdvx &mdvx);

  // assemble an error return message

  void *assembleErrorReturn(const int requestSubtype,
			    const string &errorStr,
			    bool noFilesFoundOnRead = false);
  
  // override the disassemble function
  // returns 0 on success, -1 on error

  int disassemble(const void *in_msg, const ssize_t msg_len, DsMdvx &mdvx);

  // check if message uses 32-bit headers

  bool getUse32BitHeaders() const { return _use32BitHeaders; }

  // get error string

  string &getErrStr() { return (_errStr); }

  // prints

  virtual void print(ostream &out, const char *spacer) const;
  virtual void printHeader(ostream &out, const char *spacer) const;

  // check parts for consistency - 32-bit or 64-bit?
  // sets _use32BitHeaders appropriately
  
  void check64BitHeaders() const;

  // get the label for a part id

  string getLabelForPart(int id) const;

protected:

  string _errStr;

  mutable bool _use32BitHeaders;  // using 32-bit headers

  PartHeaderLabelMap _partIdLabels;
  void _loadPartIdLabelMap();

  void _clearErrStr() { _errStr = ""; }
  
  void _printReturnHeaderType(ostream &out) const;
  
  void _addHdrsAndData(const DsMdvx &mdvx);
  void _addHdrsAndDataExtended(const DsMdvx &mdvx);

  void _addReadFormat(Mdvx::mdv_format_t read_format);
  void _addWriteFormat(Mdvx::mdv_format_t write_format);
  void _addInternalFormat(Mdvx::mdv_format_t current_format);

  int _addReadSearch(const DsMdvx &mdvx);
  void _addReadQualifiers(const DsMdvx &mdvx);
  void _addReadVsectQualifiers(const DsMdvx &mdvx);
  void _addReturnVsectInfo(const DsMdvx &mdvx);

  void _addReadFieldNum(int field_num);
  void _addReadFieldName(const string & field_name);
  void _addReadChunkNum(int chunk_num);
  
  void _addReadHorizLimits(double min_lat, double min_lon,
			   double max_lat, double max_lon);

  void _addReadVlevelLimits(double min_vlevel, double max_vlevel);
  void _addReadPlaneNumLimits(int min_plane_num, int max_plane_num);
  
  void _addReadComposite(composite_method_t type);

  void _addReadFillMissing();

  void _addReadEncoding(int encoding_type = Mdvx::ENCODING_ASIS,
			int compression_type = Mdvx::COMPRESSION_NONE,
			int scaling_type = Mdvx::SCALING_ROUNDED,
			double scale = 1.0,
			double bias = 0.0);

  void _addReadRemap(const Mdvx::coord_t &coords);
  void _addReadAutoRemap2LatLon();

  void _addReadFieldFileHeaders();

  void _addReadVsectWayPts(const vector<Mdvx::vsect_waypt_t> &waypts);
  void _addReadNVsectSamples(const int n_samples);
  void _addReadMaxVsectSamples(const int max_samples);
  void _addReadVsectAsRhi(bool as_polar, double max_az_error, bool respect_user_dist);

  void _addReadAsSingleBuffer();

  void _addReadDecimate(const int max_nxy);

  void _addReadVlevelType(Mdvx::vlevel_type_t vlevel_type);

  void _addReadTimeListAlso();

  void _addReadLatestValidModTime(time_t latest_valid_mod_time);

  void _addWriteOptions(bool write_as_forecast,
			bool write_ldata_info,
                        bool write_using_extended_path,
			bool if_forecast_write_as_forecast);

  void _addTimeListOptions(Mdvx::time_list_mode_t mode,
			   const string &urlStr,
			   time_t start_time,
			   time_t end_time,
			   time_t gen_time,
			   time_t search_time,
			   int time_margin);

  void _addConstrainLeadTimes(bool constrain_lead_times,
			      int min_lead_time,
			      int max_lead_time,
			      bool specify_by_gen_time);

  void _addMasterHeader64(const Mdvx::master_header_t & header, int part_id);
  void _addFieldHeader64(const Mdvx::field_header_t & header, int part_id);
  void _addVlevelHeader64(const Mdvx::vlevel_header_t &header, int part_id,
                          int nz = 0, const char *field_name = "unknown");
  void _addChunkHeader64(const Mdvx::chunk_header_t &header, int part_id);

  void _addMasterHeader32(const Mdvx::master_header_t & header, int part_id);
  void _addFieldHeader32(const Mdvx::field_header_t & header, int part_id);
  void _addVlevelHeader32(const Mdvx::vlevel_header_t &header, int part_id,
                          int nz = 0, const char *field_name = "unknown");
  void _addChunkHeader32(const Mdvx::chunk_header_t &header, int part_id);

  void _addFieldData(const MdvxField &field);
  void _addFieldData64(const MdvxField &field);
  void _addChunkData(const MdvxChunk& chunk);
  
  void _addVsectSamplePts(const vector<Mdvx::vsect_samplept_t> &samplePts,
			  double dx_km);
  void _addVsectSegments(const vector<Mdvx::vsect_segment_t> &segments,
			 double totalLength);

  void _addReadVsectInterpDisabled();

  void _addTimeLists(const DsMdvx &mdvx);
  void _addValidTimes(const vector<time_t> &valid_times, bool has_forecasts);
  void _addGenTimes(const vector<time_t> &gen_times, bool has_forecasts);
  void _addForecastTimes(const vector<vector<time_t> >
			 &forecast_times_array);
  void _loadTimeListMemBuf(const vector<time_t> &time_list,
			   bool hasForecasts, MemBuf &tbuf);

  void _addPathInUse(const string &path_in_use);

  void _addSingleBuffer(const MemBuf &buf);
  
  void _addXmlHeader(const string &xml);
  void _addXmlBuffer(const MemBuf &buf);

  void _addConvertMdv2NcfPart(const DsMdvx &mdvx);
  
  void _addNcfHdr(const DsMdvx &mdvx);
  void _addNcfHdrAndData(const DsMdvx &mdvx);
  
  void _addNoFilesFoundOnRead();

  void _addClimoStatTypes(const vector< DsMdvx::climo_stat_t > type_list);
  
  void _addClimoDataRange(const time_t start_time, const time_t end_time);

  void _addClimoTimeRange(const int start_hour, const int start_min,
			  const int start_sec,
			  const int end_hour, const int end_min,
			  const int end_sec);
  
  void _addAppName(const string &app_name);
  
  // get methods - get part from message, load into object
  
  int _getReadFormat(DsMdvx &mdvx);
  int _getWriteFormat(DsMdvx &mdvx);
  int _getInternalFormat(DsMdvx &mdvx);

  int _getReadSearch(DsMdvx &mdvx);
  int _getReadQualifiers(DsMdvx &mdvx);
  int _getReadVsectQualifiers(DsMdvx &mdvx);
  int _getReturnVsectInfo(DsMdvx &mdvx);
  int _getReadHorizLimits(DsMdvx &mdvx);
  int _getReadVlevelLimits(DsMdvx &mdvx);
  int _getReadPlaneNumLimits(DsMdvx &mdvx);
  int _getReadComposite(DsMdvx &mdvx);
  int _getReadEncoding(DsMdvx &mdvx);
  int _getReadRemap(DsMdvx &mdvx);
  int _getReadRemap32(DsMdvx &mdvx);
  int _getReadAutoRemap2LatLon(DsMdvx &mdvx);
  int _getReadDecimate(DsMdvx &mdvx);
  int _getReadTimeListAlso(DsMdvx &mdvx);
  int _getReadLatestValidModTime(DsMdvx &mdvx);
  int _getReadVlevelType(DsMdvx &mdvx);
  int _getReadVsectWaypts(DsMdvx &mdvx);
  int _getReadNVsectSamples(DsMdvx &mdvx);
  int _getReadMaxVsectSamples(DsMdvx &mdvx);
  int _getReadVsectAsRhi(DsMdvx &mdvx);
  int _getReadVsectInterpDisabled(DsMdvx &mdvx);

  int _getWriteOptions(DsMdvx &mdvx);
  int _getWriteUrl(DsMdvx &mdvx);
  int _getTimeListOptions(DsMdvx &mdvx);
  int _getConstrainLeadTimes(DsMdvx &mdvx);

  int _getMasterHeader(Mdvx::master_header_t &mhdr,
		       int part_id);
  int _getFieldHeader(Mdvx::field_header_t &fhdr,
		      int field_num, int part_id);
  int _getVlevelHeader(Mdvx::vlevel_header_t &vhdr,
		       int field_num, int part_id);
  int _getChunkHeader(Mdvx::chunk_header_t &chdr,
		      int chunk_num, int part_id);

  int _getHeaders(DsMdvx &mdvx);
  int _getHeadersAndData(DsMdvx &mdvx);

  int _getPathInUse(DsMdvx &mdvx);
  int _getSingleBuffer(DsMdvx &mdvx);
  int _getXmlHdrAndBuf(DsMdvx &mdvx);
  void _getNoFilesFoundOnRead(DsMdvx &mdvx);

  int _getField(Mdvx &mdvx, int field_num);
  int _getChunk(Mdvx &mdvx, int chunk_num);

  int _getVsectSegments(DsMdvx &mdvx);
  int _getVsectSamplepts(DsMdvx &mdvx);

  int _getTimeLists(DsMdvx &mdvx);
  int _getValidTimes(DsMdvx &mdvx);
  int _getGenTimes(DsMdvx &mdvx);
  int _getForecastTimes(DsMdvx &mdvx);
  int _loadTimeList(DsMdvx &mdvx,
		    msg_part_t partId,
		    ssize_t partIndex,
		    bool &has_forecasts,
		    vector<time_t> &timeList);
  
  int _getClimoQualifiers(DsMdvx &mdvx);
  int _getClimoStatTypes(DsMdvx &mdvx);
  int _getClimoDataRange(DsMdvx &mdvx);
  int _getClimoTimeRange(DsMdvx &mdvx);
  
  int _getAppName(DsMdvx &mdvx);

  int _getNcfHeaderParts(DsMdvx &mdvx);
  int _getNcfParts(DsMdvx &mdvx);
  int _getConvertMdv2Ncf(DsMdvx &mdvx);

  // disassemble methods

  int _disassembleReadAllHdrs(DsMdvx &mdvx);
  int _disassembleReadAllHdrsReturn(DsMdvx &mdvx);

  int _disassembleReadVolume(DsMdvx &mdvx);
  int _disassembleReadVolumeReturn(DsMdvx &mdvx);

  int _disassembleReadVsection(DsMdvx &mdvx);
  int _disassembleReadVsectionReturn(DsMdvx &mdvx);

  int _disassembleWrite(DsMdvx &mdvx);
  int _disassembleWriteReturn(DsMdvx &mdvx);

  int _disassembleCompileTimeList(DsMdvx &mdvx);
  int _disassembleCompileTimeListReturn(DsMdvx &mdvx);

  int _disassembleCompileTimeHeight(DsMdvx &mdvx);
  int _disassembleCompileTimeHeightReturn(DsMdvx &mdvx);

  int _disassembleConvertMdv2Ncf(DsMdvx &mdvx);
  int _disassembleConvertMdv2NcfReturn(DsMdvx &mdvx);

#ifdef NOTNOW
  int _disassembleConvertNcf2Mdv(DsMdvx &mdvx);
  int _disassembleConvertNcf2MdvReturn(DsMdvx &mdvx);

  int _disassembleReadAllHdrsNcf(DsMdvx &mdvx);
  int _disassembleReadAllHdrsNcfReturn(DsMdvx &mdvx);
  
  int _disassembleReadNcf(DsMdvx &mdvx);
  int _disassembleReadNcfReturn(DsMdvx &mdvx);
  
  int _disassembleReadAllHdrsRadx(DsMdvx &mdvx);
  int _disassembleReadAllHdrsRadxReturn(DsMdvx &mdvx);
  
  int _disassembleReadRadx(DsMdvx &mdvx);
  int _disassembleReadRadxReturn(DsMdvx &mdvx);
#endif

  // print methods

  void _print_file_search(file_search_t &fsearch, ostream &out);
  void _print_read_horiz_limits(read_horiz_limits_t &limits, ostream &out);
  void _print_read_vlevel_limits(read_vlevel_limits_t &limits, ostream &out);
  void _print_read_plane_num_limits(read_plane_num_limits_t &limits,
				    ostream &out);
  void _print_read_composite(read_composite_t &comp, ostream &out);
  void _print_read_encoding(read_encoding_t &encod, ostream &out);
  void _print_read_remap(read_remap_t &remap, ostream &out);
  void _print_write_options(write_options_t &options, ostream &out);
  void _print_time_list_options(time_list_options_t &options, ostream &out);
  void _print_time_list(time_list_hdr_t &hdr,
			ti32 *times, ostream &out);
  void _print_climo_stat_type(const climoTypePartHdr_t &hdr,
			      const climoTypePart_t *stats,
			      ostream &out) const;
  void _print_climo_data_range(const climoDataRange_t data_range,
			       ostream &out) const;
  void _print_climo_time_range(const climoTimeRange_t time_range,
			       ostream &out) const;

  // interpret a part as a string
  // make sure it is null terminated etc

  string _part2Str(const DsMsgPart *part);

  // 32-bit to 64-bit headers and vice versa
  
  void _copyFileSearch32to64(const file_search_32_t &fsearch32,
                             file_search_64_t &fsearch64);

  void _copyFileSearch64to32(const file_search_64_t &fsearch64,
                             file_search_32_t &fsearch32);

  void _copyReadRemap32to64(const read_remap_32_t &remap32,
                            read_remap_64_t &remap64);

  void _copyReadRemap64to32(const read_remap_64_t &remap64,
                            read_remap_32_t &remap32);

  void _copyTimeListOptions32to64(const time_list_options_32_t &tlist32,
                                  time_list_options_64_t &tlist64);
  
  void _copyTimeListOptions64to32(const time_list_options_64_t &tlist64,
                                  time_list_options_32_t &tlist32);
  
  void _copyClimoDataRange32to64(const climoDataRange_32_t &drange32,
                                 climoDataRange_64_t &drange64);

  void _copyClimoDataRange64to32(const climoDataRange_64_t &drange64,
                                 climoDataRange_32_t &drange32);

  // check parts

  bool _has32BitParts() const;
  bool _has64BitParts() const;

  // 32 bit headers

  void _setUse32BitHeaders(bool val) const { _use32BitHeaders = val; }

private:

  // Private members with no bodies provided -- do not use until defined.
  DsMdvxMsg(const DsMdvxMsg & orig);
  void operator= (const DsMdvxMsg & other);

};

#endif
