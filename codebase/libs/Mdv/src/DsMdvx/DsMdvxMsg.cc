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

// constructor

DsMdvxMsg::DsMdvxMsg(memModel_t mem_model /* = CopyMem */) :
  DsServerMsg(mem_model)
{
  
}

// destructor

DsMdvxMsg::~DsMdvxMsg()
{

}

//////////////////////////////////////////
// print out main header and parts headers
//

void DsMdvxMsg::print(ostream &out, const char *spacer) const
{

  // print header

  printHeader(out, spacer);

  // create map of ids and labels

  PartHeaderLabelMap labels;

  labels.insert(PartHeaderLabel(MDVP_READ_URL_PART,
				"MDVP_READ_URL_PART"));
  labels.insert(PartHeaderLabel(MDVP_ERR_STRING_PART,
				"MDVP_ERR_STRING_PART"));
  labels.insert(PartHeaderLabel(MDVP_CLIENT_USER_PART,
				"MDVP_CLIENT_USER_PART"));
  labels.insert(PartHeaderLabel(MDVP_CLIENT_HOST_PART,
				"MDVP_CLIENT_HOST_PART"));
  labels.insert(PartHeaderLabel(MDVP_CLIENT_IPADDR_PART,
				"MDVP_CLIENT_IPADDR_PART"));

  labels.insert(PartHeaderLabel(MDVP_FILE_SEARCH_PART,
				"MDVP_FILE_SEARCH_PART"));
  labels.insert(PartHeaderLabel(MDVP_APP_NAME_PART,
				"MDVP_APP_NAME_PART"));

  labels.insert(PartHeaderLabel(MDVP_READ_FORMAT_PART,
				"MDVP_READ_FORMAT_PART"));
  labels.insert(PartHeaderLabel(MDVP_WRITE_FORMAT_PART,
				"MDVP_WRITE_FORMAT_PART"));
  labels.insert(PartHeaderLabel(MDVP_CURRENT_FORMAT_PART,
				"MDVP_CURRENT_FORMAT_PART"));

  labels.insert(PartHeaderLabel(MDVP_READ_FIELD_NUM_PART,
				"MDVP_READ_FIELD_NUM_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_FIELD_NAME_PART,
				"MDVP_READ_FIELD_NAME_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_CHUNK_NUM_PART,
				"MDVP_READ_CHUNK_NUM_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_HORIZ_LIMITS_PART,
				"MDVP_READ_HORIZ_LIMITS_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VLEVEL_LIMITS_PART,
				"MDVP_READ_VLEVEL_LIMITS_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_PLANE_NUM_LIMITS_PART,
				"MDVP_READ_PLANE_NUM_LIMITS_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_COMPOSITE_PART,
				"MDVP_READ_COMPOSITE_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_FILL_MISSING_PART,
				"MDVP_READ_FILL_MISSING_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_ENCODING_PART,
				"MDVP_READ_ENCODING_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_REMAP_PART,
				"MDVP_READ_REMAP_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_AUTO_REMAP_TO_LATLON_PART,
				"MDVP_READ_AUTO_REMAP_TO_LATLON_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_FIELD_FILE_HEADERS_PART,
				"MDVP_READ_FIELD_FILE_HEADERS_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VSECT_WAYPTS_PART,
				"MDVP_READ_VSECT_WAYPTS_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VSECT_NSAMPLES_PART,
				"MDVP_READ_VSECT_NSAMPLES_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VSECT_MAXSAMPLES_PART,
				"MDVP_READ_VSECT_MAXSAMPLES_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VSECT_DISABLE_INTERP_PART,
				"MDVP_READ_VSECT_DISABLE_INTERP_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VSECT_AS_RHI_PART,
				"MDVP_READ_VSECT_AS_RHI_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_AS_SINGLE_BUFFER_PART,
				"MDVP_READ_AS_SINGLE_BUFFER_PART"));

  labels.insert(PartHeaderLabel(MDVP_WRITE_OPTIONS_PART,
				"MDVP_WRITE_OPTIONS_PART"));
  labels.insert(PartHeaderLabel(MDVP_TIME_LIST_OPTIONS_PART,
				"MDVP_TIME_LIST_OPTIONS_PART"));

  labels.insert(PartHeaderLabel(MDVP_MASTER_HEADER_PART,
				"MDVP_MASTER_HEADER_PART"));
  labels.insert(PartHeaderLabel(MDVP_MASTER_HEADER_FILE_PART,
				"MDVP_MASTER_HEADER_FILE_PART"));
  labels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_PART,
				"MDVP_FIELD_HEADER_PART"));
  labels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_FILE_PART,
				"MDVP_FIELD_HEADER_FILE_PART"));
  labels.insert(PartHeaderLabel(MDVP_FIELD_HEADER_FILE_FIELD_PART,
				"MDVP_FIELD_HEADER_FILE_FIELD_PART"));
  labels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_PART,
				"MDVP_VLEVEL_HEADER_PART"));
  labels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_FILE_PART,
				"MDVP_VLEVEL_HEADER_FILE_PART"));
  labels.insert(PartHeaderLabel(MDVP_VLEVEL_HEADER_FILE_FIELD_PART,
				"MDVP_VLEVEL_HEADER_FILE_FIELD_PART"));
  labels.insert(PartHeaderLabel(MDVP_CHUNK_HEADER_PART,
				"MDVP_CHUNK_HEADER_PART"));
  labels.insert(PartHeaderLabel(MDVP_CHUNK_HEADER_FILE_PART,
				"MDVP_CHUNK_HEADER_FILE_PART"));
  labels.insert(PartHeaderLabel(MDVP_FIELD_DATA_PART,
				"MDVP_FIELD_DATA_PART"));
  labels.insert(PartHeaderLabel(MDVP_CHUNK_DATA_PART,
				"MDVP_CHUNK_DATA_PART"));

  labels.insert(PartHeaderLabel(MDVP_VSECT_SAMPLE_PTS_PART,
				"MDVP_VSECT_SAMPLE_PTS_PART"));
  labels.insert(PartHeaderLabel(MDVP_VSECT_SEGMENTS_PART,
				"MDVP_VSECT_SEGMENTS_PART"));

  labels.insert(PartHeaderLabel(MDVP_VALID_TIMES_PART,
				"MDVP_VALID_TIMES_PART"));
  labels.insert(PartHeaderLabel(MDVP_GEN_TIMES_PART,
				"MDVP_GEN_TIMES_PART"));
  labels.insert(PartHeaderLabel(MDVP_FORECAST_TIMES_PART,
				"MDVP_FORECAST_TIMES_PART"));

  labels.insert(PartHeaderLabel(MDVP_PATH_IN_USE_PART,
				"MDVP_PATH_IN_USE_PART"));

  labels.insert(PartHeaderLabel(MDVP_SINGLE_BUFFER_PART,
				"MDVP_SINGLE_BUFFER_PART"));

  labels.insert(PartHeaderLabel(MDVP_XML_HEADER_PART,
				"MDVP_XML_HEADER_PART"));
  labels.insert(PartHeaderLabel(MDVP_XML_BUFFER_PART,
				"MDVP_XML_BUFFER_PART"));

  labels.insert(PartHeaderLabel(MDVP_NO_FILES_FOUND_ON_READ_PART,
				"MDVP_NO_FILES_FOUND_ON_READ_PART"));

  labels.insert(PartHeaderLabel(MDVP_READ_DECIMATE_PART,
				"MDVP_READ_DECIMATE_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_VLEVEL_TYPE_PART,
				"MDVP_READ_VLEVEL_TYPE_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_TIME_LIST_ALSO_PART,
				"MDVP_READ_TIME_LIST_ALSO_PART"));
  labels.insert(PartHeaderLabel(MDVP_READ_LATEST_VALID_MOD_TIME_PART,
				"MDVP_READ_LATEST_VALID_MOD_TIME_PART"));
  labels.insert(PartHeaderLabel(MDVP_CONSTRAIN_LEAD_TIMES_PART,
				"MDVP_CONSTRAIN_LEAD_TIMES_PART"));

  labels.insert(PartHeaderLabel(MDVP_NCF_HEADER_PART,
				"MDVP_NCF_HEADER_PART"));
  labels.insert(PartHeaderLabel(MDVP_NCF_BUFFER_PART,
				"MDVP_NCF_BUFFER_PART"));
  labels.insert(PartHeaderLabel(MDVP_CONVERT_MDV_TO_NCF_PART,
				"MDVP_CONVERT_MDV_TO_NCF_PART"));

  labels.insert(PartHeaderLabel(MDVP_CLIMO_STATISTIC_TYPE_PART,
				"MDVP_CLIMO_STATISTIC_TYPE_PART"));
  labels.insert(PartHeaderLabel(MDVP_CLIMO_DATA_RANGE_PART,
				"MDVP_CLIMO_DATA_RANGE_PART"));
  labels.insert(PartHeaderLabel(MDVP_CLIMO_TIME_RANGE_PART,
				"MDVP_CLIMO_TIME_RANGE_PART"));
  
  // print parts using the labels
  
  printPartHeaders(out, spacer, labels);

}

/////////////////////
// print header only

void DsMdvxMsg::printHeader(ostream &out, const char *spacer) const
{
  DsServerMsg::printHeader(out, spacer);
}

