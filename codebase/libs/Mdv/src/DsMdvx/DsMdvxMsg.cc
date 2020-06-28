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
// DsMdvxMsg.cc
//
// DsMdvxMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// The DsMdvxMsg object provides the message protocol for
// the DsMdvx service.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <Mdv/DsMdvxMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/mem.h>
using namespace std;

//////////////////////////////////////////
// constructor

DsMdvxMsg::DsMdvxMsg(memModel_t mem_model /* = CopyMem */) :
  DsServerMsg(mem_model)
{
  _use32BitHeaders = false;
  _loadPartIdLabelMap();
}

//////////////////////////////////////////
// destructor

DsMdvxMsg::~DsMdvxMsg()
{

}

//////////////////////////////////////////////////
// check parts for consistency - 32-bit or 64-bit?
// returns 0 on success, -1 on failure

void DsMdvxMsg::check64BitHeaders() const
{
  _use32BitHeaders = true;
  if (_has64BitParts()) {
    // use 64-bit headers as appropriate
    _setUse32BitHeaders(false);
  }
}

//////////////////////////////////////////////////
// get the label for a part

string DsMdvxMsg::getLabelForPart(int id) const
{
  
  PartHeaderLabelMap::const_iterator pos = _partIdLabels.find(id);
  return pos->second;
}

//////////////////////////////////////////
// check for 32-bit or 64-bit parts
//

bool DsMdvxMsg::_has32BitParts() const
{
  
  if (getPartByType(MDVP_MASTER_HEADER_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_MASTER_HEADER_FILE_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FIELD_HEADER_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FIELD_HEADER_FILE_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FIELD_HEADER_FILE_FIELD_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_VLEVEL_HEADER_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_VLEVEL_HEADER_FILE_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_CHUNK_HEADER_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_CHUNK_HEADER_FILE_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FILE_SEARCH_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_READ_REMAP_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_TIME_LIST_OPTIONS_PART_32) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_CLIMO_DATA_RANGE_PART_32) != NULL) {
    return true;
  }
  return false;
}

bool DsMdvxMsg::_has64BitParts() const
{
  
  if (getPartByType(MDVP_MASTER_HEADER_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_MASTER_HEADER_FILE_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FIELD_HEADER_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FIELD_HEADER_FILE_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FIELD_HEADER_FILE_FIELD_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_VLEVEL_HEADER_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_VLEVEL_HEADER_FILE_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_CHUNK_HEADER_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_CHUNK_HEADER_FILE_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_FILE_SEARCH_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_READ_REMAP_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_TIME_LIST_OPTIONS_PART_64) != NULL) {
    return true;
  }
  if (getPartByType(MDVP_CLIMO_DATA_RANGE_PART_64) != NULL) {
    return true;
  }
  return false;
}

//////////////////////////////////////////
// load the part header label map
//

void DsMdvxMsg::_loadPartIdLabelMap()
{

  _partIdLabels.clear();

  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_URL_PART,
                                       "MDVP_READ_URL_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_ERR_STRING_PART,
                                       "MDVP_ERR_STRING_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIENT_USER_PART,
                                       "MDVP_CLIENT_USER_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIENT_HOST_PART,
                                       "MDVP_CLIENT_HOST_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIENT_IPADDR_PART,
                                       "MDVP_CLIENT_IPADDR_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_FILE_SEARCH_PART_32,
                                       "MDVP_FILE_SEARCH_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FILE_SEARCH_PART_64,
                                       "MDVP_FILE_SEARCH_PART_64"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_APP_NAME_PART,
                                       "MDVP_APP_NAME_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_FORMAT_PART,
                                       "MDVP_READ_FORMAT_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_WRITE_FORMAT_PART,
                                       "MDVP_WRITE_FORMAT_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_INTERNAL_FORMAT_PART,
                                       "MDVP_INTERNAL_FORMAT_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_FIELD_NUM_PART,
                                       "MDVP_READ_FIELD_NUM_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_FIELD_NAME_PART,
                                       "MDVP_READ_FIELD_NAME_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_CHUNK_NUM_PART,
                                       "MDVP_READ_CHUNK_NUM_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_HORIZ_LIMITS_PART,
                                       "MDVP_READ_HORIZ_LIMITS_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VLEVEL_LIMITS_PART,
                                       "MDVP_READ_VLEVEL_LIMITS_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_PLANE_NUM_LIMITS_PART,
                                       "MDVP_READ_PLANE_NUM_LIMITS_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_COMPOSITE_PART,
                                       "MDVP_READ_COMPOSITE_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_FILL_MISSING_PART,
                                       "MDVP_READ_FILL_MISSING_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_ENCODING_PART,
                                       "MDVP_READ_ENCODING_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_REMAP_PART_32,
                                       "MDVP_READ_REMAP_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_REMAP_PART_64,
                                       "MDVP_READ_REMAP_PART_64"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_AUTO_REMAP_TO_LATLON_PART,
                                       "MDVP_READ_AUTO_REMAP_TO_LATLON_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_FIELD_FILE_HEADERS_PART,
                                       "MDVP_READ_FIELD_FILE_HEADERS_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VSECT_WAYPTS_PART_32,
                                       "MDVP_READ_VSECT_WAYPTS_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VSECT_WAYPTS_PART_64,
                                       "MDVP_READ_VSECT_WAYPTS_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VSECT_NSAMPLES_PART,
                                       "MDVP_READ_VSECT_NSAMPLES_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VSECT_MAXSAMPLES_PART,
                                       "MDVP_READ_VSECT_MAXSAMPLES_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VSECT_DISABLE_INTERP_PART,
                                       "MDVP_READ_VSECT_DISABLE_INTERP_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VSECT_AS_RHI_PART,
                                       "MDVP_READ_VSECT_AS_RHI_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_AS_SINGLE_BUFFER_PART,
                                       "MDVP_READ_AS_SINGLE_BUFFER_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_WRITE_OPTIONS_PART,
                                       "MDVP_WRITE_OPTIONS_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_TIME_LIST_OPTIONS_PART_32,
                                       "MDVP_TIME_LIST_OPTIONS_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_TIME_LIST_OPTIONS_PART_64,
                                       "MDVP_TIME_LIST_OPTIONS_PART_64"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_MASTER_HEADER_PART_32,
                                       "MDVP_MASTER_HEADER_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_MASTER_HEADER_FILE_PART_32,
                                       "MDVP_MASTER_HEADER_FILE_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_PART_32,
                                       "MDVP_FIELD_HEADER_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_FILE_PART_32,
                                       "MDVP_FIELD_HEADER_FILE_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_FILE_FIELD_PART_32,
                                       "MDVP_FIELD_HEADER_FILE_FIELD_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_PART_32,
                                       "MDVP_VLEVEL_HEADER_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_FILE_PART_32,
                                       "MDVP_VLEVEL_HEADER_FILE_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32,
                                       "MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CHUNK_HEADER_PART_32,
                                       "MDVP_CHUNK_HEADER_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CHUNK_HEADER_FILE_PART_32,
                                       "MDVP_CHUNK_HEADER_FILE_PART_32"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_DATA_PART,
                                       "MDVP_FIELD_DATA_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CHUNK_DATA_PART,
                                       "MDVP_CHUNK_DATA_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_MASTER_HEADER_PART_64,
                                       "MDVP_MASTER_HEADER_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_MASTER_HEADER_FILE_PART_64,
                                       "MDVP_MASTER_HEADER_FILE_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_PART_64,
                                       "MDVP_FIELD_HEADER_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_FILE_PART_64,
                                       "MDVP_FIELD_HEADER_FILE_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_FILE_FIELD_PART_64,
                                       "MDVP_FIELD_HEADER_FILE_FIELD_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_PART_64,
                                       "MDVP_VLEVEL_HEADER_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_FILE_PART_64,
                                       "MDVP_VLEVEL_HEADER_FILE_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64,
                                       "MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CHUNK_HEADER_PART_64,
                                       "MDVP_CHUNK_HEADER_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CHUNK_HEADER_FILE_PART_64,
                                       "MDVP_CHUNK_HEADER_FILE_PART_64"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_VSECT_SAMPLE_PTS_PART_32,
                                       "MDVP_VSECT_SAMPLE_PTS_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VSECT_SAMPLE_PTS_PART_64,
                                       "MDVP_VSECT_SAMPLE_PTS_PART_64"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VSECT_SEGMENTS_PART_32,
                                       "MDVP_VSECT_SEGMENTS_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_VSECT_SEGMENTS_PART_64,
                                       "MDVP_VSECT_SEGMENTS_PART_64"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_VALID_TIMES_PART,
                                       "MDVP_VALID_TIMES_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_GEN_TIMES_PART,
                                       "MDVP_GEN_TIMES_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_FORECAST_TIMES_PART,
                                       "MDVP_FORECAST_TIMES_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_PATH_IN_USE_PART,
                                       "MDVP_PATH_IN_USE_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_SINGLE_BUFFER_PART,
                                       "MDVP_SINGLE_BUFFER_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_XML_HEADER_PART,
                                       "MDVP_XML_HEADER_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_XML_BUFFER_PART,
                                       "MDVP_XML_BUFFER_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_NO_FILES_FOUND_ON_READ_PART,
                                       "MDVP_NO_FILES_FOUND_ON_READ_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_DECIMATE_PART,
                                       "MDVP_READ_DECIMATE_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_VLEVEL_TYPE_PART,
                                       "MDVP_READ_VLEVEL_TYPE_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_TIME_LIST_ALSO_PART,
                                       "MDVP_READ_TIME_LIST_ALSO_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_READ_LATEST_VALID_MOD_TIME_PART,
                                       "MDVP_READ_LATEST_VALID_MOD_TIME_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CONSTRAIN_LEAD_TIMES_PART,
                                       "MDVP_CONSTRAIN_LEAD_TIMES_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_NCF_HEADER_PART,
                                       "MDVP_NCF_HEADER_PART"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_NCF_BUFFER_PART,
                                       "MDVP_NCF_BUFFER_PART"));
  // _partIdLabels.insert(PartHeaderLabel(MDVP_CONVERT_MDV_TO_NCF_PART,
  //                                      "MDVP_CONVERT_MDV_TO_NCF_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIMO_STATISTIC_TYPE_PART,
                                       "MDVP_CLIMO_STATISTIC_TYPE_PART"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIMO_DATA_RANGE_PART_32,
                                       "MDVP_CLIMO_DATA_RANGE_PART_32"));
  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIMO_DATA_RANGE_PART_64,
                                       "MDVP_CLIMO_DATA_RANGE_PART_64"));

  _partIdLabels.insert(PartHeaderLabel(MDVP_CLIMO_TIME_RANGE_PART,
                                       "MDVP_CLIMO_TIME_RANGE_PART"));

}
  
//////////////////////////////////////////
// print out main header and parts headers
//

void DsMdvxMsg::print(ostream &out, const char *spacer) const
{

  // print header

  printHeader(out, spacer);

  if (_has32BitParts()) {
    out << "Contains 32-bit header parts" << endl;
  }
  if (_has64BitParts()) {
    out << "Contains 64-bit header parts" << endl;
  }

  // print parts using the labels
  
  printPartHeaders(out, spacer, _partIdLabels);

}

/////////////////////
// print header only

void DsMdvxMsg::printHeader(ostream &out, const char *spacer) const
{
  DsServerMsg::printHeader(out, spacer);
}

/////////////////////////////////////////////////////////////////////
// file search header 32-bit to 64-bit and vice versa

void DsMdvxMsg::_copyFileSearch32to64(const file_search_32_t &fsearch32,
                                      file_search_64_t &fsearch64)
  
{

  memset(&fsearch64, 0, sizeof(fsearch64));

  fsearch64.file_search_mode = fsearch32.file_search_mode;
  fsearch64.search_margin_secs = fsearch32.search_margin_secs;
  fsearch64.search_time = fsearch32.search_time;
  fsearch64.forecast_lead_secs = fsearch32.forecast_lead_secs;
  fsearch64.valid_time_search_wt = fsearch32.valid_time_search_wt;

}

void DsMdvxMsg::_copyFileSearch64to32(const file_search_64_t &fsearch64,
                                      file_search_32_t &fsearch32)
  
{

  memset(&fsearch32, 0, sizeof(fsearch32));

  fsearch32.file_search_mode = fsearch64.file_search_mode;
  fsearch32.search_margin_secs = fsearch64.search_margin_secs;
  fsearch32.search_time = fsearch64.search_time;
  fsearch32.forecast_lead_secs = fsearch64.forecast_lead_secs;
  fsearch32.valid_time_search_wt = fsearch64.valid_time_search_wt;

}

/////////////////////////////////////////////////////////////////////
// read remap header 32-bit to 64-bit and vice versa

void DsMdvxMsg::_copyReadRemap32to64(const read_remap_32_t &remap32,
                                     read_remap_64_t &remap64)
  
{

  memset(&remap64, 0, sizeof(remap64));

  remap64.proj_type = remap32.proj_type;
  remap64.nx = remap32.nx;
  remap64.ny = remap32.ny;
  remap64.minx = remap32.minx;
  remap64.miny = remap32.miny;
  remap64.dx = remap32.dx;
  remap64.dy = remap32.dy;
  remap64.origin_lat = remap32.origin_lat;
  remap64.origin_lon = remap32.origin_lon;
  for (int ii = 0; ii < MDV_MAX_PROJ_PARAMS; ii++) {
    remap64.proj_params[ii] = remap32.proj_params[ii];
  }

}

void DsMdvxMsg::_copyReadRemap64to32(const read_remap_64_t &remap64,
                                     read_remap_32_t &remap32)
  
{

  memset(&remap32, 0, sizeof(remap32));

  remap32.proj_type = remap64.proj_type;
  remap32.nx = remap64.nx;
  remap32.ny = remap64.ny;
  remap32.minx = remap64.minx;
  remap32.miny = remap64.miny;
  remap32.dx = remap64.dx;
  remap32.dy = remap64.dy;
  remap32.origin_lat = remap64.origin_lat;
  remap32.origin_lon = remap64.origin_lon;
  for (int ii = 0; ii < MDV_MAX_PROJ_PARAMS; ii++) {
    remap32.proj_params[ii] = remap64.proj_params[ii];
  }

}

/////////////////////////////////////////////////////////////////////
// time list header 32-bit to 64-bit and vice versa

void DsMdvxMsg::_copyTimeListOptions32to64(const time_list_options_32_t &tlist32,
                                           time_list_options_64_t &tlist64)
  
{

  memset(&tlist64, 0, sizeof(tlist64));

  tlist64.mode = tlist32.mode;
  tlist64.start_time = tlist32.start_time;
  tlist64.end_time = tlist32.end_time;
  tlist64.gen_time = tlist32.gen_time;
  tlist64.search_time = tlist32.search_time;
  tlist64.time_margin = tlist32.time_margin;

}

void DsMdvxMsg::_copyTimeListOptions64to32(const time_list_options_64_t &tlist64,
                                           time_list_options_32_t &tlist32)
  
{

  memset(&tlist32, 0, sizeof(tlist32));

  tlist32.mode = tlist64.mode;
  tlist32.start_time = tlist64.start_time;
  tlist32.end_time = tlist64.end_time;
  tlist32.gen_time = tlist64.gen_time;
  tlist32.search_time = tlist64.search_time;
  tlist32.time_margin = tlist64.time_margin;

}

/////////////////////////////////////////////////////////////////////
// climo data range header 32-bit to 64-bit and vice versa

void DsMdvxMsg::_copyClimoDataRange32to64(const climoDataRange_32_t &drange32,
                                          climoDataRange_64_t &drange64)
  
{

  memset(&drange64, 0, sizeof(drange64));
  drange64.start_time = drange32.start_time;
  drange64.end_time = drange32.end_time;

}

void DsMdvxMsg::_copyClimoDataRange64to32(const climoDataRange_64_t &drange64,
                                          climoDataRange_32_t &drange32)
  
{

  memset(&drange32, 0, sizeof(drange32));
  drange32.start_time = drange64.start_time;
  drange32.end_time = drange64.end_time;

}

