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
//////////////////////////////////////////////////////////
// Mdvx_xml.cc
//
// XML routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2007
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaArray.hh>
#include <dataport/bigend.h>
#include <didss/LdataInfo.hh>
#include <didss/RapDataDir.hh>

#ifndef BOOL_STR
#define BOOL_STR(a) (a == FALSE ? "false" : "true")
#endif

#define NOT_SET 0

using namespace std;

/////////////////////////////////////////////////////////////////////////
// Write object to the XML header and buffer
//
// Take the contents of the Mdvx object and write it into a string for
// the XML header and a MemBuf for the data

void Mdvx::writeToXmlBuffer(string &hdr, MemBuf &buf,
                            const string &bufFileName /* = ""*/) const
  
{

  // free up the buffer

  buf.free();

  // add the fields to the buffer
  // updating the field headers with the length and offset
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    const MdvxField &field = *_fields[ifield];
    const field_header_t &fhdr = field.getFieldHeader();
    
    // if it is compressed, convert the compression type to GZIP_VOL
    // since that is the only compression supported in XML

    if (fhdr.compression_type != COMPRESSION_NONE &&
        fhdr.compression_type != COMPRESSION_GZIP_VOL) {
      field.compress(COMPRESSION_GZIP_VOL);
    }
    
    // set the number of bytes and offset
    // if compressed, the gzip data starts after the compression header
    
    int offset = 0;
    if (fhdr.compression_type == COMPRESSION_GZIP_VOL) {
      offset = sizeof(compress_buf_hdr_t);
    }
    int nBytesData = field.getVolLen() - offset;
    char *data = (char *) field.getVol() + offset;

    // update the field header with the offset and length
    // these are mutable, so do not violate const
    
    fhdr.field_data_offset = buf.getLen();
    fhdr.volume_size = nBytesData;
    
    // add to the data buffer

    buf.add(data, nBytesData);
    
  } // ifield
  
  // add the chunks to the buffer
  // updating the chunk headers with the offset
  
  for (size_t ichunk = 0; ichunk < _chunks.size(); ichunk++) {
    
    const MdvxChunk &chunk = *_chunks[ichunk];
    const chunk_header_t &chdr = chunk.getHeader();
    
    // update the chunk header with the offset
    // this is mutable, so does not violate const

    chdr.chunk_data_offset = buf.getLen();
    
    // add to the data buffer

    buf.add(chunk.getData(), chunk.getSize());
    
  } // ichunk
  
  // write the XML header

  _writeToXmlHdr(hdr, bufFileName);

}

/////////////////////////////////////////////////////////
// Write to path as XML
// File is written to files based on the specified path.
// Returns 0 on success, -1 on error.

int Mdvx::_writeAsXml(const string &output_path) const

{

  // free up

  _xmlHdr.clear();
  _xmlBuf.free();

  // compute path
  
  string outPathStr;
  RapDataDir.fillPath(output_path, outPathStr);

  string xmlPathStr = outPathStr + ".xml";
  string bufPathStr = outPathStr + ".buf";
  
  _pathInUse = xmlPathStr;

  if (_debug) {
    cerr << "Mdvx - writing to XML path: " << xmlPathStr << endl;
    cerr << "           and to BUF path: " << bufPathStr << endl;
  }

  // compute XML file name

  Path bufPath(bufPathStr);
  string bufName = bufPath.getFile();
  
  // write up the XML header and buffer
  
  writeToXmlBuffer(_xmlHdr, _xmlBuf, bufName);
  
  if (_debug) {
    cerr << "Writing XML output" << endl;
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
    cerr << _xmlHdr;
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
  }

  // remove compressed files if they exist
  
  ta_remove_compressed(xmlPathStr.c_str());
  ta_remove_compressed(bufPathStr.c_str());

  if (_writeBufferToFile(bufPathStr, _xmlBuf.getLen(), _xmlBuf.getPtr())) {
    cerr << "ERROR - Mdvx::_write_as_xml" << endl;
    return -1;
  }

  if (_writeBufferToFile(xmlPathStr, _xmlHdr.size(), _xmlHdr.c_str())) {
    cerr << "ERROR - Mdvx::_write_as_xml" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Write XML header

void Mdvx::_writeToXmlHdr(string &hdr,
                          const string &bufFileName) const
  
{

  hdr.clear();

  // preamble

  hdr += TaXml::writeString("<?xml version=\"1.0\" ?>\n");
  
  // opening <mdv> tag

  vector<TaXml::attribute> attrs;

  string schemaInstance = "http://www.w3.org/2001/XMLSchema-instance";
  string schemaLocation =
    "http://www.ral.ucar.edu/xml/schemas/mdv.1.0.xsd "
    "http://www.ral.ucar.edu/xml/schemas/mdv.1.0.xsd";
  string xmlVersion = "1.0";
  
  TaXml::addStringAttr("xmlns:xsi", schemaInstance, attrs);
  TaXml::addStringAttr("version", xmlVersion, attrs);
  TaXml::addStringAttr("xsi:schemaLocation", schemaLocation, attrs);
  
  hdr += TaXml::writeStartTag("mdv", 0, attrs, true);
  
  // buffer file name

  if (bufFileName.size() > 0) {
    hdr += TaXml::writeString("buf-file-name", 1, bufFileName);
  }

  _writeToXmlMasterHdr(hdr);

  // fields

  for (int ifield = 0; ifield < (int) _fields.size(); ifield++) {
    _writeToXmlFieldHdr(hdr, ifield);
  }

  // chunks

  for (int ichunk = 0; ichunk < (int) _chunks.size(); ichunk++) {
    _writeToXmlChunkHdr(hdr, ichunk);
  }

  // end tag
  
  hdr += TaXml::writeEndTag("mdv", 0);

}

/////////////////////////////////////////////////////////////////////////
// Write XML master header

void Mdvx::_writeToXmlMasterHdr(string &hdr) const
  
{

  hdr += TaXml::writeStartTag("master-header", 1);

  // times
  
  hdr += TaXml::writeTime("time-valid", 2, _mhdr.time_centroid);
  hdr += TaXml::writeTime("time-gen", 2, _mhdr.time_gen);
  time_t forecast_time = 0;
  int forecast_lead_secs = 0;
  if (_fields.size() > 0) {
    forecast_time = _fields[0]->getFieldHeader().forecast_time;
    forecast_lead_secs = _fields[0]->getFieldHeader().forecast_delta;
  }
  if (forecast_time != NOT_SET) {
    hdr += TaXml::writeInt("forecast-lead-secs", 2, forecast_lead_secs);
  }
  hdr += TaXml::writeTime("time-written", 2, _mhdr.time_written);
  if (_mhdr.user_time != NOT_SET) {
    hdr += TaXml::writeTime("time-user", 2, _mhdr.user_time);
  }
  if (_mhdr.time_begin != NOT_SET) {
    hdr += TaXml::writeTime("time-begin", 2, _mhdr.time_begin);
  }
  if (_mhdr.time_end != NOT_SET) {
    hdr += TaXml::writeTime("time-end", 2, _mhdr.time_end);
  }
  if (_mhdr.time_expire != NOT_SET) {
    hdr += TaXml::writeTime("time-expire", 2, _mhdr.time_expire);
  }

  // data set information

  hdr += TaXml::writeString("data-set-name", 2, _mhdr.data_set_name);
  hdr += TaXml::writeString("data-set-info", 2, getDataSetInfo());
  hdr += TaXml::writeString("data-set-source", 2, _mhdr.data_set_source);

  // sensor location if set
  
  if (_mhdr.sensor_lon != NOT_SET ||
      _mhdr.sensor_lat != NOT_SET ||
      _mhdr.sensor_alt != NOT_SET) {
    hdr += TaXml::writeDouble("sensor-lon", 2, _mhdr.sensor_lon, "%.8f");
    hdr += TaXml::writeDouble("sensor-lat", 2, _mhdr.sensor_lat, "%.8f");
    hdr += TaXml::writeDouble("sensor-alt", 2, _mhdr.sensor_alt, "%.8f");
  }
  
  hdr += TaXml::writeInt("data-dimension", 2, _mhdr.data_dimension);
  hdr += TaXml::writeString("data-collection-type", 2,
                            _xmlCollectionType2Str(_mhdr.data_collection_type));

  hdr += TaXml::writeString("vlevel-type", 2,
                            _xmlVertType2Str(_mhdr.vlevel_type));
  hdr += TaXml::writeString("native-vlevel-type", 2,
                            _xmlVertType2Str(_mhdr.native_vlevel_type));

  // user-specified data

  if (_mhdr.user_data != NOT_SET) {
    hdr += TaXml::writeInt("user-data", 2, _mhdr.user_data);
  }

  for (int ii = 0; ii < 8; ii++) {
    if (_mhdr.user_data_si32[ii] != NOT_SET) {
      char tag[32];
      sprintf(tag, "user-int-%d", ii);
      hdr += TaXml::writeInt(tag, 2, _mhdr.user_data_si32[ii]);
    }
  }

  for (int ii = 0; ii < 6; ii++) {
    if (_mhdr.user_data_fl32[ii] != NOT_SET) {
      char tag[32];
      sprintf(tag, "user-float-%d", ii);
      hdr += TaXml::writeDouble(tag, 2, _mhdr.user_data_fl32[ii]);
    }
  }

  // number of fields and chunks

  hdr += TaXml::writeBoolean("field-grids-differ", 2,
                             _mhdr.field_grids_differ);
  hdr += TaXml::writeInt("n-fields", 2, _mhdr.n_fields);
  hdr += TaXml::writeInt("n-chunks", 2, _mhdr.n_chunks);

  hdr += TaXml::writeEndTag("master-header", 1);

}

/////////////////////////////////////////////////////////////////////////
// Write XML field header

void Mdvx::_writeToXmlFieldHdr(string &hdr, int fieldNum) const
  
{

  vector<TaXml::attribute> attrs;

  hdr += TaXml::writeStartTag("field", 1);
  
  const field_header_t &fhdr = _fields[fieldNum]->getFieldHeader();
  
  hdr += TaXml::writeString("field-name", 2, fhdr.field_name);
  hdr += TaXml::writeString("field-name-long", 2, fhdr.field_name_long);
  hdr += TaXml::writeString("field-units", 2, fhdr.units);
  hdr += TaXml::writeString("field-transform", 2, fhdr.transform);
  
  hdr += TaXml::writeString("encoding-type", 2,
                            _xmlEncodingType2Str(fhdr.encoding_type));
  hdr += TaXml::writeInt("byte-width", 2, attrs, fhdr.data_element_nbytes);
  
  hdr += TaXml::writeDouble("field-data-scale", 2, fhdr.scale);
  hdr += TaXml::writeDouble("field-data-bias", 2, fhdr.bias);
  
  hdr += TaXml::writeString("compression-type", 2,
                            _xmlCompressionType2Str(fhdr.compression_type));
  
  hdr += TaXml::writeString("transform-type", 2,
                            _xmlTransformType2Str(fhdr.transform_type));
  
  hdr += TaXml::writeString("scaling-type", 2,
                            _xmlScalingType2Str(fhdr.scaling_type));
  
  hdr += TaXml::writeDouble("missing-data-value", 2,
                            fhdr.missing_data_value, "%.8f");
  hdr += TaXml::writeDouble("bad-data-value", 2,
                            fhdr.bad_data_value, "%.8f");
  hdr += TaXml::writeDouble("min-value", 2, fhdr.min_value, "%.8f");
  hdr += TaXml::writeDouble("max-value", 2, fhdr.max_value, "%.8f");
  
  // grid
  
  hdr += TaXml::writeInt("data-dimension", 2, fhdr.data_dimension);
  hdr += TaXml::writeBoolean("dz-constant", 2, fhdr.dz_constant);
  hdr += TaXml::writeStartTag("projection", 2);
  hdr += TaXml::writeString("proj-type", 3, _xmlProjType2Str(fhdr.proj_type));
  hdr += TaXml::writeDouble("origin-lat", 3, fhdr.proj_origin_lat, "%.8f");
  hdr += TaXml::writeDouble("origin-lon", 3, fhdr.proj_origin_lon, "%.8f");
  switch (fhdr.proj_type) {
    case PROJ_LAMBERT_CONF:
      hdr += TaXml::writeDouble("lat1", 3, fhdr.proj_param[0], "%.8f");
      hdr += TaXml::writeDouble("lat2", 3, fhdr.proj_param[1], "%.8f");
      break;
    case PROJ_POLAR_STEREO:
      hdr += TaXml::writeDouble("tangen-lon", 3, fhdr.proj_param[0], "%.8f");
      if (fhdr.proj_param[1] == 0) {
        hdr += TaXml::writeString("pole", 3, "N");
      } else {
        hdr += TaXml::writeString("pole", 3, "S");
      }
      hdr += TaXml::writeDouble("central-scale", 3, fhdr.proj_param[2], "%.8f");
      break;
    case PROJ_OBLIQUE_STEREO:
      hdr += TaXml::writeDouble("tangen-lat", 3, fhdr.proj_param[0], "%.8f");
      hdr += TaXml::writeDouble("tangen-lon", 3, fhdr.proj_param[1], "%.8f");
      break;
    case PROJ_FLAT:
      hdr += TaXml::writeDouble("rotation", 3, fhdr.proj_rotation, "%.8f");
      break;
    default: {}
  }
  hdr += TaXml::writeEndTag("projection", 2);

  hdr += TaXml::writeStartTag("xy-grid", 2);
  hdr += TaXml::writeInt("nx", 3, fhdr.nx);
  hdr += TaXml::writeInt("ny", 3, fhdr.ny);
  hdr += TaXml::writeDouble("minx", 3, fhdr.grid_minx, "%.8f");
  hdr += TaXml::writeDouble("miny", 3, fhdr.grid_miny, "%.8f");
  hdr += TaXml::writeDouble("dx", 3, fhdr.grid_dx, "%.8f");
  hdr += TaXml::writeDouble("dy", 3, fhdr.grid_dy, "%.8f");
  hdr += TaXml::writeEndTag("xy-grid", 2);

  // vertical levels
    
  if (fhdr.nz > 0) {
      
    const vlevel_header_t &vhdr = _fields[fieldNum]->getVlevelHeader();

    int vtype = vhdr.type[0];
    bool vtypesVary = false;
    for (int ilevel = 1; ilevel < fhdr.nz; ilevel++) {
      if (vhdr.type[ilevel] != vtype) {
        vtypesVary = true;
      }
    }
      
    hdr += TaXml::writeInt("n-vlevels", 2, fhdr.nz);
    if (vtypesVary) {
      hdr += TaXml::writeString("vlevel-type", 2, "variable");
    } else {
      hdr += TaXml::writeString("vlevel-type", 2, _xmlVertType2Str(vtype));
    }
    hdr += TaXml::writeString("native-vlevel-type", 2,
                              _xmlVertType2Str(fhdr.native_vlevel_type));

    hdr += TaXml::writeStartTag("vlevels", 2);
      
    for (int ilevel = 0; ilevel < fhdr.nz; ilevel++) {
      if (vtypesVary) {
        TaXml::setStringAttr("vtype", _xmlVertType2Str(vhdr.type[ilevel]), attrs);
        hdr += TaXml::writeDouble("level", 3, attrs, vhdr.level[ilevel]);
      } else {
        hdr += TaXml::writeDouble("level", 3, vhdr.level[ilevel]);
      }
    }
      
    hdr += TaXml::writeEndTag("vlevels", 2);

    if (fhdr.vert_reference != NOT_SET) {
      hdr += TaXml::writeDouble("vert-reference", 2, fhdr.vert_reference);
    }

  }
  
  // data location

  hdr += TaXml::writeInt("data-offset-bytes", 2, fhdr.field_data_offset);
  hdr += TaXml::writeInt("data-length-bytes", 2, fhdr.volume_size);

  // user-specified data

  for (int ii = 0; ii < 10; ii++) {
    if (fhdr.user_data_si32[ii] != NOT_SET) {
      char tag[32];
      sprintf(tag, "user-int-%d", ii);
      hdr += TaXml::writeInt(tag, 2, fhdr.user_data_si32[ii]);
    }
  }

  for (int ii = 0; ii < 4; ii++) {
    if (fhdr.user_data_fl32[ii] != NOT_SET) {
      char tag[32];
      sprintf(tag, "user-float-%d", ii);
      hdr += TaXml::writeDouble(tag, 2, fhdr.user_data_fl32[ii]);
    }
  }

  if (fhdr.user_time1 != NOT_SET) {
    hdr += TaXml::writeTime("user-time-1", 1, fhdr.user_time1);
  }
  if (fhdr.user_time2 != NOT_SET) {
    hdr += TaXml::writeTime("user-time-2", 1, fhdr.user_time2);
  }
  if (fhdr.user_time3 != NOT_SET) {
    hdr += TaXml::writeTime("user-time-3", 1, fhdr.user_time3);
  }
  if (fhdr.user_time4 != NOT_SET) {
    hdr += TaXml::writeTime("user-time-4", 1, fhdr.user_time4);
  }

  // grib code

  if (fhdr.field_code != NOT_SET) {
    hdr += TaXml::writeInt("grib-code", 2, fhdr.field_code);
  }

  hdr += TaXml::writeEndTag("field", 1);

}

/////////////////////////////////////////////////////////////////////////
// Write XML chunk header

void Mdvx::_writeToXmlChunkHdr(string &hdr, int chunkNum) const
  
{

  const chunk_header_t &chdr = _chunks[chunkNum]->getHeader();

  hdr += TaXml::writeStartTag("chunk", 1);
  hdr += TaXml::writeInt("chunk-id", 2, chdr.chunk_id);
  hdr += TaXml::writeString("chunk-info", 2, chdr.info);
  hdr += TaXml::writeInt("data-offset-bytes", 2, chdr.chunk_data_offset);
  hdr += TaXml::writeInt("data-length-bytes", 2, chdr.size);
  hdr += TaXml::writeEndTag("chunk", 1);

}

////////////////////////////////////////////////////
// XML read volume method

int Mdvx::_readVolumeXml(bool fill_missing,
                         bool do_decimate,
                         bool do_final_convert,
                         bool is_vsection,
                         double vsection_min_lon,
                         double vsection_max_lon)
  
{

  // open XML header file

  TaFile xmlFile;
  if (xmlFile.fopenUncompress(_pathInUse.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // stat the file to get size

  if (xmlFile.fstat()) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  stat_struct_t &fileStat = xmlFile.getStat();
  size_t fileLen = fileStat.st_size;
  
  // read in buffer

  TaArray<char> fileBuf_;
  char *fileBuf = fileBuf_.alloc(fileLen + 1);
  if (xmlFile.fread(fileBuf, 1, fileLen) != fileLen) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    xmlFile.fclose();
    return -1;
  }

  // close XML header file

  xmlFile.fclose();

  // ensure null termination

  fileBuf[fileLen] = '\0';
  
  // load xml string

  string xmlBuf(fileBuf);

  if (_debug) {
    cerr << "Reading XML file" << endl;
    cerr << "path: " << _pathInUse << endl;
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
    cerr << xmlBuf;
    cerr << "===XML===XML===XML===XML===XML===XML" << endl;
  }

  // find MDV tags buf
  
  string mdvBuf;
  if (TaXml::readString(xmlBuf, "mdv", mdvBuf)) {
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "  Cannot decode XML - no mdv section\n";
    return -1;
  }

  // set buf file path
  
  string bufFileName;
  Path bufPath(_pathInUse);
  if (TaXml::readString(mdvBuf, "buf-file-name", bufFileName) == 0) {
    bufPath.setFile(bufFileName);
  } else {
    Path xmlPath(_pathInUse);
    bufPath.setExt("buf");
  }
  string bufFilePath = bufPath.getPath();

  if (_debug) {
    cerr << "Buf file path: " << bufFilePath << endl;
  }

  // open buf file

  TaFile bufFile;
  if (bufFile.fopenUncompress(bufFilePath.c_str(), "rb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    TaStr::AddStr(_errStr, "  Cannot open bufFilePath: ",
                  bufFilePath);
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // read the master header

  string masterBuf;
  if (TaXml::readString(mdvBuf, "master-header", masterBuf)) {
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "  Cannot decode XML - no master header section\n";
    return -1;
  }

  int forecast_delta = -1;
  if (_readXmlToMasterHdr(masterBuf, forecast_delta)) {
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "  Cannot decode master header\n";
    return -1;
  }

  // save the master header as found in the file
  
  _mhdrFile = _mhdr;
  
  // get the vector of field xml strings
  
  vector<string> fieldXmls;
  if (TaXml::readStringArray(mdvBuf, "field", fieldXmls)) {
    _errStr += "WARNING - Mdvx::_readVolumeXml\n";
    _errStr += "  No fields found\n";
  }

  // check the number of fields

  if ((int) fieldXmls.size() != _mhdr.n_fields) {
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "  Incorrect number of fields found in XML\n";
    TaStr::AddInt(_errStr, "  Master header n_fields: ",
                  _mhdr.n_fields);
    TaStr::AddInt(_errStr, "  XML has n fields: ",
                  (int) fieldXmls.size());
    return -1;
  }

  // load up the read field number array

  if (_loadXmlReadFieldNums(fieldXmls)) {
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "  Cannot determine which fields are to be read\n";
    return -1;
  }

  // read the fields headers

  for (int ii = 0; ii < (int) fieldXmls.size(); ii++) {

    int fieldNum = ii;
    const string &fieldXml = fieldXmls[fieldNum];
    
    field_header_t fhdr;
    vlevel_header_t vhdr;
    
    time_t forecast_time = 0;
    if (forecast_delta >= 0) {
      forecast_time = _mhdr.time_gen + forecast_delta;
    }
    if (_readXmlToFieldHeaders(fieldXml,
                               forecast_time,
                               forecast_delta,
                               fhdr, vhdr)) {
      _errStr += "ERROR - Mdvx::_readVolumeXml\n";
      _errStr += "  Cannot decode field headers\n";
      return -1;
    }

    _fhdrsFile.push_back(fhdr);
    _vhdrsFile.push_back(vhdr);

    MdvxField *fld = _readXmlField(fhdr, vhdr, bufFile);
    if (fld == NULL) {
      _errStr += "ERROR - Mdvx::_readVolumeXml\n";
      TaStr::AddStr(_errStr, "  Cannot read field, name: ", fhdr.field_name);
      return -1;
    }
    fld->setFieldHeaderFile(fhdr);
    fld->setVlevelHeaderFile(vhdr);
    addField(fld);
    
  } // ii

  // convert each field according to the read request
  
  MdvxRemapLut remapLut;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    MdvxField *field = _fields[ii];
    if (field->_apply_read_constraints(*this, fill_missing,
                                       do_decimate, do_final_convert,
                                       remapLut, is_vsection,
                                       vsection_min_lon, vsection_max_lon)) {
      _errStr += "ERROR - Mdvx::_readVolumeXml.\n";
      char errstr[128];
      sprintf(errstr, "  Converting field: %s\n",
              field->getFieldHeader().field_name);
      _errStr += errstr;
      _errStr += field->getErrStr();
      return -1;
    }
  }

  // get the vector of chunk xml strings
  
  vector<string> chunkXmls;
  TaXml::readStringArray(mdvBuf, "chunk", chunkXmls);

  // check the number of chunks
  
  if ((int) chunkXmls.size() != _mhdr.n_chunks) {
    _errStr += "ERROR - Mdvx::_readVolumeXml\n";
    _errStr += "  Incorrect number of chunks found in XML\n";
    TaStr::AddInt(_errStr, "  Master header n_chunks: ",
                  _mhdr.n_chunks);
    TaStr::AddInt(_errStr, "  XML has n chunks: ",
                  (int) chunkXmls.size());
    return -1;
  }

  // load up the read chunk number array

  if (_loadXmlReadChunkNums()) {
    _errStr += "ERROR - Mdvx::_readVolumeXml.\n";
    return -1;
  }

  // read the chunks headers
  
  for (int ii = 0; ii < (int) chunkXmls.size(); ii++) {
    
    int chunkNum = ii;
    const string &chunkXml = chunkXmls[chunkNum];
    
    chunk_header_t chdr;
    
    if (_readXmlToChunkHeader(chunkXml, chdr)) {
      _errStr += "ERROR - Mdvx::_readVolumeXml\n";
      _errStr += "  Cannot decode chunk header\n";
      return -1;
    }
    
    _chdrsFile.push_back(chdr);

    MdvxChunk *chunk = _readXmlChunk(chdr, bufFile);
    if (chunk == NULL) {
      _errStr += "ERROR - Mdvx::_readVolumeXml\n";
      TaStr::AddInt(_errStr, "  Cannot read chunk, id: ", chdr.chunk_id);
      return -1;
    }
    addChunk(chunk);
    
  } // ii

  // update the master header mutable members to match the object

  updateMasterHeader();

  return 0;

}

////////////////////////////////////////////////
// read the master header from XML

int Mdvx::_readXmlToMasterHdr(const string &xml,
                              int &forecast_delta)

{

  bool bval;
  time_t tval;
  int ival;
  double dval;
  string sval;

  if (TaXml::readTime(xml, "time-valid", tval)) {
    _errStr += "Cannot find time-valid in master header XML\n";
    return -1;
  }
  _mhdr.time_centroid = tval;

  if (TaXml::readTime(xml, "time-gen", tval) == 0) {
    _mhdr.time_gen = tval;
  }

  if (TaXml::readInt(xml, "forecast-lead-secs", ival) == 0) {
    forecast_delta = ival;
  }

  if (TaXml::readTime(xml, "time-written", tval) == 0) {
    _mhdr.time_written = tval;
  }

  if (TaXml::readTime(xml, "time-user", tval) == 0) {
    _mhdr.user_time = tval;
  }

  if (TaXml::readTime(xml, "time-begin", tval) == 0) {
    _mhdr.time_begin = tval;
  }

  if (TaXml::readTime(xml, "time-end", tval) == 0) {
    _mhdr.time_end = tval;
  }

  if (TaXml::readTime(xml, "time-expire", tval) == 0) {
    _mhdr.time_expire = tval;
  }

  if (TaXml::readString(xml, "data-set-name", sval) == 0) {
    setDataSetName(sval.c_str());
  }

  if (TaXml::readString(xml, "data-set-info", sval) == 0) {
    setDataSetInfo(sval.c_str());
  }

  if (TaXml::readString(xml, "data-set-source", sval) == 0) {
    setDataSetSource(sval.c_str());
  }

  if (TaXml::readDouble(xml, "sensor-lon", dval) == 0) {
    _mhdr.sensor_lon = dval;
  }

  if (TaXml::readDouble(xml, "sensor-lat", dval) == 0) {
    _mhdr.sensor_lat = dval;
  }

  if (TaXml::readDouble(xml, "sensor-alt", dval) == 0) {
    _mhdr.sensor_alt = dval;
  }
  
  if (TaXml::readInt(xml, "data-dimension", ival) == 0) {
    _mhdr.data_dimension = ival;
  }

  if (TaXml::readString(xml, "data-collection-type", sval) == 0) {
    _mhdr.data_collection_type = _xmlCollectionType2Int(sval);
  }

  if (TaXml::readString(xml, "vlevel-type", sval) == 0) {
    _mhdr.vlevel_type = _xmlVertType2Int(sval);
  }

  if (TaXml::readString(xml, "native-vlevel-type", sval) == 0) {
    _mhdr.native_vlevel_type = _xmlVertType2Int(sval);
  }

  if (TaXml::readInt(xml, "user-data", ival) == 0) {
    _mhdr.user_data = ival;
  }

  for (int ii = 0; ii < 8; ii++) {
    char tag[32];
    sprintf(tag, "user-int-%d", ii);
    if (TaXml::readInt(xml, tag, ival) == 0) {
      _mhdr.user_data_si32[ii] = ival;
    }
  }

  for (int ii = 0; ii < 6; ii++) {
    char tag[32];
    sprintf(tag, "user-float-%d", ii);
    if (TaXml::readDouble(xml, tag, dval) == 0) {
      _mhdr.user_data_fl32[ii] = dval;
    }
  }
  
  if (TaXml::readBoolean(xml, "field-grids-differ", bval) == 0) {
    _mhdr.field_grids_differ = (int) bval;
  }

  if (TaXml::readInt(xml, "n-fields", ival)) {
    _errStr += "Cannot find n-fields in master-header XML\n";
    return -1;
  }
  _mhdr.n_fields = ival;

  if (TaXml::readInt(xml, "n-chunks", ival)) {
    _errStr += "Cannot find n-chunks in master-header XML\n";
    return -1;
  }
  _mhdr.n_chunks = ival;

  _mhdr.num_data_times = 1;
  _mhdr.grid_orientation = ORIENT_SN_WE;
  _mhdr.data_ordering = ORDER_XYZ;
  _mhdr.vlevel_included = 1;

  return 0;

}
  
////////////////////////////////////////////////
// load up field numbers to be read

int Mdvx::_loadXmlReadFieldNums(const vector<string> &fieldXmls)

{

  // check requested field numbers
  
  if (_readFieldNums.size() > 0) {
    for (size_t i = 0; i < _readFieldNums.size(); i++) {
      if (_readFieldNums[i] > _mhdr.n_fields - 1) {
        _errStr += "  Requested field number out of range\n";
        TaStr::AddInt(_errStr, "  Requested field number: ",
		      _readFieldNums[i]);
        TaStr::AddInt(_errStr, "  Max field number: ",
		      _mhdr.n_fields - 1);
        return -1;
      }
    }
  }

  // compile a list of field names

  vector<string> fieldNames;
  vector<string> fieldNamesLong;
  for (int ii = 0; ii < (int) fieldXmls.size(); ii++) {
    const string &fieldXml = fieldXmls[ii];
    string name;
    if (TaXml::readString(fieldXml, "field-name", name)) {
      _errStr += "  Cannot find name in field XML\n";
      TaStr::AddInt(_errStr, "  Field number: ", ii);
      return -1;
    }
    fieldNames.push_back(name);
    string longName;
    if (TaXml::readString(fieldXml, "field-name-long", longName) == 0) {
      fieldNamesLong.push_back(longName);
    } else {
      fieldNamesLong.push_back(name);
    }
  }  

  // if field names are specified, look up field numbers, and load
  // up _readFieldNames.

  if (_readFieldNames.size() > 0) {
    bool error = false;
    _readFieldNums.clear();
    for (size_t ii = 0; ii < _readFieldNames.size(); ii++) {
      bool fieldFound = false;
      for (int jj = 0; jj < (int) fieldNames.size(); jj++) {
        if (_readFieldNames[ii] == fieldNames[jj] || 
            _readFieldNames[ii] == fieldNamesLong[jj]) {
          // short or long field name found
          _readFieldNums.push_back(jj);
          fieldFound = true;
          break;
        } 
      } // j
      if (!fieldFound) {
        _errStr += "ERROR - Mdvx::_readVolumeXml\n";
        _errStr += "  Field: ";
        _errStr += _readFieldNames[ii];
        _errStr += " not found in file: ";
        _errStr += _pathInUse;
        _errStr += "\n";
        error = true;
      }
    } // ii
    if (error) {
      return -1;
    }
  }

  // if no field numbers specified, use all fields
  
  if (_readFieldNums.size() == 0) {
    for (int ii = 0; ii < (int) fieldXmls.size(); ii++) {
      _readFieldNums.push_back(ii);
    }
  }
  
  return 0;

}

////////////////////////////////////////////////
// read field headers from XML

int Mdvx::_readXmlToFieldHeaders(const string &xml,
                                 time_t forecast_time,
                                 int forecast_delta,
                                 field_header_t &fhdr,
                                 vlevel_header_t &vhdr)
  
{

  MEM_zero(fhdr);
  MEM_zero(vhdr);

  if (forecast_time != 0) {
    fhdr.forecast_delta = forecast_delta;
    fhdr.forecast_time = forecast_time;
  }

  bool bval;
  time_t tval;
  int ival;
  double dval;
  string sval;

  if (TaXml::readString(xml, "field-name", sval)) {
    _errStr += "Cannot find name in field XML\n";
    return -1;
  }  
  STRncopy(fhdr.field_name, sval.c_str(), MDV_SHORT_FIELD_LEN);

  if (TaXml::readString(xml, "field-name-long", sval) == 0) {
    STRncopy(fhdr.field_name_long, sval.c_str(), MDV_LONG_FIELD_LEN);
  } else {
    STRncopy(fhdr.field_name_long, fhdr.field_name, MDV_LONG_FIELD_LEN);
  }  
  
  if (TaXml::readString(xml, "field-units", sval)) {
    _errStr += "Cannot find units in field XML\n";
    return -1;
  }  
  STRncopy(fhdr.units, sval.c_str(), MDV_UNITS_LEN);
  
  if (TaXml::readString(xml, "field-transform", sval) == 0) {
    STRncopy(fhdr.transform, sval.c_str(), MDV_TRANSFORM_LEN);
  }  

  if (TaXml::readString(xml, "encoding-type", sval)) {
    _errStr += "Cannot find encoding_type in field XML\n";
    return -1;
  }
  fhdr.encoding_type = _xmlEncodingType2Int(sval);

  if (TaXml::readInt(xml, "byte-width", ival)) {
    _errStr += "Cannot find byte_width in field XML\n";
    return -1;
  }  
  fhdr.data_element_nbytes = ival;

  if (TaXml::readDouble(xml, "field-data-scale", dval)) {
    _errStr += "Cannot find field_data_scale in field XML\n";
    return -1;
  }  
  fhdr.scale = dval;

  if (TaXml::readDouble(xml, "field-data-bias", dval)) {
    _errStr += "Cannot find field_data_bias in field XML\n";
    return -1;
  }  
  fhdr.bias = dval;

  if (TaXml::readString(xml, "compression-type", sval) == 0) {
    fhdr.compression_type = _xmlCompressionType2Int(sval);
  } else {
    fhdr.compression_type = COMPRESSION_NONE;
  }

  if (TaXml::readString(xml, "transform-type", sval) == 0) {
    fhdr.transform_type = _xmlTransformType2Int(sval);
  } else {
    fhdr.transform_type = DATA_TRANSFORM_NONE;
  }

  if (TaXml::readString(xml, "scaling-type", sval) == 0) {
    fhdr.scaling_type = _xmlScalingType2Int(sval);
  } else {
    fhdr.scaling_type = SCALING_NONE;
  }

  if (TaXml::readDouble(xml, "missing-data-value", dval)) {
    _errStr += "Cannot find missing_data_value in field XML\n";
    return -1;
  }  
  fhdr.missing_data_value = dval;

  if (TaXml::readDouble(xml, "bad-data-value", dval) == 0) {
    fhdr.bad_data_value = dval;
  } else {
    fhdr.bad_data_value = fhdr.missing_data_value;
  }

  if (TaXml::readDouble(xml, "min-value", dval) == 0) {
    fhdr.min_value = dval;
  }

  if (TaXml::readDouble(xml, "max-value", dval) == 0) {
    fhdr.max_value = dval;
  }

  if (TaXml::readInt(xml, "data-dimension", ival) == 0) {
    fhdr.data_dimension = ival;
  }

  if (TaXml::readBoolean(xml, "dz-constant", bval) == 0) {
    fhdr.dz_constant = (int) bval;
  }

  //////////////
  // projection

  string projXml;
  if (TaXml::readString(xml, "projection", projXml)) {
    _errStr += "Cannot find projection in field XML\n";
    return -1;
  }
  
  if (TaXml::readString(projXml, "proj-type", sval)) {
    _errStr += "Cannot find projection type in field XML\n";
    return -1;
  }  
  fhdr.proj_type = _xmlProjType2Int(sval);

  if (TaXml::readDouble(projXml, "origin-lat", dval) == 0) {
    fhdr.proj_origin_lat = dval;
  }

  if (TaXml::readDouble(projXml, "origin-lon", dval) == 0) {
    fhdr.proj_origin_lon = dval;
  }

  switch (fhdr.proj_type) {
    case PROJ_LAMBERT_CONF:
      if (TaXml::readDouble(projXml, "lat1", dval)) {
        _errStr += "In field XML for lambert projection\n";
        _errStr += "  Cannot find lat1\n";
        return -1;
      }
      fhdr.proj_param[0] = dval;
      if (TaXml::readDouble(projXml, "lat2", dval)) {
        _errStr += "In field XML for lambert projection\n";
        _errStr += "  Cannot find lat2\n";
        return -1;
      }
      fhdr.proj_param[1] = dval;
      break;
    case PROJ_POLAR_STEREO:
      if (TaXml::readDouble(projXml, "tangent-lon", dval)) {
        _errStr += "In field XML for polar stereographic projection\n";
        _errStr += "  Cannot find tangent_lon\n";
        return -1;
      }
      fhdr.proj_param[0] = dval;
      if (TaXml::readString(projXml, "pole", sval)) {
        _errStr += "In field XML for polar stereographic projection\n";
        _errStr += "  Cannot find pole\n";
        return -1;
      }
      if (sval == "S") {
        fhdr.proj_param[1] = 1;
      } else {
        fhdr.proj_param[1] = 0;
      }
      fhdr.proj_param[2] = 1;
      if (TaXml::readDouble(projXml, "central-scale", dval) == 0) {
        fhdr.proj_param[2] = dval;
      }
      break;
    case PROJ_OBLIQUE_STEREO:
      if (TaXml::readDouble(projXml, "tangent-lat", dval)) {
        _errStr += "In field XML for oblique stereographic projection\n";
        _errStr += "  Cannot find tangent_lat\n";
        return -1;
      }
      fhdr.proj_param[0] = dval;
      if (TaXml::readDouble(projXml, "tangent-lon", dval)) {
        _errStr += "In field XML for oblique stereographic projection\n";
        _errStr += "  Cannot find tangent_lon\n";
        return -1;
      }
      fhdr.proj_param[1] = dval;
      fhdr.proj_param[2] = 1;
      if (TaXml::readDouble(projXml, "central-scale", dval) == 0) {
        fhdr.proj_param[2] = dval;
      }
      break;
      break;
    case PROJ_FLAT:
      if (TaXml::readDouble(projXml, "rotation", dval) == 0) {
        fhdr.proj_rotation = dval;
      }
      break;
    default: {}
  }
  
  //////////////////////////////////////////////////
  // xy grid
  
  string gridXml;
  if (TaXml::readString(xml, "xy-grid", gridXml)) {
    _errStr += "Cannot find xy-grid in field XML\n";
    return -1;
  }
  
  if (TaXml::readInt(gridXml, "nx", ival)) {
    _errStr += "Cannot find xy-grid nx in field XML\n";
    return -1;
  }  
  fhdr.nx = ival;

  if (TaXml::readInt(gridXml, "ny", ival)) {
    _errStr += "Cannot find xy-grid ny in field XML\n";
    return -1;
  }  
  fhdr.ny = ival;

  if (TaXml::readDouble(gridXml, "minx", dval)) {
    _errStr += "Cannot find xy-grid minx in field XML\n";
    return -1;
  }  
  fhdr.grid_minx = dval;

  if (TaXml::readDouble(gridXml, "miny", dval)) {
    _errStr += "Cannot find xy-grid miny in field XML\n";
    return -1;
  }  
  fhdr.grid_miny = dval;

  if (TaXml::readDouble(gridXml, "miny", dval)) {
    _errStr += "Cannot find xy-grid miny in field XML\n";
    return -1;
  }  
  fhdr.grid_miny = dval;

  if (TaXml::readDouble(gridXml, "dx", dval)) {
    _errStr += "Cannot find xy-grid dx in field XML\n";
    return -1;
  }  
  fhdr.grid_dx = dval;

  if (TaXml::readDouble(gridXml, "dy", dval)) {
    _errStr += "Cannot find xy-grid dy in field XML\n";
    return -1;
  }  
  fhdr.grid_dy = dval;

  if (TaXml::readDouble(gridXml, "dy", dval)) {
    _errStr += "Cannot find xy-grid dy in field XML\n";
    return -1;
  }  
  fhdr.grid_dy = dval;

  ////////////////////////////////////////////////////////
  // vlevels

  if (TaXml::readInt(xml, "n-vlevels", ival)) {
    _errStr += "Cannot find n-vlevels in field XML\n";
    return -1;
  }  
  fhdr.nz = ival;

  if (TaXml::readString(xml, "vlevel-type", sval)) {
    _errStr += "Cannot find vlevel-type in field XML\n";
    return -1;
  }
  fhdr.vlevel_type = _xmlVertType2Int(sval);

  if (TaXml::readString(xml, "native-vlevel-type", sval) == 0) {
    fhdr.native_vlevel_type = _xmlVertType2Int(sval);
  } else {
    fhdr.native_vlevel_type = fhdr.vlevel_type;
  }

  string vxml;
  if (TaXml::readString(xml, "vlevels", vxml)) {
    _errStr += "Cannot find vlevels in field XML\n";
    return -1;
  }

  vector<string> vlevelsArray;
  if (TaXml::readTagBufArray(xml, "level", vlevelsArray)) {
    _errStr += "Cannot find levels array in field XML\n";
    return -1;
  }

  if ((int) vlevelsArray.size() != fhdr.nz) {
    _errStr += "Incorrect number of vlevels in XML array\n";
    TaStr::AddInt(_errStr, "Expected n_vlevels: ", fhdr.nz);
    TaStr::AddInt(_errStr, "Found levels array size: ", (int) vlevelsArray.size()); 
    return -1;
  }

  for (int ii = 0; ii < (int) vlevelsArray.size(); ii++) {
    const string &levelBuf = vlevelsArray[ii];
    vector<TaXml::attribute> attrs;
    if (TaXml::readDouble(levelBuf, "level", dval, attrs)) {
      _errStr += "Cannot decode vlevels in field XML\n";
      _errStr += "Entry: ";
      _errStr += levelBuf;
      _errStr += "\n";
      return -1;
    }
    vhdr.level[ii] = dval;
    vhdr.type[ii] = fhdr.vlevel_type;
    if (attrs.size() > 0) {
      if (TaXml::readStringAttr(attrs, "vtype", sval) == 0) {
        vhdr.type[ii] = _xmlVertType2Int(sval);
      }
    }
  }

  if (fhdr.nz == 1) {
    fhdr.grid_dz = 1.0;
  } else {
    fhdr.grid_dz = (vhdr.level[1] -vhdr.level[0]);
  }
  fhdr.grid_minz = vhdr.level[0];
  
  if (TaXml::readDouble(xml, "vert-reference", dval) == 0) {
    fhdr.vert_reference = dval;
  }

  // data location

  if (TaXml::readInt(xml, "data-offset-bytes", ival)) {
    _errStr += "Cannot find data-offset-bytes in field XML\n";
    return -1;
  }
  fhdr.field_data_offset = ival;

  if (TaXml::readInt(xml, "data-length-bytes", ival)) {
    _errStr += "Cannot find data-length-bytes in field XML\n";
    return -1;
  }
  fhdr.volume_size = ival;

  // user-specified data

  for (int ii = 0; ii < 10; ii++) {
    char tag[32];
    sprintf(tag, "user-int-%d", ii);
    if (TaXml::readInt(xml, tag, ival) == 0) {
      fhdr.user_data_si32[ii] = ival;
    }
  }

  for (int ii = 0; ii < 4; ii++) {
    char tag[32];
    sprintf(tag, "user-float-%d", ii);
    if (TaXml::readDouble(xml, tag, dval) == 0) {
      fhdr.user_data_fl32[ii] = dval;
    }
  }
  
  if (TaXml::readTime(xml, "user-time-1", tval) == 0) {
    fhdr.user_time1 = tval;
  }

  if (TaXml::readTime(xml, "user-time-2", tval) == 0) {
    fhdr.user_time2 = tval;
  }

  if (TaXml::readTime(xml, "user-time-3", tval) == 0) {
    fhdr.user_time3 = tval;
  }

  if (TaXml::readTime(xml, "user-time-4", tval) == 0) {
    fhdr.user_time4 = tval;
  }

  // grib code

  if (TaXml::readInt(xml, "grib-code", ival) == 0) {
    fhdr.field_code = ival;
  }

  return 0;

}
  
////////////////////////////////////////////////
// read field from buffer file
//
// returns NULL on failure

MdvxField *Mdvx::_readXmlField(const field_header_t &fhdr,
                               const vlevel_header_t &vhdr,
                               TaFile &bufFile)
  
{
  
  // seek to the start of the field data

  if (bufFile.fseek(fhdr.field_data_offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "Cannot seek to start of field data\n";
    TaStr::AddInt(_errStr, "offset: ", fhdr.field_data_offset);
    _errStr += strerror(errNum);
    _errStr += "\n";
    return NULL;
  }

  // create an array to read into

  size_t len = fhdr.volume_size;
  TaArray<char> data_;
  char *data = data_.alloc(len);

  // read the data

  if (bufFile.fread(data, 1, len) != len) {
    int errNum = errno;
    _errStr += "Cannot read in field data\n";
    TaStr::AddInt(_errStr, "offset: ", fhdr.field_data_offset);
    _errStr += strerror(errNum);
    _errStr += "\n";
    return NULL;
  }

  if (fhdr.compression_type == COMPRESSION_NONE) {
    // byte swap data
    MdvxField::buffer_from_BE(data, len, fhdr.encoding_type);
    // construct field and return
    MdvxField *fld = new MdvxField(fhdr, vhdr, data, false, false);
    return fld;
  }

  // field volume is compressed with gzip
  // construct a header for the compressed data

  compress_buf_hdr_t chdr;
  MEM_zero(chdr);
  chdr.magic_cookie = GZIP_COMPRESSED;
  chdr.nbytes_uncompressed =
    fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
  chdr.nbytes_compressed = len + sizeof(chdr);
  chdr.nbytes_coded = len;
  BE_to_array_32(&chdr, sizeof(chdr));

  // adjust volume size for the header
  
  fhdr.volume_size += sizeof(chdr);
  
  // use a MemBuf to concatenate the header with the data
  
  MemBuf membuf;
  membuf.add(&chdr, sizeof(chdr));
  membuf.add(data, len);
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, membuf.getPtr(), false, false);
  return fld;
  
}

////////////////////////////////////////////////
// load up chunk numbers to be read

int Mdvx::_loadXmlReadChunkNums()

{
  
  if (_readChunkNums.size() > 0) {
    for (size_t i = 0; i < _readChunkNums.size(); i++) {
      if (_readChunkNums[i] > _mhdr.n_chunks - 1) {
        _errStr += "ERROR - Mdvx::_readVolumeXml\n";
        _errStr += "  Requested chunk number out of range\n";
        TaStr::AddInt(_errStr, "  Requested chunk number: ",
		      _readChunkNums[i]);
        TaStr::AddInt(_errStr, "  Max chunk number: ", _mhdr.n_chunks - 1);
        return -1;
      }
    }
  }

  if (_readChunkNums.size() == 0) {

    // if no chunk numbers specified, use all chunks
    
    for (int i = 0; i < _mhdr.n_chunks; i++) {
      _readChunkNums.push_back(i);
    }

  } else if (_readChunkNums.size() > 0) {

    // if a negative chunk number is specified, read no chunks
    // Calling setReadNoChunks() sets a chunk number of -1
    
    for (size_t i = 0; i < _readChunkNums.size(); i++) {
      if (_readChunkNums[i] < 0) {
	clearReadChunks();
	break;
      }
    } // i

  }

  return 0;

}
  
////////////////////////////////////////////////
// read chunk headers from XML

int Mdvx::_readXmlToChunkHeader(const string &xml,
                                chunk_header_t &chdr)
  
{

  MEM_zero(chdr);

  int ival;
  string sval;

  if (TaXml::readString(xml, "chunk-info", sval) == 0) {
    STRncopy(chdr.info, sval.c_str(), MDV_CHUNK_INFO_LEN);
  }

  if (TaXml::readInt(xml, "chunk-id", ival)) {
    _errStr += "Cannot find id in chunk XML\n";
    return -1;
  }  
  chdr.chunk_id = ival;

  if (TaXml::readInt(xml, "data-offset-bytes", ival)) {
    _errStr += "Cannot find data-offset-bytes in chunk XML\n";
    return -1;
  }  
  chdr.chunk_data_offset = ival;

  if (TaXml::readInt(xml, "data-length-bytes", ival)) {
    _errStr += "Cannot find data-length-bytes in chunk XML\n";
    return -1;
  }  
  chdr.size = ival;

  return 0;

}
  
////////////////////////////////////////////////
// read chunk from buffer file
//
// returns NULL on failure

MdvxChunk *Mdvx::_readXmlChunk(const chunk_header_t &chdr,
                               TaFile &bufFile)
  
{
  
  // seek to the start of the chunk data

  if (bufFile.fseek(chdr.chunk_data_offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "Cannot seek to start of chunk data\n";
    TaStr::AddInt(_errStr, "offset: ", chdr.chunk_data_offset);
    _errStr += strerror(errNum);
    _errStr += "\n";
    return NULL;
  }

  // create an array to read into

  size_t len = chdr.size;
  TaArray<char> data_;
  char *data = data_.alloc(len);

  // read the data

  if (bufFile.fread(data, 1, len) != len) {
    int errNum = errno;
    _errStr += "Cannot read in chunk data\n";
    TaStr::AddInt(_errStr, "offset: ", chdr.chunk_data_offset);
    _errStr += strerror(errNum);
    _errStr += "\n";
    return NULL;
  }

  MdvxChunk *chunk = new MdvxChunk(chdr, data);
  return chunk;

}

////////////////////////////////////////////////
// return string representation of proj type

string Mdvx::_xmlProjType2Str(int proj_type)

{

  switch(proj_type)  {
    case PROJ_LATLON:
      return("latlon"); 
    case PROJ_LAMBERT_CONF:
      return("lambert-conformal"); 
    case PROJ_MERCATOR:
      return("mercator"); 
    case PROJ_POLAR_STEREO:
      return("polar-stereographic"); 
    case PROJ_FLAT:
      return("flat"); 
    case PROJ_POLAR_RADAR:
      return("polar-radar"); 
    case PROJ_VSECTION:
      return("vertical-section"); 
    case PROJ_OBLIQUE_STEREO:
      return("oblique-stereographic"); 
    case PROJ_RHI_RADAR:
      return("rhi-radar"); 
    case PROJ_TIME_HEIGHT:
      return("time-height"); 
    default:
      return("unknown");
  }
  
}

////////////////////////////////////////////////
// return string representation of vertical type

string Mdvx::_xmlVertType2Str(int vert_type)

{

  switch(vert_type)  {
    case VERT_TYPE_SURFACE: 
      return("surface"); 
    case VERT_TYPE_SIGMA_P:
      return("sigma-p"); 
    case VERT_TYPE_PRESSURE:
      return("pressure"); 
    case VERT_TYPE_Z:
      return("height-msl-km"); 
    case VERT_TYPE_SIGMA_Z:
      return("sigma-z"); 
    case VERT_TYPE_ETA:
      return("eta"); 
    case VERT_TYPE_THETA:
      return("theta"); 
    case VERT_TYPE_MIXED:
      return("mixed"); 
    case VERT_TYPE_ELEV:
      return("elevation-angles"); 
    case VERT_TYPE_COMPOSITE:
      return("composite"); 
    case VERT_TYPE_CROSS_SEC:
      return("cross-section"); 
    case VERT_SATELLITE_IMAGE:
      return("satellite"); 
    case VERT_VARIABLE_ELEV:
      return("variable-elevations"); 
    case VERT_FIELDS_VAR_ELEV:
      return("field-specifc-variable-elevations"); 
    case VERT_FLIGHT_LEVEL:
      return("flight-level"); 
    case VERT_EARTH_IMAGE:
      return("earth-conformal"); 
    case VERT_TYPE_AZ:
      return("azimuth-angles"); 
    case VERT_TYPE_TOPS:
      return("tops-msl-km"); 
    case VERT_TYPE_ZAGL_FT:
      return("height-agl-ft"); 
    case VERT_TYPE_VARIABLE: 
      return("variable"); 
    default:
      return ("unknown");
  }
}  
 
////////////////////////////////////////////////
// return string for units for x coordinate
// based on projection

string Mdvx::_xmlProjType2XUnits(int proj_type)

{
  
  switch(proj_type)  {
    case PROJ_LATLON:
      return("deg"); 
    case PROJ_TIME_HEIGHT:
      return("sec");
    default:
      return("km"); 
  }

}

////////////////////////////////////////////////
// return string for units for y coordinate
// based on projection

string Mdvx::_xmlProjType2YUnits(int proj_type)

{

  switch(proj_type)  {
    case PROJ_LATLON:
      return("deg"); 
    case PROJ_POLAR_RADAR:
    case PROJ_RHI_RADAR:
      return("deg"); 
    case PROJ_TIME_HEIGHT:
      return("");
    default:
      return("km"); 
  }

}

////////////////////////////////////////////////
// return string for units for z coordinate
// based on vlevel type

string Mdvx::_xmlVertTypeZUnits(int vert_type)

{

  switch(vert_type)  {
    case VERT_TYPE_SURFACE: 
      return(""); 
    case VERT_TYPE_SIGMA_P:
      return("sigma-p"); 
    case VERT_TYPE_PRESSURE:
      return("mb"); 
    case VERT_TYPE_Z:
      return("km"); 
    case VERT_TYPE_SIGMA_Z:
      return("sigma-z"); 
    case VERT_TYPE_ETA:
      return("eta"); 
    case VERT_TYPE_THETA:
      return("theta"); 
    case VERT_TYPE_MIXED:
      return("mixed"); 
    case VERT_TYPE_ELEV:
    case VERT_TYPE_AZ:
      return("deg"); 
    case VERT_TYPE_TOPS:
      return("km"); 
    case VERT_TYPE_COMPOSITE:
      return(""); 
    case VERT_TYPE_CROSS_SEC:
      return(""); 
    case VERT_SATELLITE_IMAGE:
      return(""); 
    case VERT_VARIABLE_ELEV:
      return("deg"); 
    case VERT_FIELDS_VAR_ELEV:
      return("deg"); 
    case VERT_FLIGHT_LEVEL:
      return("FL"); 
    case VERT_TYPE_ZAGL_FT:
      return("ft"); 
    default:
      return(""); 
  }
}  
 
////////////////////////////////////////////////
// return string representation of encoding type

string Mdvx::_xmlEncodingType2Str(int encoding_type)

{
  switch(encoding_type)  {
    case ENCODING_INT8:
      return("int8"); 
    case ENCODING_INT16:
      return("int16"); 
    case ENCODING_FLOAT32:
      return("fl32"); 
    case ENCODING_RGBA32:
      return("rgba32"); 
    default:
      return("unknown");
  }
}  
 
///////////////////////////////////////////////////
// return string representation of collection type

string Mdvx::_xmlCollectionType2Str(int collection_type)

{
  
  switch(collection_type)  {
    case DATA_MEASURED:
      return("measured");
    case DATA_EXTRAPOLATED:
      return("extrapolated");
    case DATA_FORECAST:
      return("forecast");
    case DATA_SYNTHESIS:
      return("synthesis");
    case DATA_MIXED:
      return("mixed");
    case DATA_IMAGE:
      return("rgba-image");
    case DATA_GRAPHIC:
      return("rgba-graphic");
    case DATA_CLIMO_ANA:
      return("climo-analysis");
    case DATA_CLIMO_OBS:
      return("climo-observed");
    default:
      return ("measured");
  }

}

///////////////////////////////////////////////////
// return string representation of data compression

string Mdvx::_xmlCompressionType2Str(int compression_type)

{

  switch(compression_type) {
    
    case COMPRESSION_GZIP_VOL:
      return("gzip");

    default:
      return ("none");

  }
  
}

///////////////////////////////////////////////////
// return string representation of data scaling

string Mdvx::_xmlScalingType2Str(int scaling_type)

{

  switch(scaling_type) {
    
    case SCALING_NONE:
      return("none");
    case SCALING_ROUNDED:
      return("rounded");
    case SCALING_DYNAMIC:
      return("dynamic");
    case SCALING_INTEGRAL:
      return("integral");
    case SCALING_SPECIFIED:
      return("specified");
    default:
      return ("unknown");

  }

}

///////////////////////////////////////////////////
// return string representation of data tranform

string Mdvx::_xmlTransformType2Str(int transform_type)

{

  switch(transform_type) {

    case DATA_TRANSFORM_NONE:
      return("none");
    case DATA_TRANSFORM_LOG:
      return("log");
    case DATA_TRANSFORM_POINT:
      return("point");
    case DATA_TRANSFORM_SUM:
      return("sum");
    case DATA_TRANSFORM_DIFF:
      return("diff");
    case DATA_TRANSFORM_PROD:
      return("product");
    case DATA_TRANSFORM_MAXIMUM:
      return("max");
    case DATA_TRANSFORM_MINIMUM:
      return("min");
    case DATA_TRANSFORM_MEAN:
      return("mean");
    case DATA_TRANSFORM_MEDIAN:
      return("median");
    case DATA_TRANSFORM_MODE:
      return("mode");
    case DATA_TRANSFORM_MID_RANGE:
      return("mid");
    case DATA_TRANSFORM_STDDEV:
      return("stddev");
    case DATA_TRANSFORM_VAR:
      return("variance");
    case DATA_TRANSFORM_COVAR:
      return("covariance");
    case DATA_TRANSFORM_NORM:
      return("normalized");
    default:
      return ("unknown");
  }

}

///////////////////////////////////////////////////
// return string representation of chunk ID

string Mdvx::_xmlChunkId2Str(int chunk_id)

{

  switch (chunk_id) {
    case CHUNK_COMMENT:
      return ("comment-data");
      break;
    case CHUNK_DOBSON_VOL_PARAMS:
      return ("dobson-vol-params");
      break;
    case CHUNK_DOBSON_ELEVATIONS:
      return ("dobson-elevations");
      break;
    case CHUNK_NOWCAST_DATA_TIMES:
      return ("nowcast-data-times");
      break;
    case CHUNK_DSRADAR_PARAMS:
      return ("dsradar-params");
      break;
    case CHUNK_DSRADAR_ELEVATIONS:
      return ("dsradar-elevations");
      break;
    case CHUNK_DSRADAR_AZIMUTHS:
      return ("dsradar-azimuths");
      break;
    case CHUNK_VARIABLE_ELEV:
      return ("variable-elev");
      break;
    case CHUNK_TEXT_DATA:
      return ("text-data");
      break;
    case CHUNK_CLIMO_INFO:
      return ("climo-info");
      break;
    default:
      return ("unknwon");
  }

}


////////////////////////////////////////////////
// return integer representation of proj type

int Mdvx::_xmlProjType2Int(const string &proj_type)

{

  if (proj_type == "latlon") {
    return PROJ_LATLON;
  }
  if (proj_type == "lambert-conformal") {
    return PROJ_LAMBERT_CONF;
  }
  if (proj_type == "mercator") {
    return PROJ_MERCATOR;
  }
  if (proj_type == "polar-stereographic") {
    return PROJ_POLAR_STEREO;
  }
  if (proj_type == "flat") {
    return PROJ_FLAT;
  }
  if (proj_type == "polar-radar") {
    return PROJ_POLAR_RADAR;
  }
  if (proj_type == "vertical-section") {
    return PROJ_VSECTION;
  }
  if (proj_type == "oblique-stereographic") {
    return PROJ_OBLIQUE_STEREO;
  }
  if (proj_type == "rhi-radar") {
    return PROJ_RHI_RADAR;
  }
  if (proj_type == "time-height") {
    return PROJ_TIME_HEIGHT;
  }

  return PROJ_UNKNOWN;
  
}

////////////////////////////////////////////////
// return integet representation of vertical type

int Mdvx::_xmlVertType2Int(const string &vert_type)

{

  if (vert_type == "surface") {
    return VERT_TYPE_SURFACE;
  }
  if (vert_type == "sigma-p") {
    return VERT_TYPE_SIGMA_P;
  }
  if (vert_type == "pressure") {
    return VERT_TYPE_PRESSURE;
  }
  if (vert_type == "height-msl-km") {
    return VERT_TYPE_Z;
  }
  if (vert_type == "sigma-z") {
    return VERT_TYPE_SIGMA_Z;
  }
  if (vert_type == "eta") {
    return VERT_TYPE_ETA;
  }
  if (vert_type == "theta") {
    return VERT_TYPE_THETA;
  }
  if (vert_type == "mixed") {
    return VERT_TYPE_MIXED;
  }
  if (vert_type == "elevation-angles") {
    return VERT_TYPE_ELEV;
  }
  if (vert_type == "composite") {
    return VERT_TYPE_COMPOSITE;
  }
  if (vert_type == "cross-section") {
    return VERT_TYPE_CROSS_SEC;
  }
  if (vert_type == "satellite") {
    return VERT_SATELLITE_IMAGE;
  }
  if (vert_type == "variable-elevations") {
    return VERT_VARIABLE_ELEV;
  }
  if (vert_type == "field-specifc-variable-elevations") {
    return VERT_FIELDS_VAR_ELEV;
  }
  if (vert_type == "flight-level") {
    return VERT_FLIGHT_LEVEL;
  }
  if (vert_type == "earth-conformal") {
    return VERT_EARTH_IMAGE;
  }
  if (vert_type == "azimuth-angles") {
    return VERT_TYPE_AZ;
  }
  if (vert_type == "tops-msl-km") {
    return VERT_TYPE_TOPS;
  }
  if (vert_type == "height-agl-ft") {
    return VERT_TYPE_ZAGL_FT;
  }
  if (vert_type == "variable") {
    return VERT_TYPE_VARIABLE;
  }

  return VERT_TYPE_UNKNOWN;

}  
 
////////////////////////////////////////////////
// return integer representation of encoding type

int Mdvx::_xmlEncodingType2Int(const string &encoding_type)

{
  if (encoding_type == "int16") {
    return ENCODING_INT16;
  }
  if (encoding_type == "fl32") {
    return ENCODING_FLOAT32;
  }
  if (encoding_type == "rgba32") {
    return ENCODING_RGBA32;
  }
  return ENCODING_INT8;
}  
 
///////////////////////////////////////////////////
// return integer representation of collection type

int Mdvx::_xmlCollectionType2Int(const string &collection_type)

{
  
  if (collection_type == "extrapolate") {
    return DATA_EXTRAPOLATED;
  }
  if (collection_type == "forecast") {
    return DATA_FORECAST;
  }
  if (collection_type == "synthesis") {
    return DATA_SYNTHESIS;
  }
  if (collection_type == "mixed") {
    return DATA_MIXED;
  }
  if (collection_type == "rgba-image") {
    return DATA_IMAGE;
  }
  if (collection_type == "rgba-graphic") {
    return DATA_GRAPHIC;
  }
  if (collection_type == "climo-analysis") {
    return DATA_CLIMO_ANA;
  }
  if (collection_type == "climo-observed") {
    return DATA_CLIMO_OBS;
  }

  return DATA_MEASURED;

}

///////////////////////////////////////////////////
// return integer representation of data compression

int Mdvx::_xmlCompressionType2Int(const string &compression_type)

{

  if (compression_type == "gzip") {
    return COMPRESSION_GZIP_VOL;
  }

  return COMPRESSION_NONE;
  
}

///////////////////////////////////////////////////
// return integer representation of data scaling

int Mdvx::_xmlScalingType2Int(const string &scaling_type)

{

  if (scaling_type == "none") {
    return SCALING_NONE;
  }
  if (scaling_type == "rounded") {
    return SCALING_ROUNDED;
  }
  if (scaling_type == "dynamic") {
    return SCALING_DYNAMIC;
  }
  if (scaling_type == "integral") {
    return SCALING_INTEGRAL;
  }
  if (scaling_type == "specified") {
    return SCALING_SPECIFIED;
  }
  return SCALING_NONE;

}

///////////////////////////////////////////////////
// return integer representation of data tranform

int Mdvx::_xmlTransformType2Int(const string &transform_type)

{

  if (transform_type == "none") {
    return DATA_TRANSFORM_NONE;
  }
  if (transform_type == "log") {
    return DATA_TRANSFORM_LOG;
  }
  if (transform_type == "point") {
    return DATA_TRANSFORM_POINT;
  }
  if (transform_type == "sum") {
    return DATA_TRANSFORM_SUM;
  }
  if (transform_type == "diff") {
    return DATA_TRANSFORM_DIFF;
  }
  if (transform_type == "product") {
    return DATA_TRANSFORM_PROD;
  }
  if (transform_type == "max") {
    return DATA_TRANSFORM_MAXIMUM;
  }
  if (transform_type == "min") {
    return DATA_TRANSFORM_MINIMUM;
  }
  if (transform_type == "mean") {
    return DATA_TRANSFORM_MEAN;
  }
  if (transform_type == "median") {
    return DATA_TRANSFORM_MEDIAN;
  }
  if (transform_type == "mode") {
    return DATA_TRANSFORM_MODE;
  }
  if (transform_type == "mid") {
    return DATA_TRANSFORM_MID_RANGE;
  }
  if (transform_type == "stddev") {
    return DATA_TRANSFORM_STDDEV;
  }
  if (transform_type == "variance") {
    return DATA_TRANSFORM_VAR;
  }
  if (transform_type == "covariance") {
    return DATA_TRANSFORM_COVAR;
  }
  if (transform_type == "normalized") {
    return DATA_TRANSFORM_NORM;
  }
  return DATA_TRANSFORM_NONE;

}

