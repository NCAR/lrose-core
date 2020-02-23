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
// WinsRadar.cc
//
// WinsRadar object - represents radar data in Wins format
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2000
//
///////////////////////////////////////////////////////////////


#include <rapformats/WinsRadar.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/Path.hh>
#include <toolsa/mem.h>
#include <dataport/swap.h>
#include <cerrno>
#include <math.h>
using namespace std;

// Constructor

WinsRadar::WinsRadar()

{
  
  _data = NULL;
  _debug = false;
  MEM_zero(_header);
  _headerSwapped = false;

}

// destructor

WinsRadar::~WinsRadar()

{

  // free up

  if (_data) {
    delete _data;
  }
  
}

//////////////////////////////////////////////////
// Read in a file

int WinsRadar::readFile (const string &file_path)
  
{

  if (_debug) {
    cerr << "WinsRadar::readFile" << endl;
    cerr << "  Reading in file: " << file_path << endl;
  }

  TaFile in;
  if (in.fopenUncompress(file_path.c_str(), "r") == NULL) {
    int errNum = errno;
    cerr << "ERROR - WinsRadar::readFile" << endl;
    cerr << "  Cannot open input file." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  " << file_path << ": " << strerror(errNum) << endl;
    return -1;
  }
  
  // read in header
  
  if (_debug) {
    cerr << "  Reading header" << endl;
    cerr << "    sizeof header: " << sizeof(_header) << endl;
  }
  
  if (in.fread(&_header, sizeof(_header), 1) != 1) {
    cerr << "ERROR - WinsRadar::readFile" << endl;
    cerr << "  Cannot read wins radar header." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  File: " << file_path << endl;
    return -1;
  }

  // interpret the header

  if (_interpretHeader()) {
    cerr << "ERROR - WinsRadar::readFile" << endl;
    cerr << "  Bad header" << endl;
    cerr << "  File: " << file_path << endl;
    printHeader(cerr);
    return -1;
  }

  // allocate the data array

  size_t nbytes = _header.nx * _header.ny;
  TaArray<ui08> tmpArray;
  ui08 *tmpData = tmpArray.alloc(nbytes);
  _data = new ui08[nbytes];

  // read in data array
  
  if (_debug) {
    cerr << "  Reading data array" << endl;
    cerr << "    size of data array: " << nbytes << endl;
  }
  
  if (in.fread(tmpData, 1, nbytes) != nbytes) {
    cerr << "ERROR - WinsRadar::readFile" << endl;
    cerr << "  Cannot read wins radar data array." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  File: " << file_path << endl;
    return -1;
  }
  
  in.fclose();

  // invert the data lines to start in the south

  int source = 0;
  int dest = nbytes - _header.nx;

  for (int iy = 0; iy < _header.ny;
       iy++, source += _header.nx, dest -= _header.nx) {
    memcpy(_data + dest, tmpData + source, _header.nx);
  } // iy

  return 0;

}

//////////////////////////////////////////////////
// write out to a file

int WinsRadar::writeFile (const string &file_path)
  
{

  if (_debug) {
    cerr << "WinsRadar::writeFile" << endl;
    cerr << "  Writing file: " << file_path << endl;
  }

  // make the directory

  Path outPath(file_path);
  if (outPath.makeDirRecurse()) {
    int errNum = errno;
    cerr << "ERROR - WinsRadar::writeFile" << endl;
    cerr << "  Cannot make directory" << endl;
    cerr << "  " << outPath.getDirectory() << ": "
	 << strerror(errNum) << endl;
    return -1;
  }
  
  TaFile out;
  if (out.fopen(file_path.c_str(), "w") == NULL) {
    int errNum = errno;
    cerr << "ERROR - WinsRadar::writeFile" << endl;
    cerr << "  Cannot open output file." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  " << file_path << ": " << strerror(errNum) << endl;
    return -1;
  }

  // copy header for writing, swap as required

  header_t hdr = _header;
  if (_headerSwapped) {
    _swapHeader(hdr);
  }
  
  // write out header

  if (_debug) {
    cerr << "  Writing header" << endl;
    cerr << "    sizeof header: " << sizeof(hdr) << endl;
  }
  
  if (out.fwrite(&hdr, sizeof(hdr), 1) != 1) {
    cerr << "ERROR - WinsRadar::writeFile" << endl;
    cerr << "  Cannot write wins radar header." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  File: " << file_path << endl;
    return -1;
  }

  // invert the data lines to start in the north

  size_t nbytes = _header.nx * _header.ny;
  TaArray<ui08> tmpArray;
  ui08 *tmpData = tmpArray.alloc(nbytes);
  int source = 0;
  int dest = nbytes - _header.nx;

  for (int iy = 0; iy < _header.ny;
       iy++, source += _header.nx, dest -= _header.nx) {
    memcpy(tmpData + dest, _data + source, _header.nx);
  } // iy

  // write out the data array
  
  if (_debug) {
    cerr << "  Writing data array" << endl;
    cerr << "    size of data array: " << nbytes << endl;
  }
  
  if (out.fwrite(tmpData, 1, nbytes) != nbytes) {
    cerr << "ERROR - WinsRadar::writeFile" << endl;
    cerr << "  Cannot write wins radar data array." << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  File: " << file_path << endl;
    return -1;
  }

  // close file

  out.fclose();

  return 0;

}

//////////////////////////////////////////////////
// Print the header

void WinsRadar::printHeader(ostream &out) const
  
{

  out << endl;
  out << "  WINS RADAR HEADER" << endl;
  out << "  =================" << endl;

  out << "    volume_info: " << _volumeInfo << endl;
  out << "    lat: " << _header.lat << endl;
  out << "    lon: " << _header.lon << endl;
  out << "    ht: " << _header.ht << endl;
  out << "    year: " << _header.year << endl;
  out << "    month: " << _header.month << endl;
  out << "    day: " << _header.day << endl;
  out << "    hour: " << _header.hour << endl;
  out << "    min_start: " << _header.min_start << endl;
  out << "    min_end: " << _header.min_end << endl;
  out << "    field_name: " << _fieldName << endl;
  out << "    field_units: " << _fieldUnits << endl;
  out << "    nx: " << _header.nx << endl;
  out << "    ny: " << _header.ny << endl;
  out << "    dx: " << _header.dx << endl;
  out << "    dy: " << _header.dy << endl;
  out << "    scale: " << _header.scale << endl;
  out << "    data_for_byte_0: " << _header.data_for_byte_0 << endl;
  out << "    data_for_byte_254: " << _header.data_for_byte_254 << endl;
  out << "    log_flag: " << _header.log_flag << endl;
  out << "    color_table_name: " << _colorTableName << endl;
  out << "    min_lat: " << _header.min_lat << endl;
  out << "    min_lon: " << _header.min_lon << endl;
  out << "    max_lat: " << _header.max_lat << endl;
  out << "    max_lon: " << _header.max_lon << endl;

  out << "    _vmin: " << _vmin << endl;
  out << "    _vmax: " << _vmax << endl;
  out << "    _bias: " << _bias << endl;
  out << "    _scale: " << _scale << endl;

}

//////////////////////////////////////////////////
// Print the header

void WinsRadar::printData(ostream &out) const
  
{

  out << endl;
  out << "  WINS RADAR DATA" << endl;
  out << "  ===============" << endl;

  ui08 *data = _data;

  for (int iy = 0; iy < _header.ny; iy++) {

    out << "  Row iy: " << iy << endl;

    for (int ix = 0; ix < _header.nx; ix++, data++) {

      if (*data != 255) {
	double value;
	if (_header.log_flag) {
	  value = exp(_bias + *data * _scale);
	} else {
	  value = _bias + *data * _scale;
	}
	out << value << " ";
      }
      
    } // ix

    out << endl;

  } // iy

}

//////////////////////////////////////////////////
// Interpret the header

int WinsRadar::_interpretHeader()

{
  
  // decide if we need to swap the header
  
  _headerSwapped = false;
  if (_header.month < 1 || _header.month > 12) {
    _headerSwapped = true;
  }

  if (_headerSwapped) {
    _swapHeader(_header);
  }

  // check the date is OK

  if (_header.year < 1900 || _header.year > 9999) {
    return -1;
  }

  if (_header.month < 1 || _header.month > 12) {
    return -1;
  }

  if (_header.day < 1 || _header.day > 31) {
    return -1;
  }

  if (_header.hour < 0 || _header.hour > 23) {
    return -1;
  }

  if (_header.min_start < 0 || _header.min_start > 59) {
    return -1;
  }

  _terminateString(_header.volume_info, _volumeInfo, 20);
  _terminateString(_header.field_name, _fieldName, 4);
  _terminateString(_header.field_units, _fieldUnits, 8);
  _terminateString(_header.color_table_name, _colorTableName, 8);

  _vmin = (double) _header.data_for_byte_0 / (double) _header.scale;
  _vmax = (double) _header.data_for_byte_254 / (double) _header.scale;
  
  if (_header.log_flag) {
    _bias = log(_vmax);
    _scale = (log(_vmax) - log(_vmin)) / 254.0;
  } else {
    _bias = _vmin;
    _scale = (_vmax - _vmin) / 254.0;
  }

  return 0;

}

//////////////////////////////////////////////////
// Swap the header

void WinsRadar::_swapHeader(header_t &hdr)

{

  if (_debug) {
    cerr << "  Note: -swapping header" << endl;
  }
  
  hdr.lat = SWAP_si32(hdr.lat);
  hdr.lon = SWAP_si32(hdr.lon);
  hdr.ht = SWAP_si16(hdr.ht);
  hdr.year = SWAP_si16(hdr.year);
  hdr.month = SWAP_si16(hdr.month);
  hdr.day = SWAP_si16(hdr.day);
  hdr.hour = SWAP_si16(hdr.hour);
  hdr.min_start = SWAP_si16(hdr.min_start);
  hdr.min_end = SWAP_si16(hdr.min_end);
  hdr.nx = SWAP_si16(hdr.nx);
  hdr.ny = SWAP_si16(hdr.ny);
  hdr.dx = SWAP_si16(hdr.dx);
  hdr.dy = SWAP_si16(hdr.dy);
  hdr.scale = SWAP_si16(hdr.scale);
  hdr.data_for_byte_0 = SWAP_si16(hdr.data_for_byte_0);
  hdr.data_for_byte_254 = SWAP_si16(hdr.data_for_byte_254);
  hdr.log_flag = SWAP_si16(hdr.log_flag);
  hdr.min_lat = SWAP_si32(hdr.min_lat);
  hdr.min_lon = SWAP_si32(hdr.min_lon);
  hdr.max_lat = SWAP_si32(hdr.max_lat);
  hdr.max_lon = SWAP_si32(hdr.max_lon);

}

//////////////////////////////////////////////////
// Terminate strings
//
// For files generated in FORTRAN, strings are not necessarily
// null terminated.

void WinsRadar::_terminateString(const char *input_text,
				 char *stored_text,
				 int max_len)

{

  memcpy(stored_text, input_text, max_len);
  
  for (int i = max_len - 1; i >= 0; i--) {
    if (stored_text[i] == ' ') {
      stored_text[i] = '\0';
    } else {
      break;
    }
  }
  stored_text[max_len] = '\0';
  
}




