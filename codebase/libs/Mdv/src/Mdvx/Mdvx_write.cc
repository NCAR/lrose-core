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
// Mdvx_write.cc
//
// Write routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>
#include <dataport/bigend.h>
#include <didss/LdataInfo.hh>
#include <didss/RapDataDir.hh>
using namespace std;

//////////////////////////////
// Setting up to write
//

/////////////////////////////////////////
// clear all write requests, set defaults

void Mdvx::clearWrite()

{
  clearWriteAsForecast();
  clearWriteAddYearSubdir();
  clearIfForecastWriteAsForecast();
  clearWriteFormat();
  setWriteLdataInfo();
}

////////////////////////////////////////////////////////////////////////
// write using extended paths
//
// If false, file path is:
//   topdir/yyyymmdd/hhmmss.mdv, or
//   topdir/yyyymmdd/g_hhmmss/f_ssssssss.mdv
//
// If true, file path is:
//   topdir/yyyy/yyyymmdd/yyyymmdd_hhmmss.mdv, or
//   topdir/yyyy/yyyymmdd/g_hhmmss/yyyymmdd_ghhmmss_fssssssss.mdv

void Mdvx::setWriteUsingExtendedPath()
{
  _useExtendedPaths = true;
}

void Mdvx::clearWriteUsingExtendedPath()
{
  _useExtendedPaths = false;
}

////////////////////////////////////////////////////////////////////////
// write adding a subdir for the year
//
// If true, file path is:
//   topdir/yyyy/yyyymmdd....
//
// If false, file path is:
//   topdir/yyyymmdd....
//

void Mdvx::setWriteAddYearSubdir()
{
  _writeAddYearSubdir = true;
}
void Mdvx::clearWriteAddYearSubdir()
{
  _writeAddYearSubdir = false;
}

////////////////////////////////////////////////////////////////////////
// write as forecast? - forces a forecast write if set
//
// If false, file path is computed as:
//   topdir/yyyymmdd/hhmmss.mdv
//
// If true, file path is computed as:
//   topdir/yyyymmdd/g_hhmmss/f_ssssssss.mdv, where g_hhmmss is generate
//   time and ssssssss is forecast lead time in secs.

void Mdvx::setWriteAsForecast()
{
  _writeAsForecast = true;
}

void Mdvx::clearWriteAsForecast()
{
  _writeAsForecast = false;
}

////////////////////////////////////////////////////////////////////////
// if forecast, write as forecast?
//
// Same as writeAsForecast, except it only activates if the
// data_collection_type is FORECAST or EXTRAPOLATED

void Mdvx::setIfForecastWriteAsForecast()
{
  _ifForecastWriteAsForecast = true;
}

void Mdvx::clearIfForecastWriteAsForecast()
{
  _ifForecastWriteAsForecast = false;
}

////////////////////////////////////////////////////////////////////////
// set write format

void Mdvx::setWriteFormat(mdv_format_t format)
{
  _writeFormat = format;
}

void Mdvx::clearWriteFormat()
{
  _writeFormat = FORMAT_MDV;
}

////////////////////////////////////////////////////////
// write _latest_data_info file?
//
// If true, _latest_data_info file is written in top_dir

void Mdvx::setWriteLdataInfo()
{
  _writeLdataInfo = true;
}

void Mdvx::clearWriteLdataInfo()
{
  _writeLdataInfo = false;
}

//////////////////////////////////////////////////////
// Write to directory
//
// File path is computed - see setWriteAsForecast().
// _latest_data_info file is written as appropriate - 
//    see setWriteLdataInfo().
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int Mdvx::writeToDir(const string &output_dir)
{

  clearErrStr();
  updateMasterHeader();

  // compute output name and path

  string outputName, outputPath;
  bool writeAsForecast;
  _computeOutputPath(output_dir, outputName, outputPath, writeAsForecast);

  // perform the write
  
  if (writeToPath(outputPath.c_str())) {
    _errStr += "ERROR - Mdvx::writeToDir\n";
    return -1;
  }
  
  // write the latest data info file

  if (_writeLdataInfo) {

    LdataInfo ldata;
    ldata.setDir(output_dir);
    ldata.setDebug(_debug);
    if (_writeFormat == FORMAT_XML) {
      ldata.setDataFileExt("xml");
      ldata.setDataType("xml");
    } else if (_currentFormat == FORMAT_NCF) {
      ldata.setDataFileExt("nc");
      ldata.setDataType("nc");
    } else {
      ldata.setDataFileExt("mdv");
      ldata.setDataType("mdv");
    }
    ldata.setWriter(_appName.c_str());

    string relName = outputName;
    if (_writeFormat == FORMAT_XML) {
      relName += ".xml";
    } else if (_currentFormat == FORMAT_NCF) {
      relName += getNcfExt();
    }
    ldata.setRelDataPath(relName.c_str());
    time_t latestTime;
    if (writeAsForecast) {
      ldata.setIsFcast();
      ldata.setLeadTime(getForecastLeadSecs());
      latestTime = _mhdr.time_gen;
    } else {
      latestTime = _mhdr.time_centroid;
    }
    if (ldata.write(latestTime)) {
      _errStr += "ERROR - Mdvx::writeToDir\n";
      _errStr += "  Error writing _latest_data_info file,\n";
      _errStr += "    for output file: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }

  }

  return 0;

}

/////////////////////////////////////////////////////////
// Write to path
//
// File is  written to specified path.
// Note: no _latest_data_info file is written.
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int Mdvx::writeToPath(const string &output_path)

{

  clearErrStr();
  updateMasterHeader();
  time_t now = time(NULL);
  _mhdr.time_written = (si32) now;

  // if to be written as XML, call XML method
  
  if (_writeFormat == FORMAT_XML) {
    return _write_as_xml(output_path);
  } else if (_currentFormat == FORMAT_NCF) {
    return _write_as_ncf(output_path);
  }
    
  string fullOutPath;
  RapDataDir.fillPath(output_path, fullOutPath);
  _pathInUse = fullOutPath;

  if (_debug) {
    cerr << "Mdvx - writing to path: " << fullOutPath << endl;
  }
  
  // remove compressed file if it exists
  
  ta_remove_compressed(fullOutPath.c_str());

  // open tmp file

  Path outPath(fullOutPath);
  outPath.makeDirRecurse();
  string tmpPath = outPath.computeTmpPath();
  
  TaFile outfile;
  outfile.setRemoveOnDestruct();

  if (outfile.fopen(tmpPath.c_str(), "wb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::writeToPath\n";
    _errStr += "  Cannot open file for writing: ";
    _errStr += tmpPath;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // compute offset to start of field data

  long writeOffset =
    sizeof(master_header_t) +
    _mhdr.n_fields * (sizeof(field_header_t) + sizeof(vlevel_header_t)) +
    _mhdr.n_chunks * sizeof(chunk_header_t);

  long nextOffset;

  // write field data - this also sets the field data offset in
  // the field headers

  for (int i = 0; i < _mhdr.n_fields; i++) {


    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Mdvx::writeToPath");
    }

    if (_fields[i]->_write_volume(outfile, writeOffset, nextOffset)) {
      char field_string[10];
      sprintf(field_string, "%d", i);
      
      _errStr += "ERROR - Mdvx::writeToPath\n";
      _errStr += string("  Error writing volume for field ")
	+ field_string + "\n";
      _errStr += "  Path: " + fullOutPath + "\n";
      return -1;
    }
    writeOffset = nextOffset;
  }
  
  // write chunk data - this also sets the chunk data offset in the
  // chunk data headers
  
  for (int i = 0; i < _mhdr.n_chunks; i++) {
    if (_chunks[i]->_write_data(outfile, writeOffset, nextOffset)) {
      _errStr += "ERROR - Mdvx::writeToPath\n";
      _errStr += "  Path: ";
      _errStr += fullOutPath;
      _errStr += "\n";
      return -1;
    }
    writeOffset = nextOffset;
  }

  // set the constant values in the master header

  _mhdr.record_len1 = sizeof(Mdvx::master_header_t) - (2 * sizeof(si32));
  _mhdr.struct_id = Mdvx::MASTER_HEAD_MAGIC_COOKIE;
  _mhdr.revision_number = 1;
  _mhdr.record_len2 = _mhdr.record_len1;
  
  // set the offsets in the master header

  _mhdr.field_hdr_offset = sizeof(master_header_t);
  _mhdr.vlevel_hdr_offset =
    _mhdr.field_hdr_offset + _mhdr.n_fields * sizeof(field_header_t);
  _mhdr.chunk_hdr_offset =
    _mhdr.vlevel_hdr_offset + _mhdr.n_fields * sizeof(vlevel_header_t);

  // write the master header

  if (_write_master_header(outfile)) {
    _errStr += "ERROR - Mdvx::writeToPath\n";
    _errStr += "  Path: ";
    _errStr += fullOutPath;
    _errStr += "\n";
    return -1;
  }

  // write the field headers

  for (int i = 0; i < _mhdr.n_fields; i++) {
    if (_write_field_header(i, outfile)) {
      _errStr += "ERROR - Mdvx::writeToPath\n";
      _errStr += "  Path: ";
      _errStr += fullOutPath;
      _errStr += "\n";
      return -1;
    }
  }

  // write the vlevel headers
  
  for (int i = 0; i < _mhdr.n_fields; i++) {
    if (_write_vlevel_header(i, outfile)) {
      _errStr += "ERROR - Mdvx::writeToPath\n";
      _errStr += "  Path: ";
      _errStr += fullOutPath;
      _errStr += "\n";
      return -1;
    }
  }

  // write the chunk headers

  for (int i = 0; i < _mhdr.n_chunks; i++) {
    if (_write_chunk_header(i, outfile)) {
      _errStr += "ERROR - Mdvx::writeToPath\n";
      _errStr += "  Path: ";
      _errStr += fullOutPath;
      _errStr += "\n";
      return -1;
    }
  }

  // close the output file

  outfile.fclose();

  // rename the tmp to final output file path

  if (rename(tmpPath.c_str(), fullOutPath.c_str())) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::writeToPath\n";
    _errStr += "  Cannot rename tmp file: ";
    _errStr += tmpPath;
    _errStr += " to: ";
    _errStr += fullOutPath;
    _errStr += "\n  ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  outfile.clearRemoveOnDestruct();
  
  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Write to buffer
//
// Write Mdvx object to a buffer as if written to file.

void Mdvx::writeToBuffer(MemBuf &buf) const

{

  updateMasterHeader();
  
  if (_debug) {
    cerr << "Mdvx - writing object to buffer." << endl;
  }

  // compute master header offsets
  
  _mhdr.field_hdr_offset = sizeof(master_header_t);
  _mhdr.vlevel_hdr_offset =
    _mhdr.field_hdr_offset + _fields.size() * sizeof(field_header_t);
  _mhdr.chunk_hdr_offset =
    _mhdr.vlevel_hdr_offset + _fields.size() * sizeof(vlevel_header_t);
  
  // add the headers to the buffer - they will be swapped later
  
  buf.free();
  buf.add(&_mhdr, sizeof(_mhdr));
  for (size_t i = 0; i < _fields.size(); i++) {
    buf.add(&_fields[i]->_fhdr, sizeof(field_header_t));
  }
  for (size_t i = 0; i < _fields.size(); i++) {
    buf.add(&_fields[i]->_vhdr, sizeof(vlevel_header_t));
  }
  for (size_t i = 0; i < _chunks.size(); i++) {
    buf.add(&_chunks[i]->_chdr, sizeof(chunk_header_t));
  }
  
  // set field offsets, write the field data, swapping as appropriate
  
  for (size_t i = 0; i < _fields.size(); i++) {

    // leading FORTRAN rec len
    
    int size = _fields[i]->getVolLen();
    si32 BEsize = BE_from_si32(size);
    buf.add(&BEsize, sizeof(si32));

    // data

    int offset = buf.getLen();
    buf.add(_fields[i]->getVol(), size);

    // trailing FORTRAN rec len

    buf.add(&BEsize, sizeof(si32));

    // set field_data_offset

    field_header_t *fhdr = (field_header_t *)
      ((char *) buf.getPtr() + _mhdr.field_hdr_offset +
       i * sizeof(field_header_t));
    fhdr->field_data_offset = offset;
    
    // swap data
    
    if (fhdr->compression_type == COMPRESSION_NONE) {
      void *vol = ((char *) buf.getPtr() + offset);
      switch (fhdr->encoding_type) {
      case Mdvx::ENCODING_INT8:
      case Mdvx::ENCODING_RGBA32:
	// no need to swap byte data
	break;
      case Mdvx::ENCODING_INT16:
	BE_from_array_16(vol, size);
	break;
      case Mdvx::ENCODING_FLOAT32:
	BE_from_array_32(vol, size);
	break;
      }
    }
  }

  // set chunk offsets, write chunk data

  for (size_t i = 0; i < _chunks.size(); i++) {

    // leading FORTRAN rec len
    
    int size = _chunks[i]->getSize();
    si32 BEsize = BE_from_si32(size);
    buf.add(&BEsize, sizeof(si32));

    // data

    int offset = buf.getLen();
    buf.add(_chunks[i]->getData(), size);

    // trailing FORTRAN rec len

    buf.add(&BEsize, sizeof(si32));

    // set chunk_data_offset

    chunk_header_t *chdr = (chunk_header_t *)
      ((char *) buf.getPtr() + _mhdr.chunk_hdr_offset +
       i * sizeof(chunk_header_t));
    chdr->chunk_data_offset = offset;

  }

  // go back and swap the headers

  master_header_t *mhdr = (master_header_t *) buf.getPtr();
  master_header_to_BE(*mhdr);
  field_header_t *fhdrs = (field_header_t *)
    ((char *) buf.getPtr() + _mhdr.field_hdr_offset);
  for (size_t i = 0; i < _fields.size(); i++) {
    field_header_to_BE(fhdrs[i]);
  }
  vlevel_header_t *vhdrs = (vlevel_header_t *)
    ((char *) buf.getPtr() + _mhdr.vlevel_hdr_offset);
  for (size_t i = 0; i < _fields.size(); i++) {
    vlevel_header_to_BE(vhdrs[i]);
  }
  chunk_header_t *chdrs = (chunk_header_t *)
    ((char *) buf.getPtr() + _mhdr.chunk_hdr_offset);
  for (size_t i = 0; i < _chunks.size(); i++) {
    chunk_header_to_BE(chdrs[i]);
  }
  
}

///////////////////////////////////////////////////////
// Write to path using the buffer routine.
//
// This is intended for testing only.
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int Mdvx::writeUsingBuf(const string &output_path) const

{

  clearErrStr();
  MemBuf buf;
  writeToBuffer(buf);
  
  string fullOutPath;
  RapDataDir.fillPath(output_path, fullOutPath);
  _pathInUse = fullOutPath;

  if (_debug) {
    cerr << "Mdvx::writeUsingBuf to path: " << fullOutPath << endl;
  }

  // remove compressed file if it exists
  
  ta_remove_compressed(fullOutPath.c_str());

  // open tmp file

  Path outPath(fullOutPath);
  outPath.makeDirRecurse();
  string tmpPath = outPath.computeTmpPath();
  
  TaFile outfile;
  outfile.setRemoveOnDestruct();
  
  if (outfile.fopen(tmpPath.c_str(), "wb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::writeUsingBuf\n";
    _errStr += "  Cannot open file for writing: ";
    _errStr += tmpPath;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // write the buffer

  int size = buf.getLen();
  if (outfile.fwrite(buf.getPtr(), 1, size) != size) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::writeUsingBuf\n";
    _errStr += "  Cannot write to path: ";
    _errStr += tmpPath;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  // close the output file

  outfile.fclose();

  // rename the tmp to final output file path

  if (rename(tmpPath.c_str(), fullOutPath.c_str())) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::writeUsingBuf\n";
    _errStr += "  Cannot rename tmp file: ";
    _errStr += tmpPath;
    _errStr += " to: ";
    _errStr += fullOutPath;
    _errStr += "\n  ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  outfile.clearRemoveOnDestruct();

  return 0;

}

//////////////////////
// print write options

void Mdvx::printWriteOptions(ostream &out)

{

  out << "Mdvx write options" << endl;
  out << "------------------" << endl;

  out << "  writeLdataInfo: " << (_writeLdataInfo?"T":"F") << endl;
  out << "  writeAsForecast: " << (_writeAsForecast?"T":"F") << endl;
  out << "  ifForecastWriteAsForecast: " << (_ifForecastWriteAsForecast?"T":"F") << endl;
  out << "  writeFormat: " << format2Str(_writeFormat) << endl;
  out << "  writeUsingExtendedPath: " << (_writeAsForecast?"T":"F") << endl;
  out << "  writeAddYearSubdir: " << (_writeAddYearSubdir?"T":"F") << endl;
  
}

//////////////////////////////////////////////////////////////////////////
// Write master header at top of file.
// 
// Returns 0 on success, -1 on failure.

int Mdvx::_write_master_header(TaFile &outfile) const

{
  
  // Move to the beginning of the file.

  if (outfile.fseek(0, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_master_header\n";
    _errStr += "Cannot seek to start to write master header\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // Convert local copy to BE

  master_header_t mhdr_be = _mhdr;
  Mdvx::master_header_to_BE(mhdr_be);
  
  // Write the header

  if (outfile.fwrite(&mhdr_be, sizeof(master_header_t), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_master_header\n";
    _errStr += "Cannot write master header\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Write a field header to an open file.
//
// Returns 0 on success, -1 on failure.

int Mdvx::_write_field_header(const int field_num,
			      TaFile &outfile) const

{

  // Move to the appropriate offset

  int offset = _mhdr.field_hdr_offset +
    field_num * sizeof(field_header_t);

  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_field_header\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to field header offset: %d\n", offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // make sure data dimension is set correctly

  field_header_t fhdr_be = _fields[field_num]->_fhdr;
  if (fhdr_be.nz == 1) {
    fhdr_be.data_dimension = 2;
  } else {
    fhdr_be.data_dimension = 3;
  }

  // set file-based properties

  fhdr_be.zoom_clipped = false;
  fhdr_be.zoom_no_overlap = false;

  // convert to BE

  Mdvx::field_header_to_BE(fhdr_be);
  
  // Write the header to the output file.
  
  if (outfile.fwrite(&fhdr_be, sizeof(field_header_t), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_field_header\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot write field header for field: %d\n", field_num);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Write a vlevel header to an open file.
// 
// Returns 0 on success, -1 on failure.

int Mdvx::_write_vlevel_header(const int field_num,
			       TaFile &outfile) const

{

  // Move to the appropriate offset

  int offset = _mhdr.vlevel_hdr_offset +
    field_num * sizeof(vlevel_header_t);
  
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_vlevel_header\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to vlevel header offset: %d\n", offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // Convert local copy to BE

  vlevel_header_t vhdr_be = _fields[field_num]->_vhdr;
  Mdvx::vlevel_header_to_BE(vhdr_be);
  
  // Write the header to the output file.
  
  if (outfile.fwrite(&vhdr_be, sizeof(vlevel_header_t), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_vlevel_header\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot write vlevel header for field: %d\n", field_num);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// write chunk header to open file
// 
// Returns 0 on success, -1 on failure.
 
int Mdvx::_write_chunk_header(const int chunk_num,
			      TaFile &outfile) const

{

  // Move to the appropriate offset
  
  int offset = _mhdr.chunk_hdr_offset +
    chunk_num * sizeof(chunk_header_t);
  
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_chunk_header\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to chunk header offset: %d\n", offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // Convert local copy to BE

  chunk_header_t chdr_be = _chunks[chunk_num]->_chdr;
  Mdvx::chunk_header_to_BE(chdr_be);
  
  // Write the header to the output file.
  
  if (outfile.fwrite(&chdr_be, sizeof(chunk_header_t), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_write_chunk_header\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot write chunk header for chunk: %d\n", chunk_num);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// check the environment for directives related to the write

void Mdvx::_checkEnvBeforeWrite() const
{

  // check environment for write control

  char *writeFormatStr = getenv("MDV_WRITE_FORMAT");
  if (writeFormatStr != NULL) {
    if (!strcmp(writeFormatStr, "FORMAT_MDV")) {
      _writeFormat = FORMAT_MDV;
    } else if (!strcmp(writeFormatStr, "FORMAT_XML")) {
      _writeFormat = FORMAT_XML;
    } else if (!strcmp(writeFormatStr, "FORMAT_NCF")) {
      _writeFormat = FORMAT_NCF;
    }
  }

  char *extendedPathsStr = getenv("MDV_WRITE_USING_EXTENDED_PATHS");
  if (extendedPathsStr != NULL) {
    if (!strcasecmp(extendedPathsStr, "TRUE")) {
      _useExtendedPaths = true;
    }
  }

  char *addYearSubdir = getenv("MDV_WRITE_ADD_YEAR_SUBDIR");
  if (addYearSubdir != NULL) {
    if (!strcasecmp(addYearSubdir, "TRUE")) {
      _writeAddYearSubdir = true;
    }
  }

}


//////////////////////////////////////////////////////
// compute output name and path for file write

void Mdvx::_computeOutputPath(const string &outputDir,
                              string &outputName,
                              string &outputPath,
                              bool &writeAsForecast) const
{

  // check environment for write control directives

  _checkEnvBeforeWrite();
  
  // compute output path

  char yearSubdir[MAX_PATH_LEN];
  char outputBase[MAX_PATH_LEN];
  int forecastDelta = getForecastLeadSecs();
  writeAsForecast = _writeAsForecast;
  
  if (_ifForecastWriteAsForecast) {
    if (_currentFormat == FORMAT_NCF) {
      if (_ncfIsForecast) {
        writeAsForecast = true;
      }
    } else {
      if (_mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
          _mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
        writeAsForecast = true;
      }
    }
  }

  if (writeAsForecast) {
   
    if (_mhdr.data_collection_type != Mdvx::DATA_FORECAST && 
        _mhdr.data_collection_type != Mdvx::DATA_EXTRAPOLATED) {
      _mhdr.data_collection_type = Mdvx::DATA_FORECAST;
    }
 
    date_time_t genTime;
    genTime.unix_time = getGenTime();
    uconvert_from_utime(&genTime);
    
    sprintf(yearSubdir, "%.4d", genTime.year);

    if (!_useExtendedPaths) {
      sprintf(outputBase, "%.4d%.2d%.2d%sg_%.2d%.2d%.2d%sf_%.8d",
              genTime.year, genTime.month, genTime.day,
              PATH_DELIM, genTime.hour, genTime.min, genTime.sec,
              PATH_DELIM, forecastDelta);
    } else {
      sprintf(outputBase,
              "%.4d%.2d%.2d%s"
              "g_%.2d%.2d%.2d%s"
              "%.4d%.2d%.2d_g_%.2d%.2d%.2d_f_%.8d",
              genTime.year, genTime.month, genTime.day, PATH_DELIM,
              genTime.hour, genTime.min, genTime.sec, PATH_DELIM,
              genTime.year, genTime.month, genTime.day,
              genTime.hour, genTime.min, genTime.sec,
              forecastDelta);
    }

  } else {

    date_time_t validTime;
    validTime.unix_time = getValidTime();
    uconvert_from_utime(&validTime);
    sprintf(yearSubdir, "%.4d", validTime.year);
    if (!_useExtendedPaths) {
      sprintf(outputBase, "%.4d%.2d%.2d%s%.2d%.2d%.2d",
              validTime.year, validTime.month, validTime.day,
              PATH_DELIM, validTime.hour, validTime.min, validTime.sec);
    } else {
      sprintf(outputBase,
              "%.4d%.2d%.2d%s"
              "%.4d%.2d%.2d_%.2d%.2d%.2d",
              validTime.year, validTime.month, validTime.day, PATH_DELIM,
              validTime.year, validTime.month, validTime.day,
              validTime.hour, validTime.min, validTime.sec);
    }

  }

  outputName.clear();
  if (_writeAddYearSubdir) {
    outputName += yearSubdir;
    outputName += PATH_DELIM;
  }
  outputName += outputBase;
  outputName += ".mdv";

  outputPath = outputDir;
  outputPath += PATH_DELIM;
  outputPath += outputName;

}
  
