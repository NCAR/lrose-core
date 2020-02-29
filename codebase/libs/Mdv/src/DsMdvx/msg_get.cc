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
// msg_get.cc
//
// load methods for DsMdvxMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <Mdv/DsMdvxMsg.hh>
#include <didss/DsMsgPart.hh>
#include <didss/RapDataDir.hh>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
using namespace std;

/////////////////////////////
// get data formats

int DsMdvxMsg::_getReadFormat(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_READ_FORMAT_PART);
  if (part == NULL) {
    return -1;
  }

  string formatStr(_part2Str(part));
  mdvx._readFormat = Mdvx::str2Format(formatStr);
  if (_debug) {
    cerr << "Found MDVP_READ_FORMAT_PART: "
         << Mdvx::format2Str(mdvx._readFormat) << endl;
  }

  return 0;

}

int DsMdvxMsg::_getWriteFormat(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_WRITE_FORMAT_PART);
  if (part == NULL) {
    return -1;
  }

  string formatStr(_part2Str(part));
  mdvx._writeFormat = Mdvx::str2Format(formatStr);
  if (_debug) {
    cerr << "Found MDVP_WRITE_FORMAT_PART: "
         << Mdvx::format2Str(mdvx._writeFormat) << endl;
  }

  return 0;

}

int DsMdvxMsg::_getInternalFormat(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_INTERNAL_FORMAT_PART);
  if (part == NULL) {
    return -1;
  }

  string formatStr(_part2Str(part));
  mdvx._internalFormat = Mdvx::str2Format(formatStr);
  if (_debug) {
    cerr << "Found MDVP_INTERNAL_FORMAT_PART: "
         << Mdvx::format2Str(mdvx._internalFormat) << endl;
  }

  return 0;

}

///////////////////////////////////////////////
// get the file search parameters for reading
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getReadSearch(DsMdvx &mdvx)

{
  
  // get URL
  
  DsMsgPart *urlPart = getPartByType(DS_URL);
  
  if (urlPart == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
    _errStr += "  Cannot find URL part.\n";
    return -1;
  }
  
  // String in part must be non-zero in length
  if (urlPart->getLength() <= 0) {
    _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
    _errStr += "  Zero-length URL.\n";
    return -1;
  }
  
  string urlStr(_part2Str(urlPart));
  DsURL url(urlStr);
  string filledPath;
  RapDataDir.fillPath(url.getFile(), filledPath);

  if (_debug) {
    cerr << "Getting URL: " << _part2Str(urlPart) << endl;
  }

  // look for 64-bit version

  file_search_t fsearch;
  if (_use32BitHeaders) {
    // 32-bit headers
    DsMsgPart * searchPart = getPartByType(MDVP_FILE_SEARCH_PART_32);
    if (searchPart == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
      _errStr += "  MDVP_FILE_SEARCH_PART_32 not found.\n";
      return -1;
    }
    // part must be big enough.
    file_search_32_t fsearch32;
    if (searchPart->getLength() != sizeof(fsearch32)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
      _errStr += "  MDVP_FILE_SEARCH_PART_32 is wrong size.\n";
      return -1;
    }
    memcpy(&fsearch32, searchPart->getBuf(), sizeof(fsearch32));
    // byte swap
    BE_to_array_32(&fsearch32, sizeof(fsearch32));
    // convert to 64-bit
    _copyFileSearch32to64(fsearch32, fsearch);
  } else {
    // 64-bit headers
    DsMsgPart * searchPart = getPartByType(MDVP_FILE_SEARCH_PART_64);
    if (searchPart == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
      _errStr += "  MDVP_FILE_SEARCH_PART_64 not found.\n";
      return -1;
    }
    // part must be big enough.
    if (searchPart->getLength() != sizeof(fsearch)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
      _errStr += "  MDVP_FILE_SEARCH_PART_64 is wrong size.\n";
      return -1;
    }
    memcpy(&fsearch, searchPart->getBuf(), sizeof(fsearch));
    // byte swap
    BE_to_array_64(&fsearch, sizeof(fsearch));
  }

  if (_debug) {
    _print_file_search(fsearch, cerr);
  }

  switch (fsearch.file_search_mode) {
  case MDVP_READ_FROM_PATH:
    mdvx.setReadPath(filledPath);
    break;

  case MDVP_READ_LAST:
  case MDVP_READ_CLOSEST:
  case MDVP_READ_FIRST_BEFORE:
  case MDVP_READ_FIRST_AFTER:
  case MDVP_READ_BEST_FORECAST:
  case MDVP_READ_SPECIFIED_FORECAST:
    mdvx.setReadTime((Mdvx::read_search_mode_t) fsearch.file_search_mode,
                     filledPath,
                     fsearch.search_margin_secs,
                     fsearch.search_time,
                     fsearch.forecast_lead_secs);
    break;

  default:
    _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
    TaStr::AddInt(_errStr, "  Invalid read time mode: ",
		  fsearch.file_search_mode);
    return -1;
    
  } // switch

  if (fsearch.valid_time_search_wt > 0) {
    mdvx.setValidTimeSearchWt(fsearch.valid_time_search_wt);
  }

  // optional part to constrain forecast lead times
  
  if (_getConstrainLeadTimes(mdvx)) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////
// get the read qualifiers
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getReadQualifiers(DsMdvx &mdvx)
  
{

  // field numbers / names
  
  if (getPartByType(MDVP_READ_FIELD_NUM_PART) != NULL) {
    
    DsMsgPart *part;
    int ii = 0;
    while ((part = getPartByType(MDVP_READ_FIELD_NUM_PART, ii)) != NULL) {
      // check part is correct size
      if (part->getLength() != sizeof(si32)) {
        _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
        _errStr += "  Field num is incorrect size.\n";
        TaStr::AddInt(_errStr, "  Field index: ", ii);
        TaStr::AddInt(_errStr, "  Size expected: ", sizeof(si32));
        TaStr::AddInt(_errStr, "  Size found in message: ",
		      part->getLength());
        return -1;
      }
      si32 num;
      memcpy(&num, part->getBuf(), sizeof(num));
      BE_to_array_32(&num, sizeof(num));
      mdvx.addReadField(num);
      if (_debug) {
        cerr << "Found field num: " << num << endl;
      }
     ii++;
    }
    
  } else if (getPartByType(MDVP_READ_FIELD_NAME_PART) != NULL) {
    
    DsMsgPart *part;
    int ii = 0;
    while ((part = getPartByType(MDVP_READ_FIELD_NAME_PART, ii)) != NULL) {
      // check part is correct size
      string name(_part2Str(part));
      if (name.size() < 1) {
        _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
        _errStr += "  Field name is blank.\n";
        TaStr::AddInt(_errStr, "  Field index: ", ii);
        return -1;
      }
      mdvx.addReadField(name);
      if (_debug) {
        cerr << "Found field name: " << name << endl;
      }
      ii++;
    }
    
  }

  // chunk numbers
  
  if (getPartByType(MDVP_READ_CHUNK_NUM_PART) != NULL) {
    
    DsMsgPart *part;
    int ii = 0;
    while ((part = getPartByType(MDVP_READ_CHUNK_NUM_PART, ii)) != NULL) {
      // check part is correct size
      if (part->getLength() != sizeof(si32)) {
        _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
        _errStr += "  Chunk num is incorrect size.\n";
        TaStr::AddInt(_errStr, "  Chunk index: ", ii);
        TaStr::AddInt(_errStr, "  Size expected: ", sizeof(si32));
        TaStr::AddInt(_errStr, "  Size found in message: ",
		      part->getLength());
        return -1;
      }
      si32 num;
      memcpy(&num, part->getBuf(), sizeof(num));
      BE_to_array_32(&num, sizeof(num));
      if (_debug) {
        cerr << "Found chunk num: " << num << endl;
      }
      if (num < 0) {
        mdvx.setReadNoChunks();
      } else {
        mdvx.addReadChunk(num);
      }
      ii++;
    }
    
  }

  // horizontal limits
  
  if (getPartByType(MDVP_READ_HORIZ_LIMITS_PART) != NULL) {
    if (_getReadHorizLimits(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  // vertical limits

  if (getPartByType(MDVP_READ_VLEVEL_LIMITS_PART) != NULL) {
    if (_getReadVlevelLimits(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  } else if (getPartByType(MDVP_READ_PLANE_NUM_LIMITS_PART) != NULL) {
    if (_getReadPlaneNumLimits(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  // composite?

  if (getPartByType(MDVP_READ_COMPOSITE_PART) != NULL) {
    if (_getReadComposite(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  // fillMissing?

  if (getPartByType(MDVP_READ_FILL_MISSING_PART) != NULL) {
    mdvx.setReadFillMissing();
  }
  
  // encoding, compression, scaling

  if (getPartByType(MDVP_READ_ENCODING_PART) != NULL) {
    if (_getReadEncoding(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  // remapping

  if (getPartByType(MDVP_READ_REMAP_PART_64) != NULL) {
    if (_getReadRemap(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  if (getPartByType(MDVP_READ_REMAP_PART_32) != NULL) {
    if (_getReadRemap32(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  if (getPartByType(MDVP_READ_AUTO_REMAP_TO_LATLON_PART) != NULL) {
    if (_getReadAutoRemap2LatLon(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadQualifiers.\n";
      return -1;
    }
  }

  // field file headers?

  if (getPartByType(MDVP_READ_FIELD_FILE_HEADERS_PART) != NULL) {
    mdvx.setReadFieldFileHeaders();
  }

  // single buffer?

  if (getPartByType(MDVP_READ_AS_SINGLE_BUFFER_PART) != NULL) {
    mdvx.setReadAsSinglePart();
  }

  // decimate on read?

  if (getPartByType(MDVP_READ_DECIMATE_PART) != NULL) {
    if (_getReadDecimate(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadDecimate.\n";
      return -1;
    }
  }

  // set vlevel type?
  
  if (getPartByType(MDVP_READ_VLEVEL_TYPE_PART) != NULL) {
    if (_getReadVlevelType(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVlevelType.\n";
      return -1;
    }
  }

  // read vsect as RHI?
  
  if (getPartByType(MDVP_READ_VSECT_AS_RHI_PART) != NULL) {
    if (_getReadVsectAsRhi(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVlevelType.\n";
      return -1;
    }
  }

  return 0;

}

/////////////////////////////////////////////
// get the read vsect qualifiers
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getReadVsectQualifiers(DsMdvx &mdvx)
  
{

  if (_getReadNVsectSamples(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadVsectQualifiers.\n";
    return -1;
  }
  
  if (_getReadMaxVsectSamples(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadVsectQualifiers.\n";
    return -1;
  }
  
  if (_getReadVsectWaypts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadVsectQualifiers.\n";
    return -1;
  }

  if (_getReadVsectInterpDisabled(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadVsectQualifiers.\n";
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////
// get the return vsect info
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getReturnVsectInfo(DsMdvx &mdvx)
  
{

  if (_getReadVsectWaypts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReturnVsectInfo.\n";
    return -1;
  }

  if (_getVsectSegments(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReturnVsectInfo.\n";
    return -1;
  }

  if (_getVsectSamplepts(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getReturnVsectInfo.\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////
// get the climatology qualifiers
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getClimoQualifiers(DsMdvx &mdvx)
{
  // All of the climatology parts are optional, so just update the
  // information in the mdvx object for each part found.

  // Climatology types -- there can be any number of these parts.
  
  _getClimoStatTypes(mdvx);
  
  // Date range
  
  _getClimoDataRange(mdvx);
  
  // Time range
  
  _getClimoTimeRange(mdvx);
  
  return 0;
}

/////////////////////////////
// get read horizontal limits

int DsMdvxMsg::_getReadHorizLimits(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_READ_HORIZ_LIMITS_PART);
  if (part == NULL) {
    return -1;
  }

  read_horiz_limits_t hlimits;

  // part must be big enough.
  if (part->getLength() != sizeof(hlimits)) {
    _errStr += "ERROR - DsMdvxMsg::_getHorizLimits.\n";
    _errStr += "  Horix limits part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(hlimits));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&hlimits, part->getBuf(), sizeof(hlimits));
  BE_to_array_32(&hlimits, sizeof(hlimits));
  if (_debug) {
    _print_read_horiz_limits(hlimits, cerr);
  }
  
  mdvx.setReadHorizLimits(hlimits.min_lat, hlimits.min_lon,
                          hlimits.max_lat, hlimits.max_lon);

  return 0;

}

/////////////////////////
// get read vlevel limits

int DsMdvxMsg::_getReadVlevelLimits(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_READ_VLEVEL_LIMITS_PART);
  if (part == NULL) {
    return -1;
  }

  read_vlevel_limits_t vlimits;

  // part must be big enough.
  if (part->getLength() != sizeof(vlimits)) {
    _errStr += "ERROR - DsMdvxMsg::_getVlevelLimits.\n";
    _errStr += "  Horix limits part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(vlimits));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&vlimits, part->getBuf(), sizeof(vlimits));
  BE_to_array_32(&vlimits, sizeof(vlimits));
  if (_debug) {
    _print_read_vlevel_limits(vlimits, cerr);
  }
  
  mdvx.setReadVlevelLimits(vlimits.min_vlevel, vlimits.max_vlevel);

  return 0;

}

/////////////////////////////
// get read plane_num limits

int DsMdvxMsg::_getReadPlaneNumLimits(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_READ_PLANE_NUM_LIMITS_PART);
  if (part == NULL) {
    return -1;
  }

  read_plane_num_limits_t vlimits;

  // part must be big enough.
  if (part->getLength() != sizeof(vlimits)) {
    _errStr += "ERROR - DsMdvxMsg::_getPlane_NumLimits.\n";
    _errStr += "  Horix limits part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(vlimits));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&vlimits, part->getBuf(), sizeof(vlimits));
  BE_to_array_32(&vlimits, sizeof(vlimits));
  if (_debug) {
    _print_read_plane_num_limits(vlimits, cerr);
  }
  
  mdvx.setReadPlaneNumLimits(vlimits.min_plane_num, vlimits.max_plane_num);

  return 0;

}

//////////////////////////
// get read composite part

int DsMdvxMsg::_getReadComposite(DsMdvx &mdvx)
{
  DsMsgPart * part = getPartByType(MDVP_READ_COMPOSITE_PART);
  if (part == NULL) {                             
    return -1;
  }
  read_composite_t comp;
  // part must be big enough.
  // This msg contains a spare.
  // 
  if (part->getLength() != sizeof(comp)) {
    _errStr += "ERROR - DsMdvxMsg::_getComposite.\n";
    _errStr += "  Composite part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(comp));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  memcpy(&comp, part->getBuf(), sizeof(comp));
  BE_to_array_32(&comp, sizeof(comp));
  if (_debug) {
    _print_read_composite(comp, cerr);
  }
  composite_method_t type = (DsMdvxMsg::composite_method_t) comp.type;
  if (type == MDVP_COMPOSITE_MAX) {
    mdvx.setReadComposite();
  }
  return 0;
}

/////////////////////////
// get read encoding type

int DsMdvxMsg::_getReadEncoding(DsMdvx &mdvx)
{
  DsMsgPart * part = getPartByType(MDVP_READ_ENCODING_PART);
  if (part == NULL) {
    return -1;
  }
  read_encoding_t encod;
  // part must be big enough.
  // This msg contains a spare.
  // 
  if (part->getLength() != sizeof(encod)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadEncoding.\n";
    _errStr += "  Encoding part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(encod));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  memcpy(&encod, part->getBuf(), sizeof(encod));
  BE_to_array_32(&encod, sizeof(encod));
  if (_debug) {
    _print_read_encoding(encod, cerr);
  }
  mdvx.setReadEncodingType((Mdvx::encoding_type_t)
                           encod.encoding_type);
  mdvx.setReadCompressionType((Mdvx::compression_type_t)
                              encod.compression_type);
  mdvx.setReadScalingType((Mdvx::scaling_type_t) encod.scaling_type,
                          encod.scale, encod.bias);
  
  return 0;

}

///////////////////
// get read remap

int DsMdvxMsg::_getReadRemap(DsMdvx &mdvx)
{

  // decode part

  DsMsgPart * part = getPartByType(MDVP_READ_REMAP_PART_64);
  if (part == NULL) {
    return -1;
  }
  read_remap_t remap;
  // part must be big enough.
  if (part->getLength() != sizeof(remap)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadRemap.\n";
    _errStr += "  Remap coords part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(remap));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&remap, part->getBuf(), sizeof(remap));
  BE_to_array_64(&remap, sizeof(remap));
  if (_debug) {
    _print_read_remap(remap, cerr);
  }
  
  // load up coord from remap into

  Mdvx::coord_t remapCoord;
  remapCoord.proj_type = remap.proj_type;
  remapCoord.nx = remap.nx;
  remapCoord.ny = remap.ny;
  remapCoord.dx = remap.dx;
  remapCoord.dy = remap.dy;
  remapCoord.minx = remap.minx;
  remapCoord.miny = remap.miny;
  remapCoord.proj_origin_lat = remap.origin_lat;
  remapCoord.proj_origin_lon = remap.origin_lon;
  MdvxProj::_projParams2Coord((Mdvx::projection_type_t) remap.proj_type,
			      remap.proj_params, remapCoord);
  
  // create projection
  
  MdvxProj remapProj(remapCoord);

  // set read remap

  mdvx.setReadRemap(remapProj);
  
  return 0;

}

int DsMdvxMsg::_getReadRemap32(DsMdvx &mdvx)
{
   
  // decode part

  DsMsgPart * part = getPartByType(MDVP_READ_REMAP_PART_32);
  if (part == NULL) {
    return -1;
  }
  read_remap_32_t remap32;
  // part must be big enough.
  if (part->getLength() != sizeof(remap32)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadRemap32.\n";
    _errStr += "  Remap coords part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(remap32));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&remap32, part->getBuf(), sizeof(remap32));
  BE_to_array_32(&remap32, sizeof(remap32));

  read_remap_t remap;
  _copyReadRemap32to64(remap32, remap);

  if (_debug) {
    _print_read_remap(remap, cerr);
  }
  
  // load up coord from remap into

  Mdvx::coord_t remapCoord;
  remapCoord.proj_type = remap.proj_type;
  remapCoord.nx = remap.nx;
  remapCoord.ny = remap.ny;
  remapCoord.dx = remap.dx;
  remapCoord.dy = remap.dy;
  remapCoord.minx = remap.minx;
  remapCoord.miny = remap.miny;
  remapCoord.proj_origin_lat = remap.origin_lat;
  remapCoord.proj_origin_lon = remap.origin_lon;
  MdvxProj::_projParams2Coord((Mdvx::projection_type_t) remap.proj_type,
			      remap.proj_params, remapCoord);
  
  // create projection
  
  MdvxProj remapProj(remapCoord);

  // set read remap

  mdvx.setReadRemap(remapProj);
  
  return 0;

}


////////////////////////////////
// get read auto remap 2 latlon

int DsMdvxMsg::_getReadAutoRemap2LatLon(DsMdvx &mdvx)
  
{
  DsMsgPart * part;
  part = getPartByType(MDVP_READ_AUTO_REMAP_TO_LATLON_PART);
  if (part == NULL) {
    return 0;
  }
  mdvx.setReadAutoRemap2LatLon();

  if (_debug) {
    cerr << "  Read auto remap 2 latlon" << endl;
  }

  return 0;
}

////////////////////////////////
// get read decimate

int DsMdvxMsg::_getReadDecimate(DsMdvx &mdvx)
  
{
  DsMsgPart * part;
  part = getPartByType(MDVP_READ_DECIMATE_PART);
  if (part == NULL) {
    return 0;
  }
  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(si32)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadDecimate.\n";
    _errStr += "  Decimate part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(si32));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  si32 maxNxy;
  memcpy(&maxNxy, bptr, sizeof(maxNxy));
  BE_to_array_32(&maxNxy, sizeof(maxNxy));

  mdvx.setReadDecimate(maxNxy);

  if (_debug) {
    cerr << "  Read decimate set, maxNxy: " << maxNxy << endl;
  }

  return 0;
}

////////////////////////////////
// get read time list also

int DsMdvxMsg::_getReadTimeListAlso(DsMdvx &mdvx)
  
{
  if (getPartByType(MDVP_READ_TIME_LIST_ALSO_PART) == NULL) {
    return -1;
  }
  mdvx.setReadTimeListAlso();
  return 0;
}

//////////////////////////////////////////////
// get read check file mod times for validity

int DsMdvxMsg::_getReadLatestValidModTime(DsMdvx &mdvx)
  
{
  DsMsgPart * part;
  part = getPartByType(MDVP_READ_LATEST_VALID_MOD_TIME_PART);
  if (part == NULL) {
    return 0;
  }
  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(ti32)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadCheckModTime.\n";
    _errStr += "  Mod time part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(ti32));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  ti32 latestValidModTime;
  memcpy(&latestValidModTime, bptr, sizeof(latestValidModTime));
  BE_to_array_32(&latestValidModTime, sizeof(latestValidModTime));

  mdvx.setCheckLatestValidModTime(latestValidModTime);

  if (_debug) {
    cerr << "  Setting latest valid mod time to: "
	 << DateTime::str(latestValidModTime, false) << endl;
  }

  return 0;
}

////////////////////////////////
// get read vlevel type

int DsMdvxMsg::_getReadVlevelType(DsMdvx &mdvx)
  
{

  DsMsgPart * part;
  part = getPartByType(MDVP_READ_VLEVEL_TYPE_PART);
  if (part == NULL) {
    return 0;
  }
  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(si32)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadVlevelType.\n";
    _errStr += "  Vlevel type part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(si32));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  si32 vlevelType;
  memcpy(&vlevelType, bptr, sizeof(vlevelType));
  BE_to_array_32(&vlevelType, sizeof(vlevelType));

  mdvx.setReadVlevelType((Mdvx::vlevel_type_t) vlevelType);

  if (_debug) {
    cerr << "  Read vlevel type set: "
	 << Mdvx::vertType2Str(vlevelType) << endl;
  }

  return 0;
}

////////////////////////////////
// get read vsection way points

int DsMdvxMsg::_getReadVsectWaypts(DsMdvx &mdvx)
  
{

  vector<Mdvx::vsect_waypt_t> wayPts;

  // try 64-bit version

  DsMsgPart *part64 = getPartByType(MDVP_READ_VSECT_WAYPTS_PART_64);
  if (part64 != NULL) {

    MemBuf buf64;
    buf64.add(part64->getBuf(), part64->getLength());
    string errStr;
    if (Mdvx::disassembleVsectWayPtsBuf64(buf64, wayPts, errStr)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectWaypts.\n";
      _errStr += errStr;
      return -1;
    }
    if (_debug) {
      Mdvx::printVsectWayPtsBuf64(buf64, cerr);
    }

  } else {

    // try 32-bit

    DsMsgPart *part32 = getPartByType(MDVP_READ_VSECT_WAYPTS_PART_32);
    if (part32 == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectWaypts.\n";
      _errStr += "  Cannot find waypts part.\n";
      return -1;
    }
    MemBuf buf32;
    buf32.add(part32->getBuf(), part32->getLength());
    string errStr;
    if (Mdvx::disassembleVsectWayPtsBuf32(buf32, wayPts, errStr)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectWaypts.\n";
      _errStr += errStr;
      return -1;
    }
    if (_debug) {
      Mdvx::printVsectWayPtsBuf32(buf32, cerr);
    }

  } // if (part64 ...
  
  mdvx.clearReadWayPts();
  for (int ii = 0; ii < (int) wayPts.size(); ii++) {
    const Mdvx::vsect_waypt_t &pt = wayPts[ii];
    mdvx.addReadWayPt(pt.lat, pt.lon);
  }

  return 0;

}

////////////////////////////////
// get read vsection n_samples

int DsMdvxMsg::_getReadNVsectSamples(DsMdvx &mdvx)
  
{
  DsMsgPart * part;
  part = getPartByType(MDVP_READ_VSECT_NSAMPLES_PART);
  if (part == NULL) {
    return 0;
  }
  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(si32)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadNVsectSamples.\n";
    _errStr += "  NSamples part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(si32));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  si32 nSamples;
  memcpy(&nSamples, bptr, sizeof(nSamples));
  BE_to_array_32(&nSamples, sizeof(nSamples));

  mdvx.setReadNVsectSamples(nSamples);

  return 0;
}

////////////////////////////////
// get read vsection max_samples

int DsMdvxMsg::_getReadMaxVsectSamples(DsMdvx &mdvx)
  
{
  DsMsgPart * part;
  part = getPartByType(MDVP_READ_VSECT_MAXSAMPLES_PART);
  if (part == NULL) {
    return 0;
  }
  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(si32)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadMaxVsectSamples.\n";
    _errStr += "  MaxSamples part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(si32));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  si32 maxSamples;
  memcpy(&maxSamples, bptr, sizeof(maxSamples));
  BE_to_array_32(&maxSamples, sizeof(maxSamples));

  mdvx.setReadMaxVsectSamples(maxSamples);

  return 0;
}

////////////////////////////////
// get read vsect as RHI

int DsMdvxMsg::_getReadVsectAsRhi(DsMdvx &mdvx)
  
{
  DsMsgPart * part;
  part = getPartByType(MDVP_READ_VSECT_AS_RHI_PART);
  if (part == NULL) {
    return 0;
  }
  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(rhi_request_t)) {
    _errStr += "ERROR - DsMdvxMsg::_getReadVsectAsRhi.\n";
    _errStr += "  Max az error part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(rhi_request_t));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  rhi_request_t req;
  memcpy(&req, bptr, sizeof(req));
  BE_to_array_32(&req, sizeof(req));
  
  mdvx.setReadVsectAsRhi(req.as_polar, req.max_az_error, req.respect_user_dist);

  return 0;
}

////////////////////////
// get the master header

int DsMdvxMsg::_getMasterHeader(Mdvx::master_header_t &mhdr,
                                int part_id)
{
  // Get the part.
  DsMsgPart * part = getPartByType(part_id);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getMasterHeader.\n";
    _errStr += "  Cannot find master header part.\n";
    return -1;
  }
  if (_use32BitHeaders) {
    // 32-bit
    Mdvx::master_header_32_t mhdr32;
    // Part must be big enough.
    if (part->getLength() != sizeof(mhdr32)) {
      _errStr += "ERROR - DsMdvxMsg::_getMasterHeader.\n";
      _errStr += "  Master header 32-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(mhdr32));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy perform byte swapping.
    memcpy(&mhdr32, part->getBuf(), sizeof(mhdr32));
    Mdvx::master_header_from_BE_32(mhdr32);
    // copy to 64-bit version
    Mdvx::_copyMasterHeader32to64(mhdr32, mhdr);
  } else {
    // 64-bit
    // Part must be big enough.
    if (part->getLength() != sizeof(mhdr)) {
      _errStr += "ERROR - DsMdvxMsg::_getMasterHeader.\n";
      _errStr += "  Master header 64-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(mhdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy perform byte swapping.
    memcpy(&mhdr, part->getBuf(), sizeof(mhdr));
    Mdvx::master_header_from_BE(mhdr);
  }
  if (_debug) {
    Mdvx::printMasterHeader(mhdr, cerr);
  }
  return 0;
}

///////////////////
// get field header

int DsMdvxMsg::_getFieldHeader(Mdvx::field_header_t &fhdr,
                               int field_num,
                               int part_id)
{
  // Get the part.
  DsMsgPart * part = getPartByType(part_id, field_num);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getFieldHeader.\n";
    _errStr += "  Cannot find field header part.\n";
    TaStr::AddInt(_errStr, "  Field num: ", field_num);
    return -1;
  }
  if (_use32BitHeaders) {
    // 32-bit
    Mdvx::field_header_32_t fhdr32;
    // Part must be big enough.
    if (part->getLength() != sizeof(fhdr32)) {
      _errStr += "ERROR - DsMdvxMsg::_getFieldHeader.\n";
      _errStr += "  Field header 32-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Field num: ", field_num);
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(fhdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy and perform byte swapping.
    memcpy(&fhdr32, part->getBuf(), sizeof(fhdr32));
    Mdvx::field_header_from_BE_32(fhdr32);
    // copy to 64-bit version
    Mdvx::_copyFieldHeader32to64(fhdr32, fhdr);
  } else {
    // 64-bit
    // Part must be big enough.
    if (part->getLength() != sizeof(fhdr)) {
      _errStr += "ERROR - DsMdvxMsg::_getFieldHeader.\n";
      _errStr += "  Field header 64-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Field num: ", field_num);
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(fhdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy and perform byte swapping.
    memcpy(&fhdr, part->getBuf(), sizeof(fhdr));
    Mdvx::field_header_from_BE(fhdr);
  }
  if (_debug) {
    Mdvx::printFieldHeader(fhdr, cerr);
  }

  return 0;
}

////////////////////
// get vlevel header

int DsMdvxMsg::_getVlevelHeader(Mdvx::vlevel_header_t &vhdr,
                                int field_num,
                                int part_id)
{
  // Get the part.
  DsMsgPart * part = getPartByType(part_id, field_num);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getVlevelHeader.\n";
    _errStr += "  Cannot find vlevel header part.\n";
    TaStr::AddInt(_errStr, "  Field num: ", field_num);
    return -1;
  }
  if (_use32BitHeaders) {
    // 32-bit
    Mdvx::vlevel_header_32_t vhdr32;
    // Part must be big enough.
    if (part->getLength() != sizeof(vhdr32)) {
      _errStr += "ERROR - DsMdvxMsg::_getVlevelHeader.\n";
      _errStr += "  Vlevel header 32-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Field num: ", field_num);
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(vhdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy and perform byte swapping.
    memcpy(&vhdr32, part->getBuf(), sizeof(vhdr32));
    Mdvx::vlevel_header_from_BE_32(vhdr32);
    // copy to 64-bit version
    Mdvx::_copyVlevelHeader32to64(vhdr32, vhdr);
  } else {
    // 64-bit
    // Part must be big enough.
    if (part->getLength() != sizeof(vhdr)) {
      _errStr += "ERROR - DsMdvxMsg::_getVlevelHeader.\n";
      _errStr += "  Vlevel header 64-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Field num: ", field_num);
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(vhdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy and perform byte swapping.
    memcpy(&vhdr, part->getBuf(), sizeof(vhdr));
    Mdvx::vlevel_header_from_BE(vhdr);
  }
  return 0;
}

///////////////////
// get chunk header

int DsMdvxMsg::_getChunkHeader(Mdvx::chunk_header_t &chdr,
                               int chunk_num,
                               int part_id)
{
  // Get the part.
  DsMsgPart * part = getPartByType(part_id, chunk_num);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getChunkHeader.\n";
    _errStr += "  Cannot find chunk header part.\n";
    TaStr::AddInt(_errStr, "  Chunk num: ", chunk_num);
    return -1;
  }
  if (_use32BitHeaders) {
    // 32-bit
    Mdvx::chunk_header_32_t chdr32;
    // Part must be big enough.
    if (part->getLength() != sizeof(chdr32)) {
      _errStr += "ERROR - DsMdvxMsg::_getChunkHeader.\n";
      _errStr += "  Chunk header 32-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Chunk num: ", chunk_num);
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(chdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy and perform byte swapping.
    memcpy(&chdr32, part->getBuf(), sizeof(chdr32));
    Mdvx::chunk_header_from_BE_32(chdr32);
    // copy to 64-bit version
    Mdvx::_copyChunkHeader32to64(chdr32, chdr);
  } else {
    // 64-bit
    // Part must be big enough.
    if (part->getLength() != sizeof(chdr)) {
      _errStr += "ERROR - DsMdvxMsg::_getChunkHeader.\n";
      _errStr += "  Chunk header 64-bit part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Chunk num: ", chunk_num);
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(chdr));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }
    // Copy and perform byte swapping.
    memcpy(&chdr, part->getBuf(), sizeof(chdr));
    Mdvx::chunk_header_from_BE(chdr);
  }
  if (_debug) {
    Mdvx::printChunkHeader(chdr, cerr);
  }
  return 0;
}

/////////////////////////////////////////////////////
// get mdv headers
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_getHeaders(DsMdvx &mdvx)
  
{

  if (_use32BitHeaders) {
    if (_getMasterHeader(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_32)) {
      _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
      _errStr += "Cannot find 32-bit master header file part\n";
      return -1;
    }
  } else {
    if (_getMasterHeader(mdvx._mhdrFile, MDVP_MASTER_HEADER_FILE_PART_64)) {
      _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
      _errStr += "Cannot find 64-bit master header file part\n";
      return -1;
    }
  }

  int n_fields = mdvx._mhdrFile.n_fields;
  
  mdvx._fhdrsFile.erase(mdvx._fhdrsFile.begin(), mdvx._fhdrsFile.end());
  for (int i = 0; i < n_fields; i++) {
    Mdvx::field_header_t fhdr;
    if (_use32BitHeaders) {
      if (_getFieldHeader(fhdr, i, MDVP_FIELD_HEADER_FILE_PART_32)) {
        _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
        _errStr += "Cannot find 32-bit field header file part\n";
        return -1;
      }
    } else {
      if (_getFieldHeader(fhdr, i, MDVP_FIELD_HEADER_FILE_PART_64)) {
        _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
        _errStr += "Cannot find 64-bit field header file part\n";
        return -1;
      }
    }
    mdvx._fhdrsFile.push_back(fhdr);
  }

  mdvx._vhdrsFile.erase(mdvx._vhdrsFile.begin(), mdvx._vhdrsFile.end());
  for (int i = 0; i < n_fields; i++) {
    Mdvx::vlevel_header_t vhdr;
    if (_use32BitHeaders) {
      if (_getVlevelHeader(vhdr, i, MDVP_VLEVEL_HEADER_FILE_PART_32)) {
        _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
        _errStr += "Cannot find 32-bit vlevel header file part\n";
        return -1;
      }
    } else {
      if (_getVlevelHeader(vhdr, i, MDVP_VLEVEL_HEADER_FILE_PART_64)) {
        _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
        _errStr += "Cannot find 64-bit vlevel header file part\n";
        return -1;
      }
    }
    mdvx._vhdrsFile.push_back(vhdr);
  }

  int n_chunks = mdvx._mhdrFile.n_chunks;
  
  mdvx._chdrsFile.erase(mdvx._chdrsFile.begin(), mdvx._chdrsFile.end());
  for (int i = 0; i < n_chunks; i++) {
    Mdvx::chunk_header_t chdr;
    if (_use32BitHeaders) {
      if (_getChunkHeader(chdr, i, MDVP_CHUNK_HEADER_FILE_PART_32)) {
        _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
        _errStr += "Cannot find 32-bit chunk header file part\n";
        return -1;
      }
    } else {
      if (_getChunkHeader(chdr, i, MDVP_CHUNK_HEADER_FILE_PART_64)) {
        _errStr += "ERROR - DsMdvxMsg::_getHeaders\n";
        _errStr += "Cannot find 64-bit chunk header file part\n";
        return -1;
      }
    }
    mdvx._chdrsFile.push_back(chdr);
  }
  
  return 0;

}

/////////////////////////////////////////////////////
// get mdv headers and data
//
// Returns 0 on success, -1 on error.
// getErrorStr() returns the error str for this call.

int DsMdvxMsg::_getHeadersAndData(DsMdvx &mdvx)
  
{

  // mdvx object is in separate parts
  
  if (_use32BitHeaders) {
    if (_getMasterHeader(mdvx._mhdr, MDVP_MASTER_HEADER_PART_32)) {
      _errStr += "ERROR - DsMdvxMsg::_getHeadersAndData\n";
      _errStr += "Cannot find 32-bit master header part\n";
      return -1;
    }
  } else {
    if (_getMasterHeader(mdvx._mhdr, MDVP_MASTER_HEADER_PART_64)) {
      _errStr += "ERROR - DsMdvxMsg::_getHeadersAndData\n";
      _errStr += "Cannot find 64-bit master header part\n";
      return -1;
    }
  }

  int n_fields = mdvx._mhdr.n_fields;
  mdvx.clearFields();
  for (int i = 0; i < n_fields; i++) {
    if (_getField(mdvx, i)) {
      _errStr += "ERROR - DsMdvxMsg::_getHeadersAndData\n";
      return -1;
    }
  }
  
  int n_chunks = mdvx._mhdr.n_chunks;
  mdvx.clearChunks();
  for (int i = 0; i < n_chunks; i++) {
    if (_getChunk(mdvx, i)) {
      _errStr += "ERROR - DsMdvxMsg::_getHeadersAndData\n";
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////
// get the path in use
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getPathInUse(DsMdvx &mdvx)

{
  // get path in use
  DsMsgPart *part = getPartByType(MDVP_PATH_IN_USE_PART);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getPathInUse.\n";
    _errStr += "  Cannot find pathInUse part.\n";
    return -1;
  }
  // String in part must be non-zero in length
  if (part->getLength() <= 0) {
    _errStr += "ERROR - DsMdvxMsg::_getPathInUse.\n";
    _errStr += "  Zero-length pathInUse part.\n";
    return -1;
  }
  mdvx._pathInUse = _part2Str(part);
  if (_debug) {
    cerr << "Found MDVP_PATH_IN_USE_PART: " << mdvx._pathInUse << endl;
  }
  return 0;
}

////////////////////
// get single buffer

int DsMdvxMsg::_getSingleBuffer(DsMdvx &mdvx)
{

  DsMsgPart *part = getPartByType(MDVP_SINGLE_BUFFER_PART);
  if (part == NULL) {
    return -1;
  }

  MemBuf buf;
  buf.add(part->getBuf(), part->getLength());
  if (mdvx.readFromBuffer(buf)) {
    _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
    _errStr += mdvx.getErrStr();
    return -1;
  }

  if (_debug) {
    cerr << "Found MDVP_SINGLE_BUFFER_PART, len: "
         << part->getLength()<< endl;
  }

  // get file headers if they are present

  if (_use32BitHeaders) {

    // 32-bit

    if (getPartByType(MDVP_FIELD_HEADER_FILE_FIELD_PART_32) != NULL &&
        getPartByType(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32) != NULL) {
      int n_fields = mdvx._mhdr.n_fields;
      for (int ifield = 0; ifield < n_fields; ifield++) {
        MdvxField *field = mdvx.getField(ifield);
        if (field == NULL) {
          _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
          TaStr::AddInt(_errStr, "  Cannot find field number: ", ifield);
          return -1;
        }
        field->_fhdrFile = new Mdvx::field_header_t;
        if (_getFieldHeader(*field->_fhdrFile, ifield,
                            MDVP_FIELD_HEADER_FILE_FIELD_PART_32)) {
          _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
          TaStr::AddInt(_errStr,
                        "  Cannot find file field header part for fld number: ",
                        ifield);
          return -1;
        }
        field->_vhdrFile = new Mdvx::vlevel_header_t;
        if (_getVlevelHeader(*field->_vhdrFile, ifield,
                             MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32)) {
          _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
          TaStr::AddInt(_errStr,
                        "  Cannot find file vlevel header part for fld number: ",
                        ifield);
          return -1;
        }
      } // ii
    }

  } else {

    // 64-bit

    if (getPartByType(MDVP_FIELD_HEADER_FILE_FIELD_PART_64) != NULL &&
        getPartByType(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64) != NULL) {
      int n_fields = mdvx._mhdr.n_fields;
      for (int ifield = 0; ifield < n_fields; ifield++) {
        MdvxField *field = mdvx.getField(ifield);
        if (field == NULL) {
          _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
          TaStr::AddInt(_errStr, "  Cannot find field number: ", ifield);
          return -1;
        }
        field->_fhdrFile = new Mdvx::field_header_t;
        if (_getFieldHeader(*field->_fhdrFile, ifield,
                            MDVP_FIELD_HEADER_FILE_FIELD_PART_64)) {
          _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
          TaStr::AddInt(_errStr,
                        "  Cannot find file field header part for fld number: ",
                        ifield);
          return -1;
        }
        field->_vhdrFile = new Mdvx::vlevel_header_t;
        if (_getVlevelHeader(*field->_vhdrFile, ifield,
                             MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64)) {
          _errStr += "ERROR - DsMdvxMsg::_getSingleBuffer.\n";
          TaStr::AddInt(_errStr,
                        "  Cannot find file vlevel header part for fld number: ",
                        ifield);
          return -1;
        }
      } // ii
    }

  }

  return 0;

}

////////////////////
// get xml parts

int DsMdvxMsg::_getXmlHdrAndBuf(DsMdvx &mdvx)
{

  DsMsgPart *part = getPartByType(MDVP_XML_HEADER_PART);
  if (part == NULL) {
    return -1;
  }

  // XML part must be non-zero in length

  if (part->getLength() <= 0) {
    _errStr += "ERROR - DsMdvxMsg::_getXmlHdrAndBuf\n";
    _errStr += "  Zero-length XML HDR part.\n";
    return -1;
  }
  mdvx._xmlHdr = _part2Str(part);
  if (_debug) {
    cerr << "Found MDVP_XML_HEADER_PART" << endl;
  }
  
  part = getPartByType(MDVP_XML_BUFFER_PART);
  if (part == NULL) {
    return -1;
  }
  mdvx._xmlBuf.free();
  mdvx._xmlBuf.add(part->getBuf(), part->getLength());

  return 0;

}

////////////////////////
// get no files on read

void DsMdvxMsg::_getNoFilesFoundOnRead(DsMdvx &mdvx)
{
  DsMsgPart *part = getPartByType(MDVP_NO_FILES_FOUND_ON_READ_PART);
  if (part == NULL) {
    mdvx._noFilesFoundOnRead = false;
  } else {
    mdvx._noFilesFoundOnRead = true;
  }
}

////////////////////
// get write options

int DsMdvxMsg::_getWriteOptions(DsMdvx &mdvx)
{

  DsMsgPart *part = getPartByType(MDVP_WRITE_OPTIONS_PART);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getWriteOptions\n";
    _errStr += "  Cannot find write options part.\n";
    return -1;
  }
  write_options_t options;
  // part must be big enough.
  if (part->getLength() != (sizeof(options))) {
    _errStr += "ERROR - DsMdvxMsg::_getWriteOptions.\n";
    _errStr += "  Encoding part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(options));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  memcpy(&options, part->getBuf(), sizeof(options));
  BE_to_array_32(&options, sizeof(options));
  if (_debug) {
    _print_write_options(options, cerr);
  }
  if (options.write_as_forecast) {
    mdvx.setWriteAsForecast();
  } else {
    mdvx.clearWriteAsForecast();
  }
  if (options.write_ldata_info) {
    mdvx.setWriteLdataInfo();
  } else {
    mdvx.clearWriteLdataInfo();
  }
  if (options.write_using_extended_path) {
    mdvx.setWriteUsingExtendedPath();
  } else {
    mdvx.clearWriteUsingExtendedPath();
  }
  if (options.if_forecast_write_as_forecast) {
    mdvx.setIfForecastWriteAsForecast();
  } else {
    mdvx.clearIfForecastWriteAsForecast();
  }
  return 0;
}

///////////////////////////////////////////////
// get the url for writing
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getWriteUrl(DsMdvx &mdvx)
  
{
  // get URL
  DsMsgPart *part = getPartByType(DS_URL);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getWriteUrl.\n";
    _errStr += "  Cannot find URL part.\n";
    return -1;
  }
  // String in part must be non-zero in length
  if (part->getLength() <= 0) {
    _errStr += "ERROR - DsMdvxMsg::_getWriteUrl.\n";
    _errStr += "  Zero-length URL.\n";
    return -1;
  }
  string outputUrl(_part2Str(part));
  if (_debug) {
    cerr << "Found output URL part: " << outputUrl << endl;
  }
  if (_subType == MDVP_WRITE_TO_DIR || _subType == MDVP_WRITE_TO_PATH) {
    mdvx._outputUrl = outputUrl;
  } 
  return 0;
}

////////////////////////
// get time list options

int DsMdvxMsg::_getTimeListOptions(DsMdvx &mdvx)
{

  // get URL
  DsMsgPart *part = getPartByType(DS_URL);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getTimeListOptions.\n";
    _errStr += "  Cannot find URL part.\n";
    return -1;
  }
  // String in part must be non-zero in length
  if (part->getLength() <= 0) {
    _errStr += "ERROR - DsMdvxMsg::_getTimeListOptions.\n";
    _errStr += "  Zero-length URL.\n";
    return -1;
  }
  string urlStr(_part2Str(part));
  DsURL url(urlStr);
  string dirStr = url.getFile();

  time_list_options_t options;

  if (_use32BitHeaders) {

    // 32 bit

    DsMsgPart *part32 = getPartByType(MDVP_TIME_LIST_OPTIONS_PART_32);
    if (part32 == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getTimeListOptions.\n";
      _errStr += "  No MDVP_TIME_LIST_OPTIONS_PART_32 found.\n";
      return -1;
    }
    time_list_options_32_t options32;
    // part must be big enough.
    if (part32->getLength() != sizeof(options32)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
      _errStr += "  MDVP_TIME_LIST_OPTIONS_PART_32 is wrong size.\n";
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(options32));
      TaStr::AddInt(_errStr, "  Size found in message: ", part32->getLength());
      return -1;
    }
    memcpy(&options32, part32->getBuf(), sizeof(options32));
    // byte swap
    BE_to_array_32(&options32, sizeof(options32));
    // convert to 64-bit
    _copyTimeListOptions32to64(options32, options);

  } else {

    // 64 bit
    
    DsMsgPart *part64 = getPartByType(MDVP_TIME_LIST_OPTIONS_PART_64);
    if (part64 == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getReadSearch.\n";
      _errStr += "  no MDVP_TIME_LIST_OPTIONS_PART_64 found.\n";
      return -1;
    }

    // part must be big enough.
    if (part64->getLength() != (sizeof(options))) {
      _errStr += "ERROR - DsMdvxMsg::_getTimeListOptions.\n";
      _errStr += "  MDVP_TIME_LIST_OPTIONS_PART_64 is wrong size.\n";
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(options));
      TaStr::AddInt(_errStr, "  Size found in message: ", part64->getLength());
      return -1;
    }

    memcpy(&options, part64->getBuf(), sizeof(options));
    BE_to_array_64(&options, sizeof(options));
    
  }

  if (_debug) {
    _print_time_list_options(options, cerr);
  }

  mdvx.clearTimeListMode();
  
  if (options.mode == MdvxTimeList::MODE_VALID) {
    mdvx.setTimeListModeValid(dirStr, options.start_time, options.end_time);
  } else if (options.mode == MdvxTimeList::MODE_GENERATE) {
    mdvx.setTimeListModeGen(dirStr, options.start_time, options.end_time);
  } else if (options.mode == MdvxTimeList::MODE_FORECAST) {
    mdvx.setTimeListModeForecast(dirStr, options.gen_time);
  } else if (options.mode == MdvxTimeList::MODE_GEN_PLUS_FCASTS) {
    mdvx.setTimeListModeGenPlusForecasts(dirStr,
					 options.start_time,
					 options.end_time);
  } else if (options.mode == MdvxTimeList::MODE_VALID_MULT_GEN) {
    mdvx.setTimeListModeValidMultGen(dirStr,
				     options.start_time, options.end_time);
  } else if (options.mode == MdvxTimeList::MODE_FIRST) {
    mdvx.setTimeListModeFirst(dirStr);
  } else if (options.mode == MdvxTimeList::MODE_LAST) {
    mdvx.setTimeListModeLast(dirStr);
  } else if (options.mode == MdvxTimeList::MODE_CLOSEST) {
    mdvx.setTimeListModeClosest(dirStr,
				options.search_time, options.time_margin);
  } else if (options.mode == MdvxTimeList::MODE_FIRST_BEFORE) {
    mdvx.setTimeListModeFirstBefore(dirStr,
				    options.search_time, options.time_margin);
  } else if (options.mode == MdvxTimeList::MODE_FIRST_AFTER) {
    mdvx.setTimeListModeFirstAfter(dirStr,
				   options.search_time, options.time_margin);
  } else if (options.mode == MdvxTimeList::MODE_BEST_FCAST) {
    mdvx.setTimeListModeBestForecast(dirStr,
				     options.search_time, options.time_margin);
  } else if (options.mode == MdvxTimeList::MODE_SPECIFIED_FCAST) {
    mdvx.setTimeListModeSpecifiedForecast(dirStr,
					  options.gen_time,
					  options.search_time,
					  options.time_margin);
  }

  // horizontal limits
  
  if (getPartByType(MDVP_READ_HORIZ_LIMITS_PART) != NULL) {
    if (_getReadHorizLimits(mdvx)) {
      _errStr += "ERROR - DsMdvxMsg::_getTimeListOptions.\n";
      return -1;
    }
  }

  return 0;

}

////////////////////////////
// get constrain lead times

int DsMdvxMsg::_getConstrainLeadTimes(DsMdvx &mdvx)
{

  DsMsgPart *part = getPartByType(MDVP_CONSTRAIN_LEAD_TIMES_PART);
  if (part == NULL) {
    return 0;
  }

  constrain_lead_times_t constrain;
  if (part->getLength() != sizeof(constrain)) {
    _errStr += "ERROR - DsMdvxMsg::_getConstrainLeadTimes.\n";
    _errStr += "  Constrain part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(constrain));
    TaStr::AddInt(_errStr, "  Size found in message: ",
		  part->getLength());
    return -1;
  }

  memcpy(&constrain, part->getBuf(), sizeof(constrain));
  BE_to_array_32(&constrain, sizeof(constrain));
  mdvx.setConstrainFcastLeadTimes(constrain.min_lead_time,
				  constrain.max_lead_time,
				  constrain.specify_by_gen_time);

  return 0;

}
  
////////////
// get field

int DsMdvxMsg::_getField(Mdvx &mdvx, int field_num)

{

  // get the headers

  Mdvx::field_header_t fhdr;
  if (_use32BitHeaders) {
    if (_getFieldHeader(fhdr, field_num, MDVP_FIELD_HEADER_PART_32)) {
      _errStr += "ERROR - DsMdvxMsg::_getField.\n";
      TaStr::AddInt(_errStr,
                    "  Cannot find 32-bit field header part for field number: ",
                    field_num);
      return -1;
    }
  } else {
    if (_getFieldHeader(fhdr, field_num, MDVP_FIELD_HEADER_PART_64)) {
      _errStr += "ERROR - DsMdvxMsg::_getField.\n";
      TaStr::AddInt(_errStr,
                    "  Cannot find 64-bit field header part for field number: ",
                    field_num);
      return -1;
    }
  }

  Mdvx::vlevel_header_t vhdr;
  if (_use32BitHeaders) {
    if (_getVlevelHeader(vhdr, field_num, MDVP_VLEVEL_HEADER_PART_32)) {
      _errStr += "ERROR - DsMdvxMsg::_getField.\n";
      TaStr::AddInt(_errStr,
                    "  Cannot find 32-bit vlevel header part for field number: ",
                    field_num);
      return -1;
    }
  } else {
    if (_getVlevelHeader(vhdr, field_num, MDVP_VLEVEL_HEADER_PART_64)) {
      _errStr += "ERROR - DsMdvxMsg::_getField.\n";
      TaStr::AddInt(_errStr,
                    "  Cannot find 64-bit vlevel header part for field number: ",
                    field_num);
      return -1;
    }
  }
    
  // get the data

  DsMsgPart * dataPart = getPartByType(MDVP_FIELD_DATA_PART, field_num);
  if (dataPart == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getField.\n";
    _errStr += "  Cannot find field data part.\n";
    TaStr::AddInt(_errStr, "  Field num: ", field_num);
    return -1;
  }

  // check the size

  if (dataPart->getLength() != fhdr.volume_size) {
    _errStr += "ERROR - DsMdvxMsg::_getField.\n";
    _errStr += "  Field data part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Field num: ", field_num);
    TaStr::AddInt(_errStr, "  Size expected: ", fhdr.volume_size);
    TaStr::AddInt(_errStr, "  Size found in message: ", dataPart->getLength());
    return -1;
  }

  if (_debug) {
    cerr << "Found field data, len: " << dataPart->getLength() << endl;
  }

  // create a temporary buffer, add data and byte-swap if not compressed

  MemBuf tmpbuf;
  tmpbuf.add(dataPart->getBuf(), dataPart->getLength());
  MdvxField::_data_from_BE(fhdr, tmpbuf.getPtr(), tmpbuf.getLen());

  // create new field, add to mdvx object
  
  MdvxField *field = new MdvxField(fhdr, vhdr, tmpbuf.getPtr(), false, false);
  mdvx.addField(field);

  // get file headers if appropriate

  if (_use32BitHeaders) {

    // 32-bit

    if (getPartByType(MDVP_FIELD_HEADER_FILE_FIELD_PART_32) != NULL) {
      field->_fhdrFile = new Mdvx::field_header_t;
      if (_getFieldHeader(*field->_fhdrFile, field_num,
                          MDVP_FIELD_HEADER_FILE_FIELD_PART_32)) {
        _errStr += "ERROR - DsMdvxMsg::_getField.\n";
        TaStr::AddInt(_errStr,
                      "  Cannot find 32-bit file field header part for field number: ",
                      field_num);
        return -1;
      }
    }
    
    if (getPartByType(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32) != NULL) {
      field->_vhdrFile = new Mdvx::vlevel_header_t;
      if (_getVlevelHeader(*field->_vhdrFile, field_num,
                           MDVP_VLEVEL_HEADER_FILE_FIELD_PART_32)) {
        _errStr += "ERROR - DsMdvxMsg::_getField.\n";
        TaStr::AddInt(_errStr,
                      "  Cannot find 32-bit file vlevel header part for field number: ",
                      field_num);
        return -1;
      }
    }

  } else {

    // 64-bit

    if (getPartByType(MDVP_FIELD_HEADER_FILE_FIELD_PART_64) != NULL) {
      field->_fhdrFile = new Mdvx::field_header_t;
      if (_getFieldHeader(*field->_fhdrFile, field_num,
                          MDVP_FIELD_HEADER_FILE_FIELD_PART_64)) {
        _errStr += "ERROR - DsMdvxMsg::_getField.\n";
        TaStr::AddInt(_errStr,
                      "  Cannot find 64-bit file field header part for field number: ",
                      field_num);
        return -1;
      }
    }
    
    if (getPartByType(MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64) != NULL) {
      field->_vhdrFile = new Mdvx::vlevel_header_t;
      if (_getVlevelHeader(*field->_vhdrFile, field_num,
                           MDVP_VLEVEL_HEADER_FILE_FIELD_PART_64)) {
        _errStr += "ERROR - DsMdvxMsg::_getField.\n";
        TaStr::AddInt(_errStr,
                      "  Cannot find 64-bit file vlevel header part for field number: ",
                      field_num);
        return -1;
      }
    }

  }

    
  return 0;

}

////////////
// get chunk

int DsMdvxMsg::_getChunk(Mdvx &mdvx, int chunk_num)

{

  // get the header

  Mdvx::chunk_header_t chdr;
  if (_use32BitHeaders) {
    // 32-bit
    if (_getChunkHeader(chdr, chunk_num, MDVP_CHUNK_HEADER_PART_32)) {
      _errStr += "ERROR - DsMdvxMsg::_getChunk.\n";
      TaStr::AddInt(_errStr,
                    "  Cannot find 32-bit chunk header part for chunk number: ",
                    chunk_num);
      return -1;
    }
  } else {
    // 64-bit
    if (_getChunkHeader(chdr, chunk_num, MDVP_CHUNK_HEADER_PART_64)) {
      _errStr += "ERROR - DsMdvxMsg::_getChunk.\n";
      TaStr::AddInt(_errStr,
                    "  Cannot find 64-bit chunk header part for chunk number: ",
                    chunk_num);
      return -1;
    }
  }

  // get the data

  DsMsgPart * dataPart = getPartByType(MDVP_CHUNK_DATA_PART, chunk_num);
  if (dataPart == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getChunk.\n";
    _errStr += "  Cannot find chunk data part.\n";
    TaStr::AddInt(_errStr, "  Chunk num: ", chunk_num);
    return -1;
  }

  // check the size

  if (dataPart->getLength() != chdr.size) {
    _errStr += "ERROR - DsMdvxMsg::_getChunk.\n";
    _errStr += "  Chunk data part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Chunk num: ", chunk_num);
    TaStr::AddInt(_errStr, "  Size expected: ", chdr.size);
    TaStr::AddInt(_errStr, "  Size found in message: ", dataPart->getLength());
    return -1;
  }

  if (_debug) {
    cerr << "Found chunk data, len: " << dataPart->getLength() << endl;
  }

  // create new chunk, add to mdvx object
  
  MdvxChunk *chunk = new MdvxChunk(chdr, dataPart->getBuf());
  mdvx.addChunk(chunk);
  return 0;

}

////////////////////////
// get vsection segments

int DsMdvxMsg::_getVsectSegments(DsMdvx &mdvx)
{

  vector<Mdvx::vsect_segment_t> segments;
  double totalLength = 0;

  // try 64-bit version

  DsMsgPart *part64 = getPartByType(MDVP_VSECT_SEGMENTS_PART_64);
  if (part64 != NULL) {

    MemBuf buf64;
    buf64.add(part64->getBuf(), part64->getLength());
    string errStr;
    if (Mdvx::disassembleVsectSegmentsBuf64(buf64, segments, totalLength, errStr)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectSegments.\n";
      _errStr += errStr;
      return -1;
    }
    if (_debug) {
      Mdvx::printVsectSegmentsBuf64(buf64, cerr);
    }

  } else {
  
    // try 32-bit version
    
    DsMsgPart *part32 = getPartByType(MDVP_VSECT_SEGMENTS_PART_32);
    if (part32 == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getVsectSegments.\n";
      _errStr += "  Cannot find segments part.\n";
      return -1;
    }
    MemBuf buf32;
    buf32.add(part32->getBuf(), part32->getLength());
    string errStr;
    if (Mdvx::disassembleVsectSegmentsBuf32(buf32, segments, totalLength, errStr)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectSegments.\n";
      _errStr += errStr;
      return -1;
    }
    if (_debug) {
      Mdvx::printVsectSegmentsBuf32(buf32, cerr);
    }

  } // if (part64 ...
  
  mdvx._vsectTotalLength = totalLength;
  mdvx._vsectSegments.clear();
  for (int ii = 0; ii < (int) segments.size(); ii++) {
    const Mdvx::vsect_segment_t &seg = segments[ii];
    mdvx._vsectSegments.push_back(seg);
  }
  return 0;

}

/////////////////////////////
// get vsection sample points

int DsMdvxMsg::_getVsectSamplepts(DsMdvx &mdvx)
  
{

  vector<Mdvx::vsect_samplept_t> samplePts;
  double dxKm = 0.0;

  // try 64 bit
  
  DsMsgPart *part64 = getPartByType(MDVP_VSECT_SAMPLE_PTS_PART_64);
  if (part64 != NULL) {

    MemBuf buf64;
    buf64.add(part64->getBuf(), part64->getLength());
    string errStr;
    if (Mdvx::disassembleVsectSamplePtsBuf64(buf64, samplePts, dxKm, errStr)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectSamplepts.\n";
      _errStr += errStr;
      return -1;
    }
    if (_debug) {
      Mdvx::printVsectSamplePtsBuf64(buf64, cerr);
    }

  } else {

    // try 32 bit
    
    DsMsgPart *part32 = getPartByType(MDVP_VSECT_SAMPLE_PTS_PART_32);
    if (part32 == NULL) {
      _errStr += "ERROR - DsMdvxMsg::_getVsectSamplepts.\n";
      _errStr += "  Cannot find samplepts part.\n";
      return -1;
    }
    MemBuf buf32;
    buf32.add(part32->getBuf(), part32->getLength());
    string errStr;
    if (Mdvx::disassembleVsectSamplePtsBuf32(buf32, samplePts, dxKm, errStr)) {
      _errStr += "ERROR - DsMdvxMsg::_getReadVsectSamplepts.\n";
      _errStr += errStr;
      return -1;
    }
    if (_debug) {
      Mdvx::printVsectSamplePtsBuf32(buf32, cerr);
    }

  } // if (part64 ...

  mdvx._vsectDxKm = dxKm;
  mdvx._vsectSamplePts.clear();
  for (int ii = 0; ii < (int) samplePts.size(); ii++) {
    const Mdvx::vsect_samplept_t &pt = samplePts[ii];
    mdvx._vsectSamplePts.push_back(pt);
  }
  return 0;
}
  
//////////////////////////////
// get vsection disable interp

int DsMdvxMsg::_getReadVsectInterpDisabled(DsMdvx &mdvx)
  
{
  if (getPartByType(MDVP_READ_VSECT_DISABLE_INTERP_PART) != NULL) {
    mdvx.setReadVsectDisableInterp();
  }
  return 0;
}

///////////////////
// get time lists

int DsMdvxMsg::_getTimeLists(DsMdvx &mdvx)
{

  // clear time list data
  
  mdvx._timeList.clearData();

  if (_getValidTimes(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getTimeLists\n";
    return -1;
  }

  if (_getGenTimes(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getTimeLists\n";
    return -1;
  }

  if (_getForecastTimes(mdvx)) {
    _errStr += "ERROR - DsMdvxMsg::_getTimeLists\n";
    return -1;
  }

  return 0;

}
  
///////////////////
// get valid times

int DsMdvxMsg::_getValidTimes(DsMdvx &mdvx)
{
  
  vector<time_t> validTimes;
  bool hasForecasts;
  if (_loadTimeList(mdvx, MDVP_VALID_TIMES_PART, 0,
		    hasForecasts, validTimes)) {
    _errStr += "ERROR - DsMdvxMsg::_getValidTimes\n";
    return -1;
  }
  
  mdvx._timeList.setHasForecasts(hasForecasts);
  for (size_t ii = 0; ii < validTimes.size(); ii++) {
    mdvx._timeList.addValidTime(validTimes[ii]);
  }

  return 0;

}

///////////////////
// get gen times

int DsMdvxMsg::_getGenTimes(DsMdvx &mdvx)
{
  
  vector<time_t> genTimes;
  bool hasForecasts;
  if (_loadTimeList(mdvx, MDVP_GEN_TIMES_PART, 0,
		    hasForecasts, genTimes)) {
    _errStr += "ERROR - DsMdvxMsg::_getGenTimes\n";
    return -1;
  }
  
  mdvx._timeList.setHasForecasts(hasForecasts);
  for (size_t ii = 0; ii < genTimes.size(); ii++) {
    mdvx._timeList.addGenTime(genTimes[ii]);
  }

  return 0;

}

/////////////////
// get time list

int DsMdvxMsg::_getForecastTimes(DsMdvx &mdvx)

{

  int nGenTimes = partExists(MDVP_FORECAST_TIMES_PART);
  for (int igen = 0; igen < nGenTimes; igen++) {
    vector<time_t> forecastTimes;
    bool hasForecasts;
    if (_loadTimeList(mdvx, MDVP_FORECAST_TIMES_PART, igen,
		      hasForecasts, forecastTimes)) {
      _errStr += "ERROR - DsMdvxMsg::_getForecastTimes\n";
      return -1;
    }
    mdvx._timeList.addForecastTimes(forecastTimes);
  } // igen

  return 0;

}

///////////////////
// load time list

int DsMdvxMsg::_loadTimeList(DsMdvx &mdvx,
			     msg_part_t partId,
			     ssize_t partIndex,
			     bool &has_forecasts,
			     vector<time_t> &timeList)
{

  timeList.clear();
  
  DsMsgPart *part = getPartByType(partId, partIndex);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_loadTimeList\n";
    TaStr::AddInt(_errStr, "  Cannot find part ID: ", partId);
    return -1;
  }
  
  MemBuf tbuf;
  tbuf.add(part->getBuf(), part->getLength());
  BE_to_array_32(tbuf.getPtr(), tbuf.getLen());

  size_t bufLen = part->getLength();
  char *bptr = (char *) part->getBuf();
  if (bufLen < sizeof(time_list_hdr_t)) {
    _errStr += "ERROR - DsMdvxMsg::_loadTimeList.\n";
    _errStr += "  Time list part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected at least: ",
		  sizeof(time_list_hdr_t));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  BE_to_array_32(bptr, bufLen);
  
  time_list_hdr_t thdr;
  memcpy(&thdr, bptr, sizeof(time_list_hdr_t));
  bptr += sizeof(time_list_hdr_t);
  size_t sizeExpected =
    sizeof(time_list_hdr_t) + thdr.ntimes * sizeof(ti32);
  if (bufLen < sizeExpected) {
    _errStr += "ERROR - DsMdvxMsg::_loadTimeList.\n";
    _errStr += "  Time list part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeExpected);
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }
  has_forecasts = (bool) thdr.has_forecasts;
  ti32 *times = (ti32 *) bptr;
  for (int i = 0; i < thdr.ntimes; i++) {
    timeList.push_back((time_t) times[i]);
  }
  
  return 0;

}

/////////////////////////////
// get climatology statistic type

int DsMdvxMsg::_getClimoStatTypes(DsMdvx &mdvx)
{
  // Initialize the calc climo flag.  We will only calculate
  // climatologies if we get a good stat type part.

  mdvx.clearCalcClimo();
  
  DsMsgPart * part = getPartByType(MDVP_CLIMO_STATISTIC_TYPE_PART);
  if (part == NULL) {
    return -1;
  }

  // Extract the header from the part

  climoTypePartHdr_t climo_hdr;

  if (part->getLength() < (int)sizeof(climo_hdr)) {
    _errStr += "ERROR - DsMdvxMsg::_getClimoStatType.\n";
    _errStr += "  Climo stat type part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size of header: ", sizeof(climo_hdr));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&climo_hdr, part->getBuf(), sizeof(climo_hdr));
  BE_to_array_32(&climo_hdr, sizeof(climo_hdr));

  // Now extract all of the stat specifiers

  int expected_size = sizeof(climo_hdr) +
    (climo_hdr.num_stats * sizeof(climoTypePart_t));
  
  if (part->getLength() != expected_size) {
    _errStr += "ERROR - DsMdvxMsg::_getClimoStatType.\n";
    _errStr += "  Climo stat type part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", expected_size);
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  climoTypePart_t *climo_stats = new climoTypePart_t[climo_hdr.num_stats];

  memcpy(climo_stats, part->getBuf() + sizeof(climo_hdr),
	 climo_hdr.num_stats * sizeof(climoTypePart_t));
  
  for (int i = 0; i < climo_hdr.num_stats; ++i)
    BE_to_array_32(&(climo_stats[i]), sizeof(climoTypePart_t));
    
  if (_debug)
    _print_climo_stat_type(climo_hdr, climo_stats, cerr);
  
  mdvx.setCalcClimo();
  for (int i = 0; i < climo_hdr.num_stats; ++i)
    mdvx.addClimoStatType((Mdvx::climo_type_t)climo_stats[i].climo_type,
			  climo_stats[i].divide_by_num_obs,
			  climo_stats[i].params[0],
			  climo_stats[i].params[1]);

  delete [] climo_stats;
  
  return 0;

}

/////////////////////////////
// get climatology date range

int DsMdvxMsg::_getClimoDataRange(DsMdvx &mdvx)
{

  climoDataRange_t data_range;

  if (_use32BitHeaders) {

    // 32 bit
    
    DsMsgPart * part = getPartByType(MDVP_CLIMO_DATA_RANGE_PART_32);
    if (part == NULL) {
      return -1;
    }
    
    climoDataRange_32_t data_range32;
  
    // part must be big enough.
    if (part->getLength() != sizeof(data_range32)) {
      _errStr += "ERROR - DsMdvxMsg::_getClimoDateRange32.\n";
      _errStr += "  Climo date range part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(data_range32));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }

    memcpy(&data_range32, part->getBuf(), sizeof(data_range32));
    BE_to_array_32(&data_range32, sizeof(data_range32));
    
    // convert to 64 bit header
    
    climoDataRange_t data_range;
    _copyClimoDataRange32to64(data_range32, data_range);

  } else {

    // 64 bit

    DsMsgPart * part = getPartByType(MDVP_CLIMO_DATA_RANGE_PART_64);
    if (part == NULL) {
      return -1;
    }
    
    // part must be big enough.
    if (part->getLength() != sizeof(data_range)) {
      _errStr += "ERROR - DsMdvxMsg::_getClimoDateRange.\n";
      _errStr += "  Climo date range part is incorrect size.\n";
      TaStr::AddInt(_errStr, "  Size expected: ", sizeof(data_range));
      TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
      return -1;
    }

    memcpy(&data_range, part->getBuf(), sizeof(data_range));
    BE_to_array_64(&data_range, sizeof(data_range));

  }

  if (_debug) {
    _print_climo_data_range(data_range, cerr);
  }
  
  mdvx.setClimoDataRange(data_range.start_time,
			 data_range.end_time);

  return 0;

}

/////////////////////////////
// get climatology time range

int DsMdvxMsg::_getClimoTimeRange(DsMdvx &mdvx)
{

  DsMsgPart * part = getPartByType(MDVP_CLIMO_TIME_RANGE_PART);
  if (part == NULL) {
    return -1;
  }

  climoTimeRange_t time_range;

  // part must be big enough.
  if (part->getLength() != sizeof(time_range)) {
    _errStr += "ERROR - DsMdvxMsg::_getClimoTimeRange.\n";
    _errStr += "  Climo time range part is incorrect size.\n";
    TaStr::AddInt(_errStr, "  Size expected: ", sizeof(time_range));
    TaStr::AddInt(_errStr, "  Size found in message: ", part->getLength());
    return -1;
  }

  memcpy(&time_range, part->getBuf(), sizeof(time_range));
  BE_to_array_32(&time_range, sizeof(time_range));
  if (_debug) {
    _print_climo_time_range(time_range, cerr);
  }
  
  mdvx.setClimoTimeRange(time_range.start_hour,
			 time_range.start_minute,
			 time_range.start_second,
			 time_range.end_hour,
			 time_range.end_minute,
			 time_range.end_second);

  return 0;

}

///////////////////////////////////////////////
// get the app name
//
// returns 0 on success, -1 on failure

int DsMdvxMsg::_getAppName(DsMdvx &mdvx)

{
  // get path in use
  DsMsgPart *part = getPartByType(MDVP_APP_NAME_PART);
  if (part == NULL) {
    _errStr += "ERROR - DsMdvxMsg::_getAppName.\n";
    _errStr += "  Cannot find appName part.\n";
    return -1;
  }
  // String in part must be non-zero in length
  if (part->getLength() <= 0) {
    _errStr += "ERROR - DsMdvxMsg::_getAppName.\n";
    _errStr += "  Zero-length appName part.\n";
    return -1;
  }
  mdvx._appName = _part2Str(part);
  if (_debug) {
    cerr << "Found MDVP_APP_NAME_PART: " << mdvx._appName << endl;
  }
  return 0;
}

/////////////////////////////
// get netcdf header parts

int DsMdvxMsg::_getNcfHeaderParts(DsMdvx &mdvx)
{

  // Extract the information from the message
  
  DsMsgPart *hdrPart = getPartByType(MDVP_NCF_HEADER_PART);
  if (hdrPart == NULL) {
    return -1;
  }
  
  string xml(_part2Str(hdrPart));
  if (_debug) {
    cerr << "Found MDVP_NCF_HEADER_PART" << endl;
    cerr << xml << endl;
  }

  double validTime = 0;
  if (TaXml::readDouble(xml, "valid_time", validTime)) {
    validTime = 0;
  }
  double genTime = 0;
  if (TaXml::readDouble(xml, "gen_time", genTime)) {
    genTime = 0;
  }
  double forecastTime = 0;
  if (TaXml::readDouble(xml, "forecast_time", forecastTime)) {
    forecastTime = 0;
  }

  int forecastLeadSecs = 0;
  if (TaXml::readInt(xml, "forecast_lead_secs", forecastLeadSecs)) {
    forecastLeadSecs = 0;
  }
  int epoch = 0;
  if (TaXml::readInt(xml, "epoch", epoch)) {
    epoch = 0;
  }

  bool isForecast = false;
  if (TaXml::readBoolean(xml, "is_forecast", isForecast)) {
    isForecast = false;
  }

  string suffix;
  if (TaXml::readString(xml, "suffix", suffix)) {
    suffix.clear();
  }

  // set ncf details
  
  mdvx.setNcfHeader((time_t) validTime,
		    isForecast,
		    forecastLeadSecs,
		    epoch);

  mdvx.setNcfFileSuffix(suffix);

  return 0;

}

/////////////////////////////
// get netcdf parts

int DsMdvxMsg::_getNcfParts(DsMdvx &mdvx)
{

  // first get header
  
  if (_getNcfHeaderParts(mdvx))
    return -1;
  
  // then get buffer

  DsMsgPart *bufPart = getPartByType(MDVP_NCF_BUFFER_PART);
  if (bufPart == NULL) {
    return -1;
  }

  if (_debug) {
    cerr << "Found MDVP_NCF_BUFFER_PART, len: "
         << bufPart->getLength()<< endl;
  }

  // set ncf details
  
  mdvx.setNcfBuffer(bufPart->getBuf(),
		    bufPart->getLength());

  return 0;

}

////////////////////////////////////
// get convert Mdv2Ncf specs (in XML)

int DsMdvxMsg::_getConvertMdv2Ncf(DsMdvx &mdvx)

{

  DsMsgPart *part = getPartByType(MDVP_CONVERT_MDV_TO_NCF_PART);
  if (part == NULL) {
    return -1;
  }

  string xml(_part2Str(part));
  
  if (_debug) {
    cerr << "Found MDVP_MDV_TO_NCF_PART" << endl;
    cerr << xml << endl;
  }

  mdvx.clearMdv2Ncf();

  // global attributes

  string institution, references, comment;
  if (TaXml::readString(xml, "institution", institution)) {
    institution.clear();
  }
  if (TaXml::readString(xml, "references", references)) {
    references.clear();
  }
  if (TaXml::readString(xml, "comment", comment)) {
    comment.clear();
  }
  mdvx.setMdv2NcfAttr(institution, references, comment);

  // output compression

  bool compress = false;
  if (TaXml::readBoolean(xml, "compress", compress)) {
    compress = false;
  }
  int compressionLevel = 4;
  if (TaXml::readInt(xml, "compressionLevel", compressionLevel)) {
    compressionLevel = 4;
  }
  mdvx.setMdv2NcfCompression(compress, compressionLevel);
  
  // output format

  string formatStr;
  if (TaXml::readString(xml, "fileFormat", formatStr)) {
    formatStr.clear();
  }
  DsMdvx::nc_file_format_t fileFormat = DsMdvx::ncFormat2Enum(formatStr);
  mdvx.setMdv2NcfFormat(fileFormat);

  // polar radar file type

  string ftypeStr;
  if (TaXml::readString(xml, "radialFileType", ftypeStr)) {
    ftypeStr.clear();
  }
  DsMdvx::radial_file_type_t fileType = DsMdvx::radialFileType2Enum(ftypeStr);
  mdvx.setRadialFileType(fileType);

  // output content

  bool outputLatlonArrays = false;
  bool outputMdvAttr = false;
  bool outputMdvChunks = false;
  bool outputStartEndTimes = false;
  if (TaXml::readBoolean(xml, "outputLatlonArrays", outputLatlonArrays)) {
    outputLatlonArrays = false;
  }
  if (TaXml::readBoolean(xml, "outputMdvAttr", outputMdvAttr)) {
    outputMdvAttr = false;
  }
  if (TaXml::readBoolean(xml, "outputMdvChunks", outputMdvChunks)) {
    outputMdvChunks = false;
  }
  if (TaXml::readBoolean(xml, "outputStartEndTimes", outputStartEndTimes)) {
    outputStartEndTimes = false;
  }
  mdvx.setMdv2NcfOutput(outputLatlonArrays, outputMdvAttr,
                        outputMdvChunks,outputStartEndTimes);

  // field translation

  vector<string> transVec;
  if (TaXml::readStringArray(xml, "field-translation", transVec)) {
    // no field translations
    return 0;
  }

  for (int ii = 0; ii < (int) transVec.size(); ii++) {
    
    const string &trans = transVec[ii];
    
    string mdvFieldName;
    if (TaXml::readString(trans, "mdv_field_name", mdvFieldName)) {
      mdvFieldName.clear();
    }

    string ncfFieldName;
    if (TaXml::readString(trans, "ncf_field_name", ncfFieldName)) {
      ncfFieldName.clear();
    }

    string ncfStandardName;
    if (TaXml::readString(trans, "ncf_standard_name", ncfStandardName)) {
      ncfStandardName.clear();
    }

    string ncfLongName;
    if (TaXml::readString(trans, "ncf_long_name", ncfLongName)) {
      ncfLongName.clear();
    }

    string ncfUnits;
    if (TaXml::readString(trans, "ncf_units", ncfUnits)) {
      ncfUnits.clear();
    }
    
    bool doLinearTransform = false;
    if (TaXml::readBoolean(trans, "do_linear_transform", doLinearTransform)) {
      doLinearTransform = false;
    }

    double linearMult = 1.0;
    if (TaXml::readDouble(trans, "linear_mult", linearMult)) {
      linearMult = 1.0;
    }

    double linearOffset = 0.0;
    if (TaXml::readDouble(trans, "linear_offset", linearOffset)) {
      linearOffset = 0.0;
    }

    string packStr;
    if (TaXml::readString(trans, "packing", packStr)) {
      packStr.clear();
    }
    DsMdvx::ncf_pack_t packing = DsMdvx::ncfPack2Enum(packStr);
    
    mdvx.addMdv2NcfTrans(mdvFieldName,
                         ncfFieldName,
                         ncfStandardName,
                         ncfLongName,
                         ncfUnits,
                         doLinearTransform,
                         linearMult,
                         linearOffset,
                         packing);
    
  }

  return 0;

}

////////////////////////////////////
// interpret a part as a string
// make sure it is null terminated etc

string DsMdvxMsg::_part2Str(const DsMsgPart *part)

{
  
  if (part == NULL || part->getLength() < 1) {
    // return empty string
    string str;
    return str;
  }

  // load tmp buf from part

  MemBuf buf;
  buf.add(part->getBuf(), part->getLength());

  // add null termination

  char cnull = '\0';
  buf.add(&cnull, 1);
  
  string str((char *) part->getBuf());
  return str;

}
