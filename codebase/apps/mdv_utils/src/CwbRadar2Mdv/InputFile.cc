/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// InputFile.cc
//
// InputFile object - represents radar data in CWB format
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2003
//
///////////////////////////////////////////////////////////////


#include "InputFile.hh"
#include <toolsa/TaFile.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/mem.h>
#include <dataport/swap.h>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <cstdlib>
using namespace std;

// Constructor

InputFile::InputFile(const string &prog_name,
		     const Params &params) :
  _progName(prog_name),
  _params(params)

{

  _fieldData = NULL;
  _swapped = false;
  MEM_zero(_header);

}

// destructor

InputFile::~InputFile()

{

  // free up

  if (_fieldData) {
    delete[] _fieldData;
    _fieldData = NULL;
  }
  
}

//////////////////////////////////////////////////
// Read in a file

int InputFile::readFile (const string &file_path)
  
{
  
  if (_params.debug) {
    cerr << "InputFile::readFile" << endl;
    cerr << "  Reading in file: " << file_path << endl;
  }

  TaFile in;
  if (in.fopenUncompress(file_path.c_str(), "rb") == NULL) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot open input file." << endl;
    return -1;
  }
  
  // read in header
  
  if (in.fread(&_header, sizeof(_header), 1) != 1) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read header" << endl;
    return -1;
  }
  
  // decide if we need to swap
  
  _swapped = false;
  if (_header.month < 1 || _header.month > 12) {
    _swapped = true;
  }
  if (_swapped) {
    _swapHeader();
  }

  // sanity check

  if (_header.year < 1900 || _header.year > 3000 ||
      _header.month < 0 || _header.month > 12 ||
      _header.day < 0 || _header.day > 32 ||
      _header.hour < 0 || _header.hour > 24 ||
      _header.min  < 0 || _header.min > 60 ||
      _header.sec < 0 || _header.sec > 60) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Bad header, cannot swap" << endl;
    return -1;
  }
  
  
  // set members
  
  _time.set(_header.year, _header.month, _header.day,
	    _header.hour, _header.min, _header.sec);
  
  _nx = _header.nx;
  _ny = _header.ny;
  _nz = _header.nz;
  
  _trueLat1 = (double) _header.trueLat1 / (double) _header.mapScale;
  _trueLat2 = (double) _header.trueLat2 / (double) _header.mapScale;
  _trueLon = (double) _header.trueLon / (double) _header.mapScale;
  
  _nwLat = (double) _header.nwLat / (double) _header.mapScale;
  _nwLon = (double) _header.nwLon / (double) _header.mapScale;

  _dx = (double) _header.dx / (double) _header.dxyScale;
  _dy = (double) _header.dx / (double) _header.dxyScale;
  
  _projType = _convert2String(_header.proj, 4);

  // read in z levels

  TaArray<si32> zlevels_;
  si32 *zlevels = zlevels_.alloc(_nz);
  if (in.fread(zlevels, sizeof(si32), _nz) != (size_t) _nz) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read zlevels." << endl;
    return -1;
  }
  if (_swapped) {
    SWAP_array_32((ui32 *) zlevels, _nz * sizeof(si32));
  }

  si32 zScale;
  if (in.fread(&zScale, sizeof(si32), 1) != 1) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read zScale." << endl;
    return -1;
  }
  if (_swapped) {
    zScale = SWAP_si32(zScale);
  }

  for (int i = 0; i < _nz; i++) {
    double zz = (double) zlevels[i] / (_header.xyScale);
    _zLevels.push_back(zz);
  }

  // read in iBBMode

  si32 i_bb_mode;
  if (in.fread(&i_bb_mode, sizeof(si32), 1) != 1) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read i_bb_mode." << endl;
    return -1;
  }
  if (_swapped) {
    i_bb_mode = SWAP_si32(i_bb_mode);
  }
  _iBBMode = i_bb_mode;

  // read past spares

  si32 spares[9];
  if (in.fread(spares, sizeof(si32), 9) != 9) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read spares." << endl;
    return -1;
  }

  // read name and units

  char name[24], units[8];
  memset(name, 0, 24);
  memset(units, 0, 8);
  if (in.fread(name, 1, 20) != 20) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read name." << endl;
    return -1;
  }
  if (in.fread(units, 1, 6) != 6) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read units." << endl;
    return -1;
  }
  _fieldName = _convert2String(name, 24);
  _fieldUnits = _convert2String(units, 6);

  // read in varScale & missing val

  si32 varScale;
  if (in.fread(&varScale, sizeof(si32), 1) != 1) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read varScale." << endl;
    return -1;
  }
  if (_swapped) {
    varScale = SWAP_si32(varScale);
  }

  si32 missing;
  if (in.fread(&missing, sizeof(si32), 1) != 1) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read missing." << endl;
    return -1;
  }
  if (_swapped) {
    missing = SWAP_si32(missing);
  }
  _missing = missing;

  // read in radar names
  
  si32 nRadars;
  if (in.fread(&nRadars, sizeof(si32), 1) != 1) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read nRadars." << endl;
    return -1;
  }
  if (_swapped) {
    nRadars = SWAP_si32(nRadars);
  }

  for (int i = 0; i < nRadars; i++) {
    char radarName[8];
    memset(radarName, 0, 8);
    if (in.fread(radarName, 1, 4) != 4) {
      cerr << "ERROR - InputFile::readFile" << endl;
      cerr << "  Cannot read radar name." << endl;
      return -1;
    }
    _radarNames.push_back(radarName);
  }

  // read in data
  
  size_t npoints = _nx * _ny * _nz;
  si16 *fieldData = new si16[npoints];
  if (in.fread(fieldData, sizeof(si16), npoints) != npoints) {
    cerr << "ERROR - InputFile::readFile" << endl;
    cerr << "  Cannot read data." << endl;
    return -1;
  }
  if (_swapped) {
    SWAP_array_16((ui16 *) fieldData, sizeof(fieldData));
  }
  
  if (_fieldData) {
    delete[] _fieldData;
  }
  _fieldData = new fl32[npoints];
  for (size_t i = 0; i < npoints; i++) {
    _fieldData[i] = (double) fieldData[i] / (double) varScale;
    if (_fieldData[i] < _params.reflectivity_min_value) {
      _fieldData[i] = _missing;
    }
  }
  cerr << endl;
  delete[] fieldData;
  
  // close file
  
  in.fclose();
  
  // invert the data lines to start in the south

#ifdef JUNK  
  int source = 0;
  int dest = npoints - _nx;
  fl32 *tmpData = new fl32[npoints];
  memcpy(tmpData, _fieldData, npoints * sizeof(fl32));
  for (int iy = 0; iy < _ny; iy++, source += _nx, dest -= _nx) {
    memcpy(_fieldData + dest, tmpData + source, _nx * sizeof(fl32));
  } // iy
  delete[] tmpData;
#endif

  return 0;

}

//////////////////////////////////////////////////
// Print the info

void InputFile::printInfo(ostream &out) const
  
{

  out << endl;
  out << "  CWB RADAR FILE INFO" << endl;
  out << "  ===================" << endl;

  out << "    time: " << _time.getStr() << endl;
  out << "    proj: " << _projType << endl;
  out << "    nx, ny, nz: " << _nx << ", " << _ny << ", " << _nz << endl;
  out << "    trueLat1: " << _trueLat1 << endl;
  out << "    trueLat2: " << _trueLat2 << endl;
  out << "    trueLon: " << _trueLon << endl;
  out << "    nwLon: " << _nwLon << endl;
  out << "    nwLat: " << _nwLat << endl;
  out << "    dx: " << _dx << endl;
  out << "    dy: " << _dy << endl;
  out << "    zLevels: " << endl;
  for (size_t ii = 0; ii < _zLevels.size(); ii++) {
    out << "      " << _zLevels[ii] << endl;
  }
  out << "    iBBMode: " << _iBBMode << endl;
  out << "    name: " << _fieldName << endl;
  out << "    units: " << _fieldUnits << endl;
  out << "    missing: " << _missing << endl;
  out << "    nRadars: " << _radarNames.size() << endl;
  out << "    radar Names: " << endl;
  for (size_t ii = 0; ii < _radarNames.size(); ii++) {
    out << "      " << _radarNames[ii] << endl;
  }

}

//////////////////////////////////////////////////
// Print the data

void InputFile::printData(ostream &out) const
  
{

  out << endl;
  out << "  CWB RADAR DATA" << endl;
  out << "  ==============" << endl;

  fl32 *data = _fieldData;
  
  for (int iy = 0; iy < _ny; iy++) {

    out << "  Row iy: " << iy << endl;

    for (int ix = 0; ix < _nx; ix++, data++) {
      out << *data << " ";
    }
    
    out << endl;

  } // iy

}

//////////////////////////////////////////////////
// Print the header

void InputFile::printHeader(ostream &out) const
  
{

  out << endl;
  out << "  CWB RADAR HEADER" << endl;
  out << "  ================" << endl;

  out << "    year: " << _header.year << endl;
  out << "    month: " << _header.month << endl;
  out << "    day: " << _header.day << endl;
  out << "    hour: " << _header.hour << endl;
  out << "    min: " << _header.min << endl;
  out << "    sec: " << _header.sec << endl;
  out << "    nx: " << _header.nx << endl;
  out << "    ny: " << _header.ny << endl;
  out << "    nz: " << _header.nz << endl;
  out << "    mapScale: " << _header.mapScale << endl;
  out << "    trueLat1: " << _header.trueLat1 << endl;
  out << "    trueLat2: " << _header.trueLat2 << endl;
  out << "    trueLon: " << _header.trueLon << endl;
  out << "    nwLon: " << _header.nwLon << endl;
  out << "    nwLat: " << _header.nwLat << endl;
  out << "    xyScale: " << _header.xyScale << endl;
  out << "    dx: " << _header.dx << endl;
  out << "    dy: " << _header.dy << endl;
  out << "    dxyScale: " << _header.dxyScale << endl;

}

//////////////////////////////////////////////////
// Swap the header

void InputFile::_swapHeader()

{

  if (_params.debug) {
    cerr << "  Note: swapping header" << endl;
  }
  
  _header.year = SWAP_si32(_header.year);
  _header.month = SWAP_si32(_header.month);
  _header.day = SWAP_si32(_header.day);
  _header.hour = SWAP_si32(_header.hour);
  _header.min = SWAP_si32(_header.min);
  _header.sec = SWAP_si32(_header.sec);
  
  _header.nx = SWAP_si32(_header.nx);
  _header.ny = SWAP_si32(_header.ny);
  _header.nz = SWAP_si32(_header.nz);

  _header.mapScale = SWAP_si32(_header.mapScale);
  _header.trueLat1 = SWAP_si32(_header.trueLat1);
  _header.trueLat2 = SWAP_si32(_header.trueLat2);
  _header.trueLon = SWAP_si32(_header.trueLon);
  _header.nwLon = SWAP_si32(_header.nwLon);
  _header.nwLat = SWAP_si32(_header.nwLat);
  _header.xyScale = SWAP_si32(_header.xyScale);
  _header.dx = SWAP_si32(_header.dx);
  _header.dy = SWAP_si32(_header.dy);
  _header.dxyScale = SWAP_si32(_header.dxyScale);

}

//////////////////////////////////////////////////
// Terminate strings
//
// For files generated in FORTRAN, strings are not necessarily
// null terminated.

string InputFile::_convert2String(const char *input_text, int max_len)

{
  
  TaArray<char> stored_text_;
  char *stored_text = stored_text_.alloc(max_len + 1);
  memcpy(stored_text, input_text, max_len);
  
  for (int i = max_len - 1; i >= 0; i--) {
    if (stored_text[i] == ' ') {
      stored_text[i] = '\0';
    } else {
      break;
    }
  }
  stored_text[max_len] = '\0';

  return stored_text;
  
}

