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
///////////////////////////////////////////////////////////////
// msg_add.cc
//
// add methods for DsMdvxMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <Mdv/DsMdvxMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
using namespace std;

/////////////////////////////
// add data formats parts

void DsMdvxMsg::_addReadFormat(Mdvx::mdv_format_t read_format)
  
{

  string formatStr = Mdvx::format2Str(read_format);
  if (_debug) {
    cerr << "Adding read format: " << formatStr << endl;
  }
  addPart(MDVP_READ_FORMAT_PART, formatStr.size() + 1, formatStr.c_str());

}

void DsMdvxMsg::_addWriteFormat(Mdvx::mdv_format_t write_format)
  
{

  string formatStr = Mdvx::format2Str(write_format);
  if (_debug) {
    cerr << "Adding write format: " << formatStr << endl;
  }
  addPart(MDVP_WRITE_FORMAT_PART, formatStr.size() + 1, formatStr.c_str());

}

void DsMdvxMsg::_addInternalFormat(Mdvx::mdv_format_t internal_format)
  
{

  string formatStr = Mdvx::format2Str(internal_format);
  if (_debug) {
    cerr << "Adding internal format: " << formatStr << endl;
  }
  addPart(MDVP_INTERNAL_FORMAT_PART, formatStr.size() + 1, formatStr.c_str());

}

//////////////////////////////////////
// add the read search parameter parts
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_addReadSearch(const DsMdvx &mdvx)

{

  file_search_t fsearch;
  MEM_zero(fsearch);
  
  if (mdvx._readTimeSet) {
    
    fsearch.file_search_mode = mdvx._readSearchMode;
    fsearch.search_margin_secs = mdvx._readSearchMargin;
    fsearch.search_time = mdvx._readSearchTime;
    fsearch.forecast_lead_secs = mdvx._readForecastLeadTime;
    addURL(mdvx._readDirUrl);
    if (_debug) {
      cerr << "Adding URL: " << mdvx._readDirUrl << endl;
    }
    
  } else if (mdvx._readPathSet) {
    
    fsearch.file_search_mode = MDVP_READ_FROM_PATH;
    addURL(mdvx._readPathUrl);
    if (_debug) {
      cerr << "Adding URL: " << mdvx._readPathUrl << endl;
    }
    
  } else {

    TaStr::AddStr(_errStr, "ERROR - DsMdvxMsg::_addReadSearch");
    TaStr::AddStr(_errStr, "  Must set either path or time");
    return -1;

  }

  fsearch.valid_time_search_wt = mdvx.getValidTimeSearchWt();

  if (_debug) {
    _print_file_search(fsearch, cerr);
  }

  // send both 32-bit and 64-bit request parts
  // this is done so that legacy servers will respond correctly
  // in addition to new server

  // 32-bit

  file_search_32_t fsearch32;
  _copyFileSearch64to32(fsearch, fsearch32);
  BE_from_array_32(&fsearch32, sizeof(fsearch32));
  addPart(MDVP_FILE_SEARCH_PART_32, sizeof(fsearch32), &fsearch32);

  // 64-bit

  if (!_use32BitHeaders) {
    BE_from_array_64(&fsearch, sizeof(fsearch));
    addPart(MDVP_FILE_SEARCH_PART_64, sizeof(fsearch), &fsearch);
  }

  // optional part to constrain forecast lead times

  _addConstrainLeadTimes(mdvx.getConstrainFcastLeadTimes(),
			 mdvx.getMinFcastLeadTime(),
			 mdvx.getMaxFcastLeadTime(),
			 mdvx.getSpecifyFcastByGenTime());

  return 0;
  
}
  
/////////////////////////////////////////////////////////
// add the read qualifier parts

void DsMdvxMsg::_addReadQualifiers(const DsMdvx &mdvx)

{

  // field specs
  
  if (mdvx._readFieldNums.size() > 0) {
    for (size_t i = 0; i < mdvx._readFieldNums.size(); i++) {
      _addReadFieldNum(mdvx._readFieldNums[i]);
    }
  } else if (mdvx._readFieldNames.size() > 0) {
    for (size_t i = 0; i < mdvx._readFieldNames.size(); i++) {
      _addReadFieldName(mdvx._readFieldNames[i]);
    }
  }

  // chunk specs

  if (mdvx._readChunkNums.size() > 0) {
    for (size_t i = 0; i < mdvx._readChunkNums.size(); i++) {
      _addReadChunkNum(mdvx._readChunkNums[i]);
    }
  }

  // horizontal limits

  if (mdvx._readHorizLimitsSet) {
    _addReadHorizLimits(mdvx._readMinLat, mdvx._readMinLon,
			mdvx._readMaxLat, mdvx._readMaxLon);
  }

  // vertical limits

  if (mdvx._readVlevelLimitsSet) {
    _addReadVlevelLimits(mdvx._readMinVlevel, mdvx._readMaxVlevel);
  } else if (mdvx._readPlaneNumLimitsSet) {
    _addReadPlaneNumLimits(mdvx._readMinPlaneNum, mdvx._readMaxPlaneNum);
  }

  // composite?

  if (mdvx._readComposite) {
    _addReadComposite(MDVP_COMPOSITE_MAX);
  }

  // fill missing?

  if (mdvx._readFillMissing) {
    _addReadFillMissing();
  }

  // encoding, compression, scaling

  _addReadEncoding(mdvx._readEncodingType,
		   mdvx._readCompressionType,
		   mdvx._readScalingType,
		   mdvx._readScale,
		   mdvx._readBias);

  // remapping

  if (mdvx._readRemapSet) {
    _addReadRemap(mdvx._readRemapCoords);
  }

  if (mdvx._readAutoRemap2LatLon) {
    _addReadAutoRemap2LatLon();
  }

  // read field file headers

  if (mdvx._readFieldFileHeaders) {
    _addReadFieldFileHeaders();
  }

  // get as a single buffer

  if (mdvx._readAsSingleBuffer) {
    _addReadAsSingleBuffer();
  }

  // decimation

  if (mdvx._readDecimate) {
    _addReadDecimate(mdvx._readDecimateMaxNxy);
  }

  if (mdvx._readSpecifyVlevelType) {
    _addReadVlevelType(mdvx._readVlevelType);
  }

  if (mdvx._readVsectAsRhi) {
    _addReadVsectAsRhi(mdvx._readRhiAsPolar, mdvx._readRhiMaxAzError,
                       mdvx._readRhiRespectUserDist);
  }

  // Climatology requests

  if (mdvx._calcClimo) {
    _addClimoStatTypes(mdvx._climoTypeList);

    if (mdvx._climoDataStart != DateTime::NEVER &&
	mdvx._climoDataEnd != DateTime::NEVER)
      _addClimoDataRange(mdvx._climoDataStart.utime(),
			 mdvx._climoDataEnd.utime());

    if (mdvx._climoStartTime.hour >= 0 &&
	mdvx._climoEndTime.hour >= 0)
      _addClimoTimeRange(mdvx._climoStartTime.hour,
			 mdvx._climoStartTime.minute,
			 mdvx._climoStartTime.second,
			 mdvx._climoEndTime.hour,
			 mdvx._climoEndTime.minute,
			 mdvx._climoEndTime.second);
  }

}

/////////////////////////////////////////////////////////
// add the read vsect qualifiers

void DsMdvxMsg::_addReadVsectQualifiers(const DsMdvx &mdvx)
  
{

  // add the waypoint information

  _addReadVsectWayPts(mdvx._vsectWayPts);
  
  if (mdvx._vsectDisableInterp) {
    _addReadVsectInterpDisabled();
  }

  // add n_samples, max_samples if set

  if (mdvx._readNVsectSamples != -1) {
    _addReadNVsectSamples(mdvx._readNVsectSamples);
  }
  if (mdvx._readMaxVsectSamples != DsMdvx::_defaultMaxVsectSamples) {
    _addReadMaxVsectSamples(mdvx._readMaxVsectSamples);
  }

}

/////////////////////////////////////////////////////////
// add the return vsect info

void DsMdvxMsg::_addReturnVsectInfo(const DsMdvx &mdvx)
  
{

  _addReadVsectWayPts(mdvx._vsectWayPts);
  _addVsectSegments(mdvx._vsectSegments, mdvx._vsectTotalLength);
  _addVsectSamplePts(mdvx._vsectSamplePts, mdvx._vsectDxKm);

}

///////////////////////////
// add read field num part

void DsMdvxMsg::_addReadFieldNum(int field_num)
{
  if (_debug) {
    cerr << "Adding field num: " << field_num << endl;
  }
  si32 BEfield_num = BE_from_si32(field_num);
  addPart(MDVP_READ_FIELD_NUM_PART, sizeof(BEfield_num), &BEfield_num);
}

///////////////////////////
// add read field name part

void DsMdvxMsg::_addReadFieldName(const string & field_name)
{
  if (_debug) {
    cerr << "Adding field name: " << field_name << endl;
  }
  addPart(MDVP_READ_FIELD_NAME_PART, field_name.size() + 1,
	  field_name.c_str());
}

//////////////////////////
// add read chunk num part

void DsMdvxMsg::_addReadChunkNum(int chunk_num)
{
  if (_debug) {
    cerr << "Adding chunk num: " << chunk_num << endl;
  }
  si32 BEchunk_num = BE_from_si32(chunk_num);
  addPart(MDVP_READ_CHUNK_NUM_PART, sizeof(BEchunk_num), &BEchunk_num);
}

/////////////////////////////
// add read horiz limits part

void DsMdvxMsg::_addReadHorizLimits(double min_lat, double min_lon,
				    double max_lat, double max_lon)

{
  read_horiz_limits_t limits;
  MEM_zero(limits);
  limits.min_lat = min_lat;
  limits.min_lon = min_lon;
  limits.max_lat = max_lat;
  limits.max_lon = max_lon;
  if (_debug) {
    _print_read_horiz_limits(limits, cerr);
  }
  BE_from_array_32(&limits, sizeof(limits));
  addPart(MDVP_READ_HORIZ_LIMITS_PART, sizeof(limits), &limits);
}

///////////////////////////////
// add read vlevel limits part

void DsMdvxMsg::_addReadVlevelLimits(double min_vlevel, double max_vlevel)

{
  read_vlevel_limits_t limits;
  MEM_zero(limits);
  limits.min_vlevel = min_vlevel;
  limits.max_vlevel = max_vlevel;
  if (_debug) {
    _print_read_vlevel_limits(limits, cerr);
  }
  BE_from_array_32(&limits, sizeof(limits));
  addPart(MDVP_READ_VLEVEL_LIMITS_PART, sizeof(limits), &limits);
}

//////////////////////////////////
// add read plane_num limits part

void DsMdvxMsg::_addReadPlaneNumLimits(int min_plane_num,
				       int max_plane_num)

{
  read_plane_num_limits_t limits;
  MEM_zero(limits);
  limits.min_plane_num = min_plane_num;
  limits.max_plane_num = max_plane_num;
  if (_debug) {
    _print_read_plane_num_limits(limits, cerr);
  }
  BE_from_array_32(&limits, sizeof(limits));
  addPart(MDVP_READ_PLANE_NUM_LIMITS_PART, sizeof(limits), &limits);
}

//////////////////////////
// add read composite part

void DsMdvxMsg::_addReadComposite(composite_method_t type)
{
  read_composite_t comp;
  MEM_zero(comp);
  comp.type = type;
  if (_debug) {
    _print_read_composite(comp, cerr);
  }
  BE_from_array_32(&comp, sizeof(comp));
  addPart(MDVP_READ_COMPOSITE_PART, sizeof(comp), &comp);
}

/////////////////////////////
// add read fill missing part

void DsMdvxMsg::_addReadFillMissing()
{
  // Add a (data-less) part indicating that we want missing data
  // filled in using surrounding points
  if (_debug) {
    cerr << "Adding MDVP_READ_FILL_MISSING_PART" << endl;
  }
  addPart(MDVP_READ_FILL_MISSING_PART, 0, NULL);
}

////////////////////////////////
// add read encoding type part

void DsMdvxMsg::_addReadEncoding(int encoding_type /* = Mdvx::ENCODING_ASIS*/,
				 int compression_type /* = Mdvx::COMPRESSION_NONE*/,
				 int scaling_type /* = Mdvx::SCALING_ROUNDED*/,
				 double scale /* = 1.0*/,
				 double bias /* = 0.0*/ )
  
{
  read_encoding_t encode;
  MEM_zero(encode);
  encode.encoding_type = encoding_type;
  encode.compression_type = compression_type;
  encode.scaling_type = scaling_type;
  encode.scale = scale;
  encode.bias = bias;
  if (_debug) {
    _print_read_encoding(encode, cerr);
  }
  BE_from_array_32(&encode, sizeof(encode));
  addPart(MDVP_READ_ENCODING_PART, sizeof(encode), &encode);
}

////////////////////////
// add remap coords part

void DsMdvxMsg::_addReadRemap(const Mdvx::coord_t &coords)
  
{
 
  read_remap_t remap;

  // projection type

  remap.proj_type = coords.proj_type;

  // grid

  remap.nx = coords.nx;
  remap.ny = coords.ny;
  remap.minx = coords.minx;
  remap.miny = coords.miny;
  remap.dx = coords.dx;
  remap.dy = coords.dy;

  // origin

  remap.origin_lat = coords.proj_origin_lat;
  remap.origin_lon = coords.proj_origin_lon;

  // proj params array

  MdvxProj::_coord2ProjParams((Mdvx::projection_type_t) coords.proj_type,
			      coords, remap.proj_params);

  if (_debug) {
    _print_read_remap(remap, cerr);
  }

  // send both 32-bit and 64-bit request parts
  // this is done so that legacy servers will respond correctly
  // in addition to new server

  // 32-bit

  read_remap_32_t remap32;
  _copyReadRemap64to32(remap, remap32);
  BE_from_array_32(&remap32, sizeof(remap32));
  addPart(MDVP_READ_REMAP_PART_32, sizeof(remap32), &remap32);

  // 64-bit

  if (!_use32BitHeaders) {
    BE_from_array_64(&remap, sizeof(remap));
    addPart(MDVP_READ_REMAP_PART_64, sizeof(remap), &remap);
  }

}

/////////////////////////////////
// add read remap to latlon

void DsMdvxMsg::_addReadAutoRemap2LatLon()
{

  // Add a part indicating that the server should remap
  //   the projection to latlon automatically

  if (_debug) {
    cerr << "Adding MDVP_READ_AUTO_REMAP_TO_LATLON_PART" << endl;
  }
  addPart(MDVP_READ_AUTO_REMAP_TO_LATLON_PART, 0, NULL);
}

///////////////////////////////////
// add read field file headers part

void DsMdvxMsg::_addReadFieldFileHeaders()
{
  // Add a (data-less) part indicating that the server should
  //   return the actual file headers as part of the field
  // 
  if (_debug) {
    cerr << "Adding MDVP_READ_FIELD_FILE_HEADERS_PART" << endl;
  }
  addPart(MDVP_READ_FIELD_FILE_HEADERS_PART, 0, NULL);
}

////////////////////////////////////
// add read vsection waypoints part

void DsMdvxMsg::_addReadVsectWayPts(const vector<Mdvx::vsect_waypt_t> &wayPts)
  
{

  // add 32-bit version

  MemBuf buf32;
  Mdvx::assembleVsectWayPtsBuf32(wayPts, buf32);
  if (_debug) {
    Mdvx::printVsectWayPtsBuf32(buf32, cerr);
  }
  addPart(MDVP_READ_VSECT_WAYPTS_PART_32, buf32.getLen(), buf32.getPtr());

  // add 64-bit version

  if (!_use32BitHeaders) {
    MemBuf buf64;
    Mdvx::assembleVsectWayPtsBuf64(wayPts, buf64);
    if (_debug) {
      Mdvx::printVsectWayPtsBuf64(buf64, cerr);
    }
    addPart(MDVP_READ_VSECT_WAYPTS_PART_64, buf64.getLen(), buf64.getPtr());
  }

}

////////////////////////////////////
// add read vsection n_samples part

void DsMdvxMsg::_addReadNVsectSamples(const int n_samples)
  
{
  si32 nSamples = n_samples;
  BE_from_array_32(&nSamples, sizeof(nSamples));
  addPart(MDVP_READ_VSECT_NSAMPLES_PART, sizeof(nSamples), &nSamples);
}

////////////////////////////////////
// add read vsection max_samples part

void DsMdvxMsg::_addReadMaxVsectSamples(const int max_samples)
  
{
  si32 maxSamples = max_samples;
  BE_from_array_32(&maxSamples, sizeof(maxSamples));
  addPart(MDVP_READ_VSECT_MAXSAMPLES_PART, sizeof(maxSamples), &maxSamples);
  if (_debug) {
    cerr << "Adding MDVP_READ_VSECT_MAXSAMPLES_PART" << endl;
  }
}

///////////////////////////////////
// add read vsect interp disabled

void DsMdvxMsg::_addReadVsectInterpDisabled()
{
  // Add a (data-less) part indicating that the server should
  //   not use interpolation for vert section samples.
  // 
  if (_debug) {
    cerr << "Adding MDVP_READ_VSECT_DISABLE_INTERP_PART" << endl;
  }
  addPart(MDVP_READ_VSECT_DISABLE_INTERP_PART, 0, NULL);
}

/////////////////////////////////
// add read as single buffer part

void DsMdvxMsg::_addReadAsSingleBuffer()
{
  // Add a (data-less) part indicating that the server should
  //   return a complete mdv dataset in a single part instead
  //   of separate parts
  // 
  if (_debug) {
    cerr << "Adding MDVP_READ_AS_SINGLE_BUFFER_PART" << endl;
  }
  addPart(MDVP_READ_AS_SINGLE_BUFFER_PART, 0, NULL);
}

/////////////////////////////////
// add read decimate part

void DsMdvxMsg::_addReadDecimate(const int max_nxy)
{
  // Add a part indicating that the server should decimate
  //   the data to a given max number of xy points

  si32 maxNxy = max_nxy;
  BE_from_array_32(&maxNxy, sizeof(maxNxy));
  addPart(MDVP_READ_DECIMATE_PART, sizeof(maxNxy), &maxNxy);
  if (_debug) {
    cerr << "Adding MDVP_READ_DECIMATE_PART" << endl;
    cerr << "  maxNxy: " << max_nxy << endl;
  }
}

/////////////////////////////////
// add read vlevel type part

void DsMdvxMsg::_addReadVlevelType(Mdvx::vlevel_type_t vlevel_type)
{
  // Add a part indicating that the server should convert
  //   the data onto the specified vlevel

  si32 vlevelType = (si32) vlevel_type;
  BE_from_array_32(&vlevelType, sizeof(vlevelType));
  addPart(MDVP_READ_VLEVEL_TYPE_PART, sizeof(vlevelType), &vlevelType);
  if (_debug) {
    cerr << "Adding MDVP_READ_VLEVEL_TYPE_PART" << endl;
    cerr << "  vlevelType: " << Mdvx::vertType2Str(vlevel_type) << endl;
  }
}

/////////////////////////////////
// add read vsection as RHI

void DsMdvxMsg::_addReadVsectAsRhi(bool as_polar, double max_az_error,
                                   bool respect_user_dist)
{
  // Add a part indicating that the server should read the
  //   vsert section as an RHI

  rhi_request_t req;
  MEM_zero(req);
  req.as_polar = (si32) as_polar;
  req.max_az_error = (fl32) max_az_error;
  req.respect_user_dist = (si32) respect_user_dist;
  BE_from_array_32(&req, sizeof(req));
  addPart(MDVP_READ_VSECT_AS_RHI_PART, sizeof(req), &req);
  if (_debug) {
    cerr << "Adding MDVP_READ_VSECT_AS_RHI_PART" << endl;
    cerr << "  asPolar: " << as_polar << endl;
    cerr << "  maxAzError: " << max_az_error << endl;
    cerr << "  respectUserDist: " << respect_user_dist << endl;
  }
}

/////////////////////////////////
// add read time list also part

void DsMdvxMsg::_addReadTimeListAlso()
{
  // Add a (data-less) part indicating that the time list
  // should be read and returned as well as other data.
  if (_debug) {
    cerr << "Adding MDVP_READ_TIME_LIST_ALSO_PART" << endl;
  }
  addPart(MDVP_READ_TIME_LIST_ALSO_PART, 0, NULL);
}

////////////////////////////////////
// add read check file mod time part

void DsMdvxMsg::_addReadLatestValidModTime(time_t latest_valid_mod_time)
{
  if (_debug) {
    cerr << "Adding latest valid mod time: "
	 << DateTime::str(latest_valid_mod_time, false) << endl;
  }
  ti32 mtime = (ti32) latest_valid_mod_time;
  BE_from_array_32(&mtime, sizeof(mtime));
  addPart(MDVP_READ_LATEST_VALID_MOD_TIME_PART, sizeof(mtime), &mtime);
}

/////////////////////////
// add write options part

void DsMdvxMsg::_addWriteOptions(bool write_as_forecast,
				 bool write_ldata_info,
                                 bool write_using_extended_path,
                                 bool if_forecast_write_as_forecast)

{
  write_options_t options;
  MEM_zero(options);
  options.write_as_forecast = write_as_forecast;
  options.if_forecast_write_as_forecast = if_forecast_write_as_forecast;
  options.write_ldata_info = write_ldata_info;
  options.write_using_extended_path = write_using_extended_path;
  if (_debug) {
    _print_write_options(options, cerr);
  }
  BE_from_array_32(&options, sizeof(options));
  addPart(MDVP_WRITE_OPTIONS_PART, sizeof(options), &options);
}

/////////////////////////////
// add time list options part

void DsMdvxMsg::_addTimeListOptions(Mdvx::time_list_mode_t mode,
				    const string &urlStr,
				    time_t start_time,
				    time_t end_time,
				    time_t gen_time,
				    time_t search_time,
				    int time_margin)
  
{
  addURL(urlStr);
  time_list_options_t options;
  MEM_zero(options);
  options.mode = mode;
  options.start_time = start_time;
  options.end_time = end_time;
  options.gen_time = gen_time;
  options.search_time = search_time;
  options.time_margin = time_margin;
  if (_debug) {
    _print_time_list_options(options, cerr);
  }

  // send both 32-bit and 64-bit request parts
  // this is done so that legacy servers will respond correctly
  // in addition to new server

  // 32-bit

  time_list_options_32_t options32;
  _copyTimeListOptions64to32(options, options32);
  BE_from_array_32(&options32, sizeof(options32));
  addPart(MDVP_TIME_LIST_OPTIONS_PART_32, sizeof(options32), &options32);

  // 64-bit

  if (!_use32BitHeaders) {
    BE_from_array_64(&options, sizeof(options));
    addPart(MDVP_TIME_LIST_OPTIONS_PART_64, sizeof(options), &options);
  }

}

/////////////////////////////////
// add constrain lead times part

void DsMdvxMsg::_addConstrainLeadTimes(bool constrain_lead_times,
				       int min_lead_time,
				       int max_lead_time,
				       bool specify_by_gen_time)
  
{

  if (constrain_lead_times) {
    constrain_lead_times_t constrain;
    MEM_zero(constrain);
    constrain.min_lead_time = min_lead_time;
    constrain.max_lead_time = max_lead_time;
    constrain.specify_by_gen_time = specify_by_gen_time;
    BE_from_array_32(&constrain, sizeof(constrain));
    addPart(MDVP_CONSTRAIN_LEAD_TIMES_PART, sizeof(constrain), &constrain);
  }

}

/////////////////////////
// add master header part

void DsMdvxMsg::_addMasterHeader64(const Mdvx::master_header_t &header,
                                   int part_id)
{
  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printMasterHeader(header, cerr);
  }
  // make local copy for swapping
  Mdvx::master_header_t locHdr = header;
  Mdvx::master_header_to_BE(locHdr);
  // Add the data to the message.
  addPart(part_id, sizeof(locHdr), &locHdr);
}

void DsMdvxMsg::_addMasterHeader32(const Mdvx::master_header_t &header,
                                   int part_id)
{
  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printMasterHeader(header, cerr);
  }
  // make 32-bit copy
  Mdvx::master_header_32_t hdr32;
  Mdvx::_copyMasterHeader64to32(header, hdr32);
  Mdvx::master_header_to_BE_32(hdr32);
  // Add the data to the message.
  addPart(part_id, sizeof(hdr32), &hdr32);
}

/////////////////////////
// add field header part

void DsMdvxMsg::_addFieldHeader64(const Mdvx::field_header_t &header,
                                  int part_id)
{

  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printFieldHeader(header, cerr);
  }
  // make local copy for swapping
  Mdvx::field_header_t locHdr = header;
  Mdvx::field_header_to_BE(locHdr);
  // Add the data to the message.
  addPart(part_id, sizeof(locHdr), &locHdr);
}

void DsMdvxMsg::_addFieldHeader32(const Mdvx::field_header_t &header,
                                  int part_id)
{

  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printFieldHeader(header, cerr);
  }
  // make 32-bit copy
  Mdvx::field_header_32_t hdr32;
  Mdvx::_copyFieldHeader64to32(header, hdr32);
  Mdvx::field_header_to_BE_32(hdr32);
  // Add the data to the message.
  addPart(part_id, sizeof(hdr32), &hdr32);
}

/////////////////////////
// add vlevel header part

void DsMdvxMsg::_addVlevelHeader64(const Mdvx::vlevel_header_t &header,
                                   int part_id,
                                   int nz,
                                   const char *field_name)
{
  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printVlevelHeader(header, nz, field_name, cerr);
  }
  // make local copy for swapping
  Mdvx::vlevel_header_t locHdr = header;
  Mdvx::vlevel_header_to_BE(locHdr);
  // Add the data to the message.
  addPart(part_id, sizeof(locHdr), &locHdr);
}

void DsMdvxMsg::_addVlevelHeader32(const Mdvx::vlevel_header_t &header,
                                   int part_id,
                                   int nz,
                                   const char *field_name)
{
  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printVlevelHeader(header, nz, field_name, cerr);
  }
  // make 32-bit copy
  Mdvx::vlevel_header_32_t hdr32;
  Mdvx::_copyVlevelHeader64to32(header, hdr32);
  Mdvx::vlevel_header_to_BE_32(hdr32);
  // Add the data to the message.
  addPart(part_id, sizeof(hdr32), &hdr32);
}

/////////////////////////
// add chunk header part

void DsMdvxMsg::_addChunkHeader64(const Mdvx::chunk_header_t &header,
                                  int part_id)
{
  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printChunkHeader(header, cerr);
  }
  // make local copy for swapping
  Mdvx::chunk_header_t locHdr = header;
  Mdvx::chunk_header_to_BE(locHdr);
  // Add the data to the message.
  addPart(part_id, sizeof(locHdr), &locHdr);
}

void DsMdvxMsg::_addChunkHeader32(const Mdvx::chunk_header_t &header,
                                  int part_id)
{
  if (_debug) {
    cerr << "====>> Part: " << getLabelForPart(part_id) << " <<====" << endl;
    Mdvx::printChunkHeader(header, cerr);
  }
  // make 32-bit copy
  Mdvx::chunk_header_32_t hdr32;
  Mdvx::_copyChunkHeader64to32(header, hdr32);
  Mdvx::chunk_header_to_BE_32(hdr32);
  // Add the data to the message.
  addPart(part_id, sizeof(hdr32), &hdr32);
}

//////////////////////
// add field data part

void DsMdvxMsg::_addFieldData(const MdvxField& field)
{

  // tmp memory buffer for data

  MemBuf buf;
  if (field.getVolLen() > 0) {
    buf.add(field.getVol(), field.getVolLen());
  }
  
  // swap if data is not compressed

  if (!field.isCompressed()) {
    MdvxField::buffer_to_BE(buf.getPtr(), buf.getLen(),
			    field.getFieldHeader().encoding_type);
  }

  if (_debug) {
    cerr << "Adding field data, len: " << field.getVolLen() << endl;
  }

  // Add to the message.
  addPart(MDVP_FIELD_DATA_PART, buf.getLen(), buf.getPtr());

}

void DsMdvxMsg::_addFieldData64(const MdvxField& field)
{

  // tmp memory buffer for data
  
  MemBuf buf;
  if (field.getVolLen() > 0) {
    buf.add(field.getVol(), field.getVolLen());
  }
  
  // swap if data is not compressed
  if (!field.isCompressed()) {
    MdvxField::buffer_to_BE(buf.getPtr(), buf.getLen(),
			    field.getFieldHeader().encoding_type);
  }
  
  if (_debug) {
    cerr << "Adding field data, len: " << field.getVolLen() << endl;
  }
  
  // Add to the message.
  addPart(MDVP_FIELD_DATA_PART, buf.getLen(), buf.getPtr());

}

//////////////////////
// add chunk data part

void DsMdvxMsg::_addChunkData(const MdvxChunk& chunk)
{
  if (_debug) {
    cerr << "Adding chunk data, len: " << chunk.getSize() << endl;
  }
  addPart(MDVP_CHUNK_DATA_PART, chunk.getSize(), chunk.getData());
}

///////////////////////////////
// add vsection sample pts part

void DsMdvxMsg::_addVsectSamplePts
(const vector<Mdvx::vsect_samplept_t> &samplePts, double dx_km)
  
{

  // add 32-bit version

  MemBuf buf32;
  Mdvx::assembleVsectSamplePtsBuf32(samplePts, dx_km, buf32);
  if (_debug) {
    Mdvx::printVsectSamplePtsBuf32(buf32, cerr);
  }
  addPart(MDVP_VSECT_SAMPLE_PTS_PART_32, buf32.getLen(), buf32.getPtr());

  // add 64-bit version

  MemBuf buf64;
  Mdvx::assembleVsectSamplePtsBuf64(samplePts, dx_km, buf64);
  if (_debug) {
    Mdvx::printVsectSamplePtsBuf64(buf64, cerr);
  }
  addPart(MDVP_VSECT_SAMPLE_PTS_PART_64, buf64.getLen(), buf64.getPtr());

}

/////////////////////////////
// add vsection segments part

void DsMdvxMsg::_addVsectSegments
(const vector<Mdvx::vsect_segment_t> &segments, double totalLength)

{

  // add 32-bit version

  MemBuf buf32;
  Mdvx::assembleVsectSegmentsBuf32(segments, totalLength, buf32);
  if (_debug) {
    Mdvx::printVsectSegmentsBuf32(buf32, cerr);
  }
  addPart(MDVP_VSECT_SEGMENTS_PART_32, buf32.getLen(), buf32.getPtr());

  // add 64-bit version

  MemBuf buf64;
  Mdvx::assembleVsectSegmentsBuf64(segments, totalLength, buf64);
  if (_debug) {
    Mdvx::printVsectSegmentsBuf64(buf64, cerr);
  }
  addPart(MDVP_VSECT_SEGMENTS_PART_64, buf64.getLen(), buf64.getPtr());

}

//////////////////////////////////////
// add the time list parts
//
// returns 0 on success, -1 on failure

void DsMdvxMsg::_addTimeLists(const DsMdvx &mdvx)
  
{
  _addValidTimes(mdvx.getValidTimes(), mdvx.timeListHasForecasts());
  _addGenTimes(mdvx.getGenTimes(), mdvx.timeListHasForecasts());
  _addForecastTimes(mdvx._timeList.getForecastTimesArray());
}

////////////////////////
// add valid times part

void DsMdvxMsg::_addValidTimes(const vector<time_t> &valid_times,
			       bool has_forecasts)
  
{
  MemBuf tbuf;
  _loadTimeListMemBuf(valid_times, has_forecasts, tbuf);
  addPart(MDVP_VALID_TIMES_PART, tbuf.getLen(), tbuf.getPtr());
}

/////////////////////
// add gen times part

void DsMdvxMsg::_addGenTimes(const vector<time_t> &gen_times,
			     bool has_forecasts)
  
{
  MemBuf tbuf;
  _loadTimeListMemBuf(gen_times, has_forecasts, tbuf);
  addPart(MDVP_GEN_TIMES_PART, tbuf.getLen(), tbuf.getPtr());
}

///////////////////////////
// add forecast times parts

void DsMdvxMsg::_addForecastTimes(const vector<vector<time_t> >
				  &forecast_times_array)

{

  for (size_t ii = 0; ii < forecast_times_array.size(); ii++) {
    const vector<time_t> forecastTimes = forecast_times_array[ii];
    MemBuf tbuf;
    _loadTimeListMemBuf(forecastTimes, true, tbuf);
    addPart(MDVP_FORECAST_TIMES_PART, tbuf.getLen(), tbuf.getPtr());
  } // ii

}


///////////////////////////////
// load MemBuf for time list

void DsMdvxMsg::_loadTimeListMemBuf(const vector<time_t> &time_list,
				    bool has_forecasts,
				    MemBuf &tbuf)
  
{

  time_list_hdr_t hdr;
  MEM_zero(hdr);
  int ntimes = (int) time_list.size();
  hdr.ntimes = ntimes;
  hdr.has_forecasts = has_forecasts;
  tbuf.add(&hdr, sizeof(hdr));
  for (int i = 0; i < ntimes; i++) {
    ti32 dtime = time_list[i];
    tbuf.add(&dtime, sizeof(ti32));
  }
  BE_from_array_32(tbuf.getPtr(), tbuf.getLen());

}

/////////////////////
// add pathInUse part

void DsMdvxMsg::_addPathInUse(const string &path_in_use)
{
  if (_debug) {
    cerr << "Adding MDVP_PATH_IN_USE_PART, path: " << path_in_use << endl;
  }
  addPart(MDVP_PATH_IN_USE_PART, path_in_use.size() + 1,
	  path_in_use.c_str());
}

/////////////////////////
// add single buffer part

void DsMdvxMsg::_addSingleBuffer(const MemBuf &buf)
{
  if (_debug) {
    cerr << "Adding MDVP_SINGLE_BUFFER_PART, len: " << buf.getLen() << endl;
  }
  addPart(MDVP_SINGLE_BUFFER_PART, buf.getLen(), buf.getPtr());
}

/////////////////////////
// add xml header part

void DsMdvxMsg::_addXmlHeader(const string &xml)
{
  if (_debug) {
    cerr << "Adding MDVP_XML_HEADER_PART, len: " << xml.size() << endl;
  }
  addPart(MDVP_XML_HEADER_PART, xml.size() + 1, xml.c_str());
}

/////////////////////////
// add xml buffer part

void DsMdvxMsg::_addXmlBuffer(const MemBuf &buf)
{
  if (_debug) {
    cerr << "Adding MDVP_XML_BUFFER_PART, len: " << buf.getLen() << endl;
  }
  addPart(MDVP_XML_BUFFER_PART, buf.getLen(), buf.getPtr());
}

/////////////////////////
// add netcdf CF parts

void DsMdvxMsg::_addNcfHdrAndData(const DsMdvx &mdvx)
  
{
  
  // Add the header part

  _addNcfHdr(mdvx);
  
  // Add the data part

  const MemBuf &buf = mdvx._ncfBuf;

  if (_debug) {
    cerr << "Adding MDVP_NETCDF_BUFFER_PART, len: " << buf.getLen() << endl;
  }
  addPart(MDVP_NCF_BUFFER_PART, buf.getLen(), buf.getPtr());

}

/////////////////////////
// add just the netcdf CF header parts

void DsMdvxMsg::_addNcfHdr(const DsMdvx &mdvx)
  
{
  
  time_t valid_time = mdvx._ncfValidTime;
  time_t gen_time = mdvx._ncfGenTime;
  time_t forecast_time = mdvx._ncfForecastTime;
  int forecast_lead_secs = mdvx._ncfForecastDelta;
  bool is_forecast = mdvx._ncfIsForecast;
  int epoch = mdvx._ncfEpoch;
  string suffix = mdvx._ncfFileSuffix;
  
  string xml;
  xml += TaXml::writeStartTag("ncf-header", 0);

  xml += TaXml::writeDouble("valid_time", 1, valid_time, "%.15e");
  xml += TaXml::writeDouble("gen_time", 1, gen_time, "%.15e");
  xml += TaXml::writeDouble("forecast_time", 1, forecast_time, "%.15e");
  xml += TaXml::writeInt("forecast_lead_secs", 1, forecast_lead_secs);
  xml += TaXml::writeBoolean("is_forecast", 1, is_forecast);
  xml += TaXml::writeInt("epoch", 1, epoch);
  xml += TaXml::writeString("suffix", 1, suffix);
  
  xml += TaXml::writeEndTag("ncf-header", 0);
  
  if (_debug) {
    cerr << "Adding MDVP_NETCDF_HEADER_PART" << endl;
    cerr << xml << endl;
  }
  addPart(MDVP_NCF_HEADER_PART, xml.size() + 1, xml.c_str());
  
}


////////////////////////////
// add convert mdv2ncf part
//
// This part is encoded in XML

void DsMdvxMsg::_addConvertMdv2NcfPart(const DsMdvx &mdvx)
  
{

  string institution = mdvx._ncfInstitution;
  string references = mdvx._ncfReferences;
  string comment = mdvx._ncfComment;
  const vector<DsMdvx::Mdv2NcfFieldTrans> &fieldTrans =
    mdvx._mdv2NcfTransArray;
  bool compress = mdvx._ncfCompress;
  int compressionLevel = mdvx._ncfCompressionLevel;
  DsMdvx::nc_file_format_t fileFormat = mdvx._ncfFileFormat;
  DsMdvx::radial_file_type_t fileType = mdvx._ncfRadialFileType;
  bool outputLatlonArrays = mdvx._ncfOutputLatlonArrays;
  bool outputMdvAttr = mdvx._ncfOutputMdvAttr;
  bool outputMdvChunks = mdvx._ncfOutputMdvChunks;
  bool outputStartEndTimes =  mdvx._ncfOutputStartEndTimes;
  
  string xml;
  xml += TaXml::writeStartTag("mdv-to-ncf-conversion", 0);

  xml += TaXml::writeString("institution", 1, institution);
  xml += TaXml::writeString("references", 1, references);
  xml += TaXml::writeString("comment", 1, comment);
  
  xml += TaXml::writeBoolean("compress", 1, compress);
  xml += TaXml::writeInt("compressionLevel", 1, compressionLevel);
  
  xml += TaXml::writeString("fileFormat", 1,
                            DsMdvx::ncFormat2Str(fileFormat));

  xml += TaXml::writeString("radialFileType", 1,
                            DsMdvx::radialFileType2Str(fileType));

  xml += TaXml::writeBoolean("outputLatlonArrays", 1, outputLatlonArrays);
  xml += TaXml::writeBoolean("outputMdvAttr", 1, outputMdvAttr);
  xml += TaXml::writeBoolean("outputMdvChunks", 1, outputMdvChunks);
  xml += TaXml::writeBoolean("outputStartEndTimes", 1,  outputStartEndTimes);

  for (int ii = 0; ii < (int) fieldTrans.size(); ii++) {

    xml += TaXml::writeStartTag("field-translation", 1);
    
    xml += TaXml::writeString("mdv_field_name", 2, fieldTrans[ii].mdvFieldName);
    xml += TaXml::writeString("ncf_field_name", 2, fieldTrans[ii].ncfFieldName);
    xml += TaXml::writeString("ncf_standard_name", 2, fieldTrans[ii].ncfStandardName);
    xml += TaXml::writeString("ncf_long_name", 2, fieldTrans[ii].ncfLongName);
    xml += TaXml::writeString("ncf_units", 2, fieldTrans[ii].ncfUnits);
    
    xml += TaXml::writeBoolean("do_linear_transform", 2, fieldTrans[ii].doLinearTransform);
    xml += TaXml::writeDouble("linear_mult", 2, fieldTrans[ii].linearMult);
    xml += TaXml::writeDouble("linear_offset", 2, fieldTrans[ii].linearOffset);
    xml += TaXml::writeString("packing", 2, DsMdvx::ncfPack2Str(fieldTrans[ii].packing));

    xml += TaXml::writeEndTag("field-translation", 1);

  } // ii
  
  xml += TaXml::writeEndTag("mdv-to-ncf-conversion", 0);

  if (_debug) {
    cerr << xml << endl;
  }
    
  if (_debug) {
    cerr << "Adding MDVP_CONVERT_MDV_TO_NCF_PART" << endl;
  }
  addPart(MDVP_CONVERT_MDV_TO_NCF_PART, xml.size() + 1, xml.c_str());

}

///////////////////////////////////
// add no files found on read part

void DsMdvxMsg::_addNoFilesFoundOnRead()
{
  // Add a (data-less) part indicating that the server found
  //   no files to read
  // 
  if (_debug) {
    cerr << "Adding MDVP_NO_FILES_FOUND_ON_READ_PART" << endl;
  }
  addPart(MDVP_NO_FILES_FOUND_ON_READ_PART, 0, NULL);
}

/////////////////////////////
// add climo statistic type part

void DsMdvxMsg::_addClimoStatTypes(const vector< DsMdvx::climo_stat_t > stat_list)
{
  MemBuf part_buffer;
  
  // Put the header in the part buffer

  climoTypePartHdr_t part_hdr;
  MEM_zero(part_hdr);
  part_hdr.num_stats = stat_list.size();
  
  part_buffer.add(&part_hdr, sizeof(part_hdr));
  
  // Put each of the statistics in the part buffer

  vector< DsMdvx::climo_stat_t >::const_iterator stat;
  
  for (stat = stat_list.begin(); stat != stat_list.end(); ++stat)
  {
    climoTypePart_t stat_part;
    MEM_zero(stat_part);
    stat_part.climo_type = stat->type;
    if (stat->divide_by_num_obs)
      stat_part.divide_by_num_obs = 1;
    else
      stat_part.divide_by_num_obs = 0;
    stat_part.params[0] = stat->params[0];
    stat_part.params[1] = stat->params[1];
    
    part_buffer.add(&stat_part, sizeof(stat_part));
  }
  
  if (_debug)
    _print_climo_stat_type(part_hdr,
			   (climoTypePart_t *)((char *)part_buffer.getPtr() +
					       sizeof(part_hdr)),
			   cerr);

  BE_from_array_32(part_buffer.getPtr(), part_buffer.getLen());
  addPart(MDVP_CLIMO_STATISTIC_TYPE_PART,
	  part_buffer.getLen(), part_buffer.getPtr());
}

/////////////////////////////
// add climo date range part

void DsMdvxMsg::_addClimoDataRange(const time_t start_time,
				   const time_t end_time)

{
  climoDataRange_t data_range;
  MEM_zero(data_range);
  data_range.start_time = start_time;
  data_range.end_time = end_time;
  if (_debug) {
    _print_climo_data_range(data_range, cerr);
  }
  // send both 32-bit and 64-bit request parts
  // this is done so that legacy servers will respond correctly
  // in addition to new server

  // 32-bit

  climoDataRange_32_t data_range32;
  _copyClimoDataRange64to32(data_range, data_range32);
  BE_from_array_32(&data_range32, sizeof(data_range32));
  addPart(MDVP_CLIMO_DATA_RANGE_PART_32, sizeof(data_range32), &data_range32);

  // 64-bit

  BE_from_array_64(&data_range, sizeof(data_range));
  addPart(MDVP_CLIMO_DATA_RANGE_PART_64, sizeof(data_range), &data_range);

}

/////////////////////////////
// add climo time range part

void DsMdvxMsg::_addClimoTimeRange(const int start_hour, const int start_min,
				   const int start_sec,
				   const int end_hour, const int end_min,
				   const int end_sec)

{
  climoTimeRange_t time_range;
  MEM_zero(time_range);
  time_range.start_hour = start_hour;
  time_range.start_minute = start_min;
  time_range.start_second = start_sec;
  time_range.end_hour = end_hour;
  time_range.end_minute = end_min;
  time_range.end_second = end_sec;
  
  if (_debug) {
    _print_climo_time_range(time_range, cerr);
  }
  BE_from_array_32(&time_range, sizeof(time_range));
  addPart(MDVP_CLIMO_TIME_RANGE_PART, sizeof(time_range), &time_range);
}

/////////////////////
// add app name part

void DsMdvxMsg::_addAppName(const string &app_name)
{
  if (_debug) {
    cerr << "Adding MDVP_APP_NAME_PART, app_name: " << app_name << endl;
  }
  addPart(MDVP_APP_NAME_PART, app_name.size() + 1,
	  app_name.c_str());
}

