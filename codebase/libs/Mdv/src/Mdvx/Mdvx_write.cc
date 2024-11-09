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
#include <didss/RapDataDir.hh>
#include <dsserver/DsLdataInfo.hh>
#include <dsserver/DmapAccess.hh>
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

// default is netcdf

void Mdvx::clearWriteFormat()
{
  _writeFormat = FORMAT_NCF;
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

/////////////////////////////////////////////////////////////////////////
// Get the size of an MDV file that would be written
// using 32-bit headers

si64 Mdvx::getWriteLen32() const
  
{

  si64 len = 0;
  len += sizeof(master_header_32_t);
  
  for (size_t i = 0; i < _fields.size(); i++) {
    len += sizeof(field_header_32_t);
    len += sizeof(vlevel_header_32_t);
    field_header_64_t fhdr = _fields[i]->_fhdr;
    si64 fieldLen = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
    len += fieldLen;
  }

  for (size_t i = 0; i < _chunks.size(); i++) {
    len += sizeof(chunk_header_32_t);
    len += _chunks[i]->_chdr.size;
  }

  return len;

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

//////////////////////////////////////////////////////
// Write to directory
//
// File path is computed - see setWriteAsForecast().
// _latest_data_info file is written as appropriate - 
//    see setWriteLdataInfo().
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int Mdvx::writeToDir(const string &outputDir)
{

  if (_debug) {
    cerr << "DEBUG - Mdvx::writeToDir" << endl;
  }

  clearErrStr();
  updateMasterHeader();

  // if the internal format is a NetCDF buffer
  // force NCF write

  if (_internalFormat == FORMAT_NCF) {
    _writeFormat = FORMAT_NCF;
  }
  
  // compute output name and path

  string outputName, outputPath;
  bool writeAsForecast;
  _computeOutputPath(outputDir, outputName, outputPath, writeAsForecast);
  _pathInUse = outputPath;
  
  // perform the write
  
  if (writeToPath(outputPath)) {
    _errStr += "ERROR - Mdvx::writeToDir\n";
    return -1;
  }
  
  // write the latest data info file

  if (_writeLdataInfo) {

    Path opath(outputPath);
    LdataInfo ldata;
    ldata.setDir(outputDir);
    ldata.setDebug(_debug);
    ldata.setDataFileExt(opath.getExt());
    ldata.setDataType(opath.getExt());
    ldata.setWriter(_appName);
    string relName = outputName;
    ldata.setRelDataPath(relName);
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

int Mdvx::writeToPath(const string &outputPath)

{

  if (_debug) {
    cerr << "DEBUG - Mdvx::writeToPath" << endl;
    printFormats(cerr, true);
  }

  // init
  
  clearErrStr();

  // update the master header with the write time

  updateMasterHeader();
  time_t now = time(NULL);
  _mhdr.time_written = (si32) now;

  // check the environment vars for write details

  _checkEnvBeforeWrite();
  
  // compute 32-bit MDV write length
  
  si64 writeLen = getWriteLen32();

  // get the date of the data
  
  DateTime dtime(_mhdr.time_centroid);

  // force NetCDF output instaed of MDV for large files
  // or data beyond a specified date (2025)
  
  if (_internalFormat == FORMAT_NCF ||
      writeLen >= SI32_MAX ||
      dtime.getYear() >= 2025) {
    _writeFormat = FORMAT_NCF;
  }
  
  // if already converted into netcdf buffer, write it out directly
  // from the buffer

  if (_internalFormat == FORMAT_NCF) {
    return _writeNcfBufToFile(outputPath);
  }
  
  // if to be written as XML, call XML method
  
  if (_writeFormat == FORMAT_XML) {
    return _writeAsXml(outputPath);
  }
    
  // if to be written as NCF, call NCF method
  
  if (_writeFormat == FORMAT_NCF) {
    return _writeAsNcf(outputPath);
  }

   // otherwise write as MDV - legacy 32-bit headers

  return _writeAsMdv32(outputPath);

}

////////////////////////////////////////////////
// write as MDV
// legacy 32-bit headers
// returns 0 on success, -1 on failure

int Mdvx::_writeAsMdv32(const string &outputPath)
  
{
  
  if (_debug) {
    cerr << "DEBUG - Mdvx::_writeAsMdv - writing to path: " << outputPath << endl;
  }
  
  // remove gzipped file if it already exists
  
  ta_remove_compressed(outputPath.c_str());
  
  // open tmp file

  Path outPath(outputPath);
  outPath.makeDirRecurse();
  string tmpPath = outPath.computeTmpPath();
  
  TaFile outfile;
  outfile.setRemoveOnDestruct();
  
  if (outfile.fopen(tmpPath.c_str(), "wb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
    _errStr += "  Cannot open file for writing: ";
    _errStr += tmpPath;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // compute offset to start of field data

  int64_t writeOffset =
    sizeof(master_header_32_t) +
    _mhdr.n_fields * (sizeof(field_header_32_t) + sizeof(vlevel_header_32_t)) +
    _mhdr.n_chunks * sizeof(chunk_header_32_t);

  int64_t nextOffset = writeOffset;

  // write field data - this also sets the field data offset in
  // the field headers
  
  for (int i = 0; i < _mhdr.n_fields; i++) {

    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Mdvx::_writeAsMdv32");
    }

    if (_fields[i]->_write_volume(outfile, writeOffset, nextOffset)) {

      _errStr += _fields[i]->getErrStr();
      char field_string[32];
      snprintf(field_string, 31, "%d", i);
      
      _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
      _errStr += string("  Error writing volume for field ")
	+ field_string + "\n";
      _errStr += "  Path: " + outputPath + "\n";
      return -1;
    }
    writeOffset = nextOffset;
  }
  
  // write chunk data - this also sets the chunk data offset in the
  // chunk data headers

  for (int i = 0; i < _mhdr.n_chunks; i++) {
    if (_chunks[i]->_write_data(outfile, writeOffset, nextOffset)) {
      _errStr += _chunks[i]->getErrStr();
      _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
    writeOffset = nextOffset;
  }

  // write the master header

  if (_writeMasterHeader32(outfile)) {
    _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
    _errStr += "  Path: ";
    _errStr += outputPath;
    _errStr += "\n";
    return -1;
  }

  // write the field headers

  for (int i = 0; i < _mhdr.n_fields; i++) {
    if (_writeFieldHeader32(i, outfile)) {
      _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
  }

  // write the vlevel headers
  
  for (int i = 0; i < _mhdr.n_fields; i++) {
    if (_writeVlevelHeader32(i, outfile)) {
      _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
  }

  // write the chunk headers

  for (int i = 0; i < _mhdr.n_chunks; i++) {
    if (_writeChunkHeader32(i, outfile)) {
      _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
  }

  // close the output file

  outfile.fclose();

  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), outputPath.c_str())) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeAsMdv32\n";
    _errStr += "  Cannot rename tmp file: ";
    _errStr += tmpPath;
    _errStr += " to: ";
    _errStr += outputPath;
    _errStr += "\n  ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  outfile.clearRemoveOnDestruct();
  
  return 0;

}

///////////////////////////////////////////////////////
// write as MDV
// 64-bit headers
// This is not normally used.
// For 64-bit support, we write out as NetCDF instead.
// returns 0 on success, -1 on failure

int Mdvx::_writeAsMdv64(const string &outputPath)
  
{
  
  if (_debug) {
    cerr << "DEBUG - Mdvx::_writeAsMdv - writing to path: " << outputPath << endl;
  }
  
  // remove gzipped file if it already exists
  
  ta_remove_compressed(outputPath.c_str());
  
  // open tmp file

  Path outPath(outputPath);
  outPath.makeDirRecurse();
  string tmpPath = outPath.computeTmpPath();
  
  TaFile outfile;
  outfile.setRemoveOnDestruct();
  
  if (outfile.fopen(tmpPath.c_str(), "wb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
    _errStr += "  Cannot open file for writing: ";
    _errStr += tmpPath;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // compute offset to start of field data

  int64_t writeOffset =
    sizeof(master_header_64_t) +
    _mhdr.n_fields * (sizeof(field_header_64_t) + sizeof(vlevel_header_64_t)) +
    _mhdr.n_chunks * sizeof(chunk_header_64_t);

  int64_t nextOffset = writeOffset;

  // write field data - this also sets the field data offset in
  // the field headers
  
  for (int i = 0; i < _mhdr.n_fields; i++) {

    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Mdvx::_writeAsMdv64");
    }

    if (_fields[i]->_write_volume(outfile, writeOffset, nextOffset)) {

      _errStr += _fields[i]->getErrStr();
      char field_string[32];
      snprintf(field_string, 31, "%d", i);
      
      _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
      _errStr += string("  Error writing volume for field ")
	+ field_string + "\n";
      _errStr += "  Path: " + outputPath + "\n";
      return -1;
    }
    writeOffset = nextOffset;
  }
  
  // write chunk data - this also sets the chunk data offset in the
  // chunk data headers

  for (int i = 0; i < _mhdr.n_chunks; i++) {
    if (_chunks[i]->_write_data(outfile, writeOffset, nextOffset)) {
      _errStr += _chunks[i]->getErrStr();
      _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
    writeOffset = nextOffset;
  }

  // write the master header

  if (_writeMasterHeader64(outfile)) {
    _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
    _errStr += "  Path: ";
    _errStr += outputPath;
    _errStr += "\n";
    return -1;
  }

  // write the field headers

  for (int i = 0; i < _mhdr.n_fields; i++) {
    if (_writeFieldHeader64(i, outfile)) {
      _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
  }

  // write the vlevel headers
  
  for (int i = 0; i < _mhdr.n_fields; i++) {
    if (_writeVlevelHeader64(i, outfile)) {
      _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
  }

  // write the chunk headers

  for (int i = 0; i < _mhdr.n_chunks; i++) {
    if (_writeChunkHeader64(i, outfile)) {
      _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
      _errStr += "  Path: ";
      _errStr += outputPath;
      _errStr += "\n";
      return -1;
    }
  }

  // close the output file

  outfile.fclose();

  // rename the tmp to final output file path
  
  if (rename(tmpPath.c_str(), outputPath.c_str())) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeAsMdv64\n";
    _errStr += "  Cannot rename tmp file: ";
    _errStr += tmpPath;
    _errStr += " to: ";
    _errStr += outputPath;
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
// Use legacy 32-bit headers.
// Write Mdvx object to a buffer as if written to file.

void Mdvx::writeToBuffer(MemBuf &buf) const

{
  writeToBuffer32(buf);
}

/////////////////////////////////////////////////////////////////////////
// Write to 64-bit buffer
//
// Write Mdvx object to a buffer as if written to file.

void Mdvx::writeToBuffer64(MemBuf &buf) const

{

  updateMasterHeader();
  
  if (_debug) {
    cerr << "Mdvx - writing object to buffer - 64-bit headers." << endl;
  }

  // compute master header offsets
  
  _mhdr.field_hdr_offset = sizeof(master_header_64_t);
  _mhdr.vlevel_hdr_offset =
    _mhdr.field_hdr_offset + _fields.size() * sizeof(field_header_64_t);
  _mhdr.chunk_hdr_offset =
    _mhdr.vlevel_hdr_offset + _fields.size() * sizeof(vlevel_header_64_t);
  
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
    
    int64_t size = _fields[i]->getVolLen();
    si32 BEsize = BE_from_si32(size);
    buf.add(&BEsize, sizeof(si32));

    // data

    int64_t offset = buf.getLen();
    buf.add(_fields[i]->getVol(), size);

    // trailing FORTRAN rec len

    buf.add(&BEsize, sizeof(si32));

    // set field_data_offset

    field_header_64_t *fhdr = (field_header_64_t *)
      ((char *) buf.getPtr() + _mhdr.field_hdr_offset +
       i * sizeof(field_header_64_t));
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
    
    int64_t size = _chunks[i]->getSize();
    si32 BEsize = BE_from_si32(size);
    buf.add(&BEsize, sizeof(si32));

    // data

    int64_t offset = buf.getLen();
    buf.add(_chunks[i]->getData(), size);

    // trailing FORTRAN rec len

    buf.add(&BEsize, sizeof(si32));

    // set chunk_data_offset

    chunk_header_64_t *chdr = (chunk_header_64_t *)
      ((char *) buf.getPtr() + _mhdr.chunk_hdr_offset +
       i * sizeof(chunk_header_64_t));
    chdr->chunk_data_offset = offset;

  }

  // go back and swap the headers

  master_header_64_t *mhdr = (master_header_64_t *) buf.getPtr();
  master_header_to_BE(*mhdr);
  field_header_64_t *fhdrs = (field_header_64_t *)
    ((char *) buf.getPtr() + _mhdr.field_hdr_offset);
  for (size_t i = 0; i < _fields.size(); i++) {
    field_header_to_BE(fhdrs[i]);
  }
  vlevel_header_64_t *vhdrs = (vlevel_header_64_t *)
    ((char *) buf.getPtr() + _mhdr.vlevel_hdr_offset);
  for (size_t i = 0; i < _fields.size(); i++) {
    vlevel_header_to_BE(vhdrs[i]);
  }
  chunk_header_64_t *chdrs = (chunk_header_64_t *)
    ((char *) buf.getPtr() + _mhdr.chunk_hdr_offset);
  for (size_t i = 0; i < _chunks.size(); i++) {
    chunk_header_to_BE(chdrs[i]);
  }
  
}

/////////////////////////////////////////////////////////////////////////
// Write to buffer - 32-bit headers
//
// Write Mdvx object to a buffer as if written to file.

void Mdvx::writeToBuffer32(MemBuf &buf) const

{

  updateMasterHeader();
  
  if (_debug) {
    cerr << "Mdvx - writing object to buffer - 32-bit headers." << endl;
  }

  // compute master header offsets

  master_header_32_t mhdr32;
  _copyMasterHeader64to32(_mhdr, mhdr32);
  
  mhdr32.field_hdr_offset = sizeof(master_header_32_t);
  mhdr32.vlevel_hdr_offset =
    mhdr32.field_hdr_offset + _fields.size() * sizeof(field_header_32_t);
  mhdr32.chunk_hdr_offset =
    mhdr32.vlevel_hdr_offset + _fields.size() * sizeof(vlevel_header_32_t);
  
  // add the headers to the buffer - they will be swapped later
  
  buf.free();
  buf.add(&mhdr32, sizeof(mhdr32));
  for (size_t i = 0; i < _fields.size(); i++) {
    field_header_32_t fhdr32;
    _copyFieldHeader64to32(_fields[i]->_fhdr, fhdr32);
    buf.add(&fhdr32, sizeof(fhdr32));
  }
  for (size_t i = 0; i < _fields.size(); i++) {
    vlevel_header_32_t vhdr32;
    _copyVlevelHeader64to32(_fields[i]->_vhdr, vhdr32);
    buf.add(&vhdr32, sizeof(vhdr32));
  }
  for (size_t i = 0; i < _chunks.size(); i++) {
    chunk_header_32_t chdr32;
    _copyChunkHeader64to32(_chunks[i]->_chdr, chdr32);
    buf.add(&chdr32, sizeof(chdr32));
  }
  
  // set field offsets, write the field data, swapping as appropriate
  
  for (size_t i = 0; i < _fields.size(); i++) {

    // leading FORTRAN rec len
    
    int64_t size = _fields[i]->getVolLen();
    si32 BEsize = BE_from_si32(size);
    buf.add(&BEsize, sizeof(si32));
    
    // data
    
    int64_t offset = buf.getLen();
    buf.add(_fields[i]->getVol(), size);

    // trailing FORTRAN rec len

    buf.add(&BEsize, sizeof(si32));

    // set field_data_offset

    field_header_32_t *fhdr32 = (field_header_32_t *)
      ((char *) buf.getPtr() + mhdr32.field_hdr_offset +
       i * sizeof(field_header_32_t));
    fhdr32->field_data_offset = offset;
    
    // swap data
    
    if (fhdr32->compression_type == COMPRESSION_NONE) {
      void *vol = ((char *) buf.getPtr() + offset);
      switch (fhdr32->encoding_type) {
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
    
    int64_t size = _chunks[i]->getSize();
    si32 BEsize = BE_from_si32(size);
    buf.add(&BEsize, sizeof(si32));

    // data

    int64_t offset = buf.getLen();
    buf.add(_chunks[i]->getData(), size);

    // trailing FORTRAN rec len

    buf.add(&BEsize, sizeof(si32));

    // set chunk_data_offset

    chunk_header_32_t *chdr32 = (chunk_header_32_t *)
      ((char *) buf.getPtr() + mhdr32.chunk_hdr_offset +
       i * sizeof(chunk_header_32_t));
    chdr32->chunk_data_offset = offset;

  }

  // go back and swap the headers

  master_header_32_t *mhdr = (master_header_32_t *) buf.getPtr();
  master_header_to_BE_32(*mhdr);
  field_header_32_t *fhdrs = (field_header_32_t *)
    ((char *) buf.getPtr() + mhdr32.field_hdr_offset);
  for (size_t i = 0; i < _fields.size(); i++) {
    field_header_to_BE_32(fhdrs[i]);
  }
  vlevel_header_32_t *vhdrs = (vlevel_header_32_t *)
    ((char *) buf.getPtr() + mhdr32.vlevel_hdr_offset);
  for (size_t i = 0; i < _fields.size(); i++) {
    vlevel_header_to_BE_32(vhdrs[i]);
  }
  chunk_header_32_t *chdrs = (chunk_header_32_t *)
    ((char *) buf.getPtr() + mhdr32.chunk_hdr_offset);
  for (size_t i = 0; i < _chunks.size(); i++) {
    chunk_header_to_BE_32(chdrs[i]);
  }
  
}

///////////////////////////////////////////////////////
// Write to path using the buffer routine.
//
// This is intended for testing only.
//
// Returns 0 on success, -1 on error.
// getErrStr() retrieves the error string.

int Mdvx::writeUsingBuf(const string &outputPath) const

{

  clearErrStr();

  // load up membuf buffer with Mdv as though written to file

  MemBuf buf;
  writeToBuffer(buf);
  
  if (_debug) {
    cerr << "Mdvx::writeUsingBuf to path: " << outputPath << endl;
  }

  // remove compressed file if it exists
  
  ta_remove_compressed(outputPath.c_str());

  // open tmp file

  Path outPath(outputPath);
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

  size_t fsize = buf.getLen();
  if (outfile.fwrite(buf.getPtr(), 1, fsize) != fsize) {
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

  if (rename(tmpPath.c_str(), outputPath.c_str())) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::writeUsingBuf\n";
    _errStr += "  Cannot rename tmp file: ";
    _errStr += tmpPath;
    _errStr += " to: ";
    _errStr += outputPath;
    _errStr += "\n  ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  outfile.clearRemoveOnDestruct();

  return 0;

}

//////////////////////////////////////////////////////////////////////////
// Write master header at top of file.
// Legacy 32-bit headers.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeMasterHeader32(TaFile &outfile) const

{
  
  // Move to the beginning of the file.

  if (outfile.fseek(0, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeMasterHeader32\n";
    _errStr += "Cannot seek to start to write master header\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // set the offsets in the master header
  
  _mhdr.field_hdr_offset = sizeof(master_header_32_t);
  _mhdr.vlevel_hdr_offset =
    _mhdr.field_hdr_offset + _mhdr.n_fields * sizeof(field_header_32_t);
  _mhdr.chunk_hdr_offset =
    _mhdr.vlevel_hdr_offset + _mhdr.n_fields * sizeof(vlevel_header_32_t);
    
  // create and fill 32-bit header
  
  master_header_32_t mhdr32;
  _copyMasterHeader64to32(_mhdr, mhdr32);
    
  // Convert to BE
  
  Mdvx::master_header_to_BE_32(mhdr32);
    
  // Write the header
  
  if (outfile.fwrite(&mhdr32, sizeof(mhdr32), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeMasterHeader32\n";
    _errStr += "Cannot write master header\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////////////////
// Write master header at top of file.
// 64-bit headers.
// This is not actually used.
// For 64-bit, we force a NetCDF write instead.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeMasterHeader64(TaFile &outfile) const

{
  
  // Move to the beginning of the file.

  if (outfile.fseek(0, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeMasterHeader64\n";
    _errStr += "Cannot seek to start to write master header\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // 64-bit headers

  // set the offsets in the master header
  
  _mhdr.field_hdr_offset = sizeof(master_header_64_t);
  _mhdr.vlevel_hdr_offset =
    _mhdr.field_hdr_offset + _mhdr.n_fields * sizeof(field_header_64_t);
  _mhdr.chunk_hdr_offset =
    _mhdr.vlevel_hdr_offset + _mhdr.n_fields * sizeof(vlevel_header_64_t);
  
  // set the constant values in the master header
  
  _mhdr.record_len1 = sizeof(Mdvx::master_header_64_t) - (2 * sizeof(si32));
  _mhdr.struct_id = Mdvx::MASTER_HEAD_MAGIC_COOKIE_64;
  _mhdr.revision_number = 2;
  _mhdr.record_len2 = _mhdr.record_len1;
  
  // Make local copy, convert to BE
  
  master_header_t mhdr = _mhdr;
  Mdvx::master_header_to_BE(mhdr);
  
  // Write the header
  
  if (outfile.fwrite(&mhdr, sizeof(mhdr), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeMasterHeader64\n";
    _errStr += "Cannot write master header\n";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Write a field header to an open file.
// Legacy 32-bit headers.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeFieldHeader32(const int field_num,
                              TaFile &outfile) const

{
  
  // Move to the appropriate offset
  
  int64_t offset = _mhdr.field_hdr_offset + field_num * sizeof(field_header_32_t);

  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeFieldHeader32\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to field header offset: %lld\n",
            (long long) offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // make local copy of field header

  field_header_t fhdr = _fields[field_num]->_fhdr;
  if (fhdr.nz == 1) {
    fhdr.data_dimension = 2;
  } else {
    fhdr.data_dimension = 3;
  }

  // set file-based properties
  
  fhdr.zoom_clipped = false;
  fhdr.zoom_no_overlap = false;

  // set 32-bit header
  
  field_header_32_t fhdr32;
  _copyFieldHeader64to32(fhdr, fhdr32);
  
  // convert to BE
  
  Mdvx::field_header_to_BE_32(fhdr32);
  
  // Write the header to the output file.
  
  if (outfile.fwrite(&fhdr32, sizeof(fhdr32), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeFieldHeader32\n";
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
// Write a field header to an open file.
// Legacy 32-bit headers.
// This is not actually used.
// For 64-bit, we force a NetCDF write instead.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeFieldHeader64(const int field_num,
                              TaFile &outfile) const

{

  // Move to the appropriate offset
  
  int64_t offset = _mhdr.field_hdr_offset + field_num * sizeof(field_header_64_t);
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeFieldHeader64\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to field header offset: %lld\n",
            (long long) offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // make local copy of field header
  
  field_header_64_t fhdr = _fields[field_num]->_fhdr;
  if (fhdr.nz == 1) {
    fhdr.data_dimension = 2;
  } else {
    fhdr.data_dimension = 3;
  }

  // set file-based properties
  
  fhdr.zoom_clipped = false;
  fhdr.zoom_no_overlap = false;

  // convert to BE
  
  Mdvx::field_header_to_BE(fhdr);
    
  // Write the header to the output file.
  
  if (outfile.fwrite(&fhdr, sizeof(fhdr), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeFieldHeader64\n";
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
// Legacy 32-bit headers.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeVlevelHeader32(const int field_num,
                               TaFile &outfile) const

{

  // Move to the appropriate offset

  int64_t offset = _mhdr.vlevel_hdr_offset + field_num * sizeof(vlevel_header_32_t);
  
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeVlevelHeader32\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to vlevel header offset: %lld\n", 
            (long long) offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // make local copy
  
  vlevel_header_64_t vhdr64 = _fields[field_num]->_vhdr;
  vlevel_header_32_t vhdr32;
  _copyVlevelHeader64to32(vhdr64, vhdr32);
  
  // convert to BE
  
  Mdvx::vlevel_header_to_BE_32(vhdr32);

  // Write the header to the output file.
  
  if (outfile.fwrite(&vhdr32, sizeof(vhdr32), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeVlevelHeader32\n";
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
// Write a vlevel header to an open file.
// Legacy 32-bit headers.
// This is not actually used.
// For 64-bit, we force a NetCDF write instead.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeVlevelHeader64(const int field_num,
                               TaFile &outfile) const

{

  // Move to the appropriate offset

  int64_t offset = _mhdr.vlevel_hdr_offset + field_num * sizeof(vlevel_header_64_t);
  
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeVlevelHeader64\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to vlevel header offset: %lld\n", 
            (long long) offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // make local copy
  
  vlevel_header_64_t vhdr = _fields[field_num]->_vhdr;
  
  // Convert local copy to BE
  
  Mdvx::vlevel_header_to_BE(vhdr);
    
  // Write the header to the output file.
  
  if (outfile.fwrite(&vhdr, sizeof(vhdr), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeVlevelHeader64\n";
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
// legacr 32-bit headers
// Returns 0 on success, -1 on failure.
 
int Mdvx::_writeChunkHeader32(const int chunk_num,
                              TaFile &outfile) const

{

  // Move to the appropriate offset
  
  int64_t offset = _mhdr.chunk_hdr_offset + chunk_num * sizeof(chunk_header_32_t);
  
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeChunkHeader32\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to chunk header offset: %lld\n",
            (long long) offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  // make local copy
  
  chunk_header_64_t chdr64 = _chunks[chunk_num]->_chdr;
  chunk_header_32_t chdr32;
  _copyChunkHeader64to32(chdr64, chdr32);
  
  // convert to BE
  
  Mdvx::chunk_header_to_BE_32(chdr32);
  
  // Write the header to the output file.
  
  if (outfile.fwrite(&chdr32, sizeof(chdr32), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeChunkHeader32\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot write chunk header for chunk: %d\n", chunk_num);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////
// Write a chunk header to an open file.
// Legacy 32-bit headers.
// This is not actually used.
// For 64-bit, we force a NetCDF write instead.
// Returns 0 on success, -1 on failure.

int Mdvx::_writeChunkHeader64(const int chunk_num,
                              TaFile &outfile) const

{

  // Move to the appropriate offset
  
  int64_t offset = _mhdr.chunk_hdr_offset + chunk_num * sizeof(chunk_header_64_t);
  
  if (outfile.fseek(offset, SEEK_SET)) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeChunkHeader64\n";
    char tmpstr[128];
    sprintf(tmpstr, "Cannot seek to chunk header offset: %lld\n",
            (long long) offset);
    _errStr += tmpstr;
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // make local copy
  
  chunk_header_64_t chdr = _chunks[chunk_num]->_chdr;

  // convert to BE
  
  Mdvx::chunk_header_to_BE(chdr);
    
  // Write the header to the output file.
  
  if (outfile.fwrite(&chdr, sizeof(chdr), 1) != 1) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeChunkHeader64\n";
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

  // default is NETCDF unless XML is specified
  
  if (_writeFormat != FORMAT_XML) {
    _writeFormat = FORMAT_NCF;
  }
  
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
    } else if (!strcasecmp(extendedPathsStr, "FALSE")) {
      _useExtendedPaths = false;
    }
  }
  
  char *addYearSubdir = getenv("MDV_WRITE_ADD_YEAR_SUBDIR");
  if (addYearSubdir != NULL) {
    if (!strcasecmp(addYearSubdir, "TRUE")) {
      _writeAddYearSubdir = true;
    } else if (!strcasecmp(addYearSubdir, "FALSE")) {
      _writeAddYearSubdir = false;
    }
  }
  
  char *writeCompStr = getenv("MDV_NCF_COMPRESSION_LEVEL");
  if (writeCompStr != NULL) {
    int level = 4;
    if (sscanf(writeCompStr, "%d", &level) != 1) {
      level = 4;
    }
    _ncfCompress = true;
    _ncfCompressionLevel = level;
  }
  
  char *ncfCompressStr = getenv("MDV_NCF_COMPRESS");
  if (ncfCompressStr != NULL) {
    if (!strcasecmp(ncfCompressStr, "TRUE")) {
      _ncfCompress = true;
    } else if (!strcasecmp(ncfCompressStr, "FALSE")) {
      _ncfCompress = false;
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
    if (_writeFormat == FORMAT_NCF) {
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
      if (forecastDelta < 0)  {
        sprintf(outputBase, "%.4d%.2d%.2d%sg_%.2d%.2d%.2d%sf_%.7d",
                genTime.year, genTime.month, genTime.day,
                PATH_DELIM, genTime.hour, genTime.min, genTime.sec,
                PATH_DELIM, forecastDelta);
      } else {
        sprintf(outputBase, "%.4d%.2d%.2d%sg_%.2d%.2d%.2d%sf_%.8d",
                genTime.year, genTime.month, genTime.day,
                PATH_DELIM, genTime.hour, genTime.min, genTime.sec,
                PATH_DELIM, forecastDelta);
      }
    } else {
      if (forecastDelta < 0) {
        sprintf(outputBase,
                "%.4d%.2d%.2d%s"
                "g_%.2d%.2d%.2d%s"
                "%.4d%.2d%.2d_g_%.2d%.2d%.2d_f_%.7d",
                genTime.year, genTime.month, genTime.day, PATH_DELIM,
                genTime.hour, genTime.min, genTime.sec, PATH_DELIM,
                genTime.year, genTime.month, genTime.day,
                genTime.hour, genTime.min, genTime.sec,
                forecastDelta);
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
    } // if (!_useExtendedPaths)

  } else { // if (writeAsForecast)

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

  } // if (writeAsForecast)

  outputName.clear();
  if (_writeAddYearSubdir) {
    outputName += yearSubdir;
    outputName += PATH_DELIM;
  }
  outputName += outputBase;
  outputName += ".mdv";

  if (_writeFormat == FORMAT_NCF) {
    outputName += ".cf.nc";
  } else if (_writeFormat == FORMAT_XML) {
    outputName += ".xml";
  }

  RapDataDir.fillPath(outputDir, outputPath);
  outputPath += PATH_DELIM;
  outputPath += outputName;

}

//////////////////////////////////////

void Mdvx::_doWriteLdataInfo(const string &outputDir,
                             const string &outputPath,
                             const string &dataType)
{
  
  DsLdataInfo ldata(outputDir, _debug);
  ldata.setPathAndTime(outputDir, outputPath);
  time_t latestTime;
  if (_getWriteAsForecast()) {
    latestTime = getGenTime();
    int leadtime = _mhdr.forecast_delta;
    ldata.setLeadTime(leadtime);
    ldata.setIsFcast(true);
  } else {
    latestTime = getValidTime();
  }
  ldata.setDataType(dataType);
  ldata.write(latestTime);

}

/////////////////////////////////////////////////////////
// write as forecast?

bool Mdvx::_getWriteAsForecast()
{
  
  if (_ifForecastWriteAsForecast) {
    if (_writeFormat == FORMAT_NCF) {
      if (_ncfIsForecast) {
        return true;
      }
    } else {
      if (_mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
          _mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
        return true;
      }
    }
  }

  return _writeAsForecast;

}
  
////////////////////////////////////////////////
// write generic buffer to file

int Mdvx::_writeBufferToFile(const string &pathStr,
                             size_t len,
                             const void *data) const

{

  // write to tmp file
  
  Path path(pathStr);
  path.makeDirRecurse();
  string tmpPathStr = path.computeTmpPath();
  
  TaFile out;
  out.setRemoveOnDestruct();

  if (out.fopen(tmpPathStr.c_str(), "wb") == NULL) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeBufferToFile\n";
    _errStr += "  Cannot open file for writing: ";
    _errStr += tmpPathStr;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  if (out.fwrite(data, 1, len) != len) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeBufferToFile\n";
    _errStr += "  Cannot write to path: ";
    _errStr += tmpPathStr;
    _errStr += "\n    ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }

  // close the  file
  
  out.fclose();
  
  // rename the tmp  file 
  
  if (rename(tmpPathStr.c_str(), pathStr.c_str())) {
    int errNum = errno;
    _errStr += "ERROR - Mdvx::_writeBufferToFile\n";
    _errStr += "  Cannot rename tmp file: ";
    _errStr += tmpPathStr;
    _errStr += " to: ";
    _errStr += pathStr;
    _errStr += "\n  ";
    _errStr += strerror(errNum);
    _errStr += "\n";
    return -1;
  }
  
  out.clearRemoveOnDestruct();

  return 0;

}

