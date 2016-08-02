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
/////////////////////////////////////////////////////////////
// GenPt.cc
//
// C++ wrapper for generic point data.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2000
//////////////////////////////////////////////////////////////


#include <rapformats/GenPt.hh>
#include <dataport/bigend.h>
#include <toolsa/udatetime.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
using namespace std;

const double GenPt::missingVal = -9999.0;

// constructor

GenPt::GenPt()

{
  clear();
}

// destructor

GenPt::~GenPt()

{
}

////////////////////
// clear

void GenPt::clear()
{
  _time = 0;
  _lat = 0.0;
  _lon = 0.0;
  _nLevels = 1;
  _id = 0;
  _name = "";
  _text = "";
  _errStr = "";
  _type = DATA_SURFACE;
  clearVals();
  clearFieldInfo();
}

//////////////////////////////////////////////////////////////////
// Check we have a valid object.
// Use this after the set methods to check the object.
// On failure, use getErrStr() to see error

bool GenPt::check() const
{
  
  _errStr = "ERROR - GenPt::check()\n";
  
  if (_fieldInfo.size() < 1) {
    _errStr += "  Must have at least 1 field.\n";
    return false;
  }
  
  if (_nLevels < 1) {
    _errStr += "  Must have at least 1 level.\n";
    return false;
  }

  int nvals = _fieldInfo.size() * _nLevels;
  if (nvals != (int) _vals.size()) {
    TaStr::AddInt(_errStr, "  Wrong number of values: ", (int)_vals.size());
    TaStr::AddInt(_errStr, "  nFields: ", _fieldInfo.size());
    TaStr::AddInt(_errStr, "  nLevels: ", _nLevels);
    TaStr::AddInt(_errStr, "  Expected nvals: ", nvals);
    return false;
  }

  return true;

}

//////////////////////////////////////////////////////////////////
// clear and add fields

void GenPt::clearFieldInfo()

{
  _fieldInfo.erase(_fieldInfo.begin(), _fieldInfo.end());
}

void GenPt::addFieldInfo(const string &name,
			 const string &units)
  
{
  FieldInfo fld;
  fld.name = name;
  fld.units = units;
  _fieldInfo.push_back(fld);
}

// set the field info from a composite string
//
// returns 0 on success, -1 on failure

int GenPt::setFieldInfo(const string &infoStr)

{

  clearFieldInfo();
  string field;
  size_t start = 0;
  size_t comma = 0;
  while (comma != string::npos) {
    comma = infoStr.find_first_of(',', start);
    // cerr << "comma: " << comma << endl;
    // cerr << "start: " << start << endl;
    if (comma == string::npos) {
      field.assign(infoStr, start, string::npos);
    } else {
      field.assign(infoStr, start, comma - start);
    }
    start = comma + 1;
    // cerr << "Field: " << field << endl;
    size_t colon;
    colon = field.find_first_of(':', 0);
    if (colon == string::npos) {
      cerr << "ERROR - field: " << field << endl;
      return -1;
    }
    FieldInfo info;
    info.name.assign(field, 0, colon);
    info.units.assign(field, colon + 1, string::npos);
    // cerr << "  info.name: " << info.name << endl;
    // cerr << "  info.units: " << info.units << endl;
    _fieldInfo.push_back(info);
  }

  return 0;

}
  
///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the object values.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int GenPt::disassemble(const void *buf, int len)
  
{
  
  clear();
  _errStr = "ERROR - GenPt::disassemble()\n";
  
  // check minimum len for header
  
  if (len < (int) sizeof(header_t)) {
    TaStr::AddInt(_errStr, "  Buffer too short for header, len: ", len);
    TaStr::AddInt(_errStr, "  Minimum valid len: ",
		  sizeof(header_t));
    return -1;
  }
  
  // local copy of buffer

  _memBuf.free();
  _memBuf.add(buf, len);
  
  // get header
  
  header_t *hdr = (header_t *) _memBuf.getPtr();
  _BE_to_header(*hdr);
  _time = hdr->time;
  _lat = hdr->lat;
  _lon = hdr->lon;
  _nLevels = hdr->n_levels;
  _id = hdr->id;
  _type = (data_type_t)hdr->type;
  int nVals = hdr->n_fields * _nLevels;

  // check expected len
  
  if (len != hdr->buf_len) {
    TaStr::AddInt(_errStr, "  Buffer wrong length, len: ", len);
    TaStr::AddInt(_errStr, "  Expected len: ", hdr->buf_len);
    TaStr::AddInt(_errStr, "  nFields: ", hdr->n_fields);
    TaStr::AddInt(_errStr, "  nLevels: ", _nLevels);
    return -1;
  }
  
  // data values

  fl32 *vals = (fl32 *) ((char *) _memBuf.getPtr() + sizeof(header_t));
  BE_to_array_32(vals, nVals * sizeof(fl32));
  for (int i = 0; i < nVals; i++) {
    _vals.push_back(vals[i]);
  }

  // name string
  
  char *name = (char *) (vals + nVals);
  if (hdr->name_len > 0) {
    name[hdr->name_len - 1] = '\0';
    _name = name;
  }
  
  // field info
  
  char *field_info = name + hdr->name_len;
  field_info[hdr->field_info_len - 1] = '\0';
  string infoStr = field_info;

  if (setFieldInfo(infoStr)) {
    return -1;
  }

  if (hdr->n_fields != (int)_fieldInfo.size()) {
    _errStr += "  Inconsistent number of fields\n";
    TaStr::AddInt(_errStr, "    Number set in header: ", hdr->n_fields);
    TaStr::AddInt(_errStr, "    Number from field info in header: ",
		  _fieldInfo.size());
    return -1;
  }
  
  // text string

  char *text = field_info + hdr->field_info_len;
  text[hdr->text_len - 1] = '\0';
  _text = text;
  
  return 0;

}

//////////////////////////////
// get field info string

string GenPt::getFieldInfoStr() const

{
  string fieldInfoStr;
  _combineFieldInfo(fieldInfoStr);
  return (fieldInfoStr);
}

///////////////////////////////////////
// Get a field number given the name
// Returns -1 on error.

int GenPt::getFieldNum(const string &name) const

{
  for (size_t i = 0; i < _fieldInfo.size(); i++) {
    if (_fieldInfo[i].name == name) {
      return (int) i;
    }
  }
  return -1;
}

////////////////////////////////////////////////////////
// get the field name and units, given the field number

string GenPt::getFieldName(int field_num)
{
  if (field_num > (int) _fieldInfo.size() - 1) {
    cerr << "ERROR - GenPt::getFieldName()" << endl;
    cerr << "  Field number: " << field_num <<  " out of range." << endl;
    cerr << "  Max field number: " << _fieldInfo.size() - 1 << endl;
    return "Invalid field number";
  }
  return _fieldInfo[field_num].name;
}

string GenPt::getFieldUnits(int field_num)
{
  if (field_num > (int) _fieldInfo.size() - 1) {
    cerr << "ERROR - GenPt::getFieldUnits()" << endl;
    cerr << "  Field number: " << field_num <<  " out of range." << endl;
    cerr << "  Max field number: " << _fieldInfo.size() - 1 << endl;
    return "Invalid field number";
  }
  return _fieldInfo[field_num].units;
}

////////////////////////////////////////////////
// get data values

// get a value, given the 1D index
  
double GenPt::get1DVal(int index) const
{
  if (!check()) {
    cerr << "ERROR - GenPt::get1DVal()" << endl;
    cerr << _errStr;
    return 0.0;
  }
  if (index > (int) _vals.size() - 1) {
    cerr << "ERROR - GenPt::get1DVal()" << endl;
    cerr << "  Array index: " << index <<  " out of range." << endl;
    cerr << "  Max index: " << _vals.size() - 1 << endl;
    return 0.0;
  }
  return _vals[index];
}
  
// get a value, given the 2D indices
  
double GenPt::get2DVal(int level_num, int field_num) const
{
  if (!check()) {
    cerr << "ERROR - GenPt::get2DVal()" << endl;
    cerr << _errStr;
    return 0.0;
  }
  int index = level_num * _fieldInfo.size() + field_num;
  if (index > (int) _vals.size() - 1) {
    cerr << "ERROR - GenPt::getVal()" << endl;
    cerr << "  Array index: " << index <<  " out of range." << endl;
    cerr << "  Max index: " << _vals.size() - 1 << endl;
    cerr << "  Requested level_num: " << level_num << endl;
    cerr << "  Max level number: " << _nLevels - 1 << endl;
    cerr << "  Requested field_num: " << field_num << endl;
    cerr << "  Max field number: " << _fieldInfo.size() - 1 << endl;
    return 0.0;
  }
  return _vals[index];
}
  
///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.
//
// returns 0 on success, -1 on failure
// Use getErrStr() on failure.

int GenPt::assemble()
  
{

  // check if we have a valid object

  if (!check()) {
    _errStr += "ERROR - GenPt::assemble()\n";
    return -1;
  }

  // combine the field info into a single string
  
  string fieldInfoStr;
  _combineFieldInfo(fieldInfoStr);

  // load the header

  header_t hdr;
  MEM_zero(hdr);
  hdr.time = _time;
  hdr.lat = _lat;
  hdr.lon = _lon;
  hdr.n_fields = _fieldInfo.size();
  hdr.n_levels = _nLevels;
  hdr.name_len = _name.size() + 1;
  hdr.field_info_len = fieldInfoStr.size() + 1;
  hdr.text_len = _text.size() + 1;
  hdr.buf_len = 
    sizeof(header_t) + (_vals.size() * sizeof(fl32)) +
    hdr.name_len + hdr.field_info_len + hdr.text_len;
  hdr.id = _id;
  hdr.type = _type;
  _BE_from_header(hdr);

  // assemble buffer

  _memBuf.free();
  _memBuf.add(&hdr, sizeof(header_t));

  for (size_t i = 0; i < _vals.size(); i++) {
    fl32 val = (fl32) _vals[i];
    BE_from_array_32(&val, sizeof(val));
    _memBuf.add(&val, sizeof(val));
  }
  
  _memBuf.add(_name.c_str(), _name.size() + 1);
  _memBuf.add(fieldInfoStr.c_str(), fieldInfoStr.size() + 1);
  _memBuf.add(_text.c_str(), _text.size() + 1);

  return 0;

}

////////////////////////////////////////////////////////
// prints

void GenPt::print(FILE *out) const

{

  fprintf(out, "  ==========================\n");
  fprintf(out, "  GenPt - generic point data\n");
  fprintf(out, "  ==========================\n");
  fprintf(out, "  time: %s\n", utimstr(_time));
  fprintf(out, "  lat: %g\n", _lat);
  fprintf(out, "  lon: %g\n", _lon);
  fprintf(out, "  nfields: %d\n", (int) _fieldInfo.size());
  fprintf(out, "  nlevels: %d\n", (int) _nLevels);
  fprintf(out, "  id: %d\n", _id);
  fprintf(out, "  type: %s\n", dataType2Str(_type));
  
  int index = 0;
  for (int j = 0; j < _nLevels; j++) {
    if (_nLevels > 1) {
      fprintf(out, "    Level %d, fields: ", j);
    } else {
      fprintf(out, "    Fields: ");
    }
    for (int i = 0; i < (int)_fieldInfo.size(); i++, index++) {
      fprintf(out, "%g", _vals[index]);
      if (i == (int)_fieldInfo.size() - 1) {
	fprintf(out, "\n");
      } else {
	fprintf(out, ", ");
      }
    } // i
  } // j

  fprintf(out, "  name: %s\n", _name.c_str());
  string fieldInfoStr;
  _combineFieldInfo(fieldInfoStr);
  fprintf(out, "  fieldInfo: %s\n", fieldInfoStr.c_str());
  fprintf(out, "  text: %s\n", _text.c_str());

}

void GenPt::print(ostream &out) const

{
  out << "  ==========================" << endl;
  out << "  GenPt - generic point data" << endl;
  out << "  ==========================" << endl;
  out << "  time: " << utimstr(_time) << endl;
  out << "  lat: " << _lat << endl;
  out << "  lon: " << _lon << endl;
  out << "  nfields: " << _fieldInfo.size() << endl;
  out << "  nlevels: " << _nLevels << endl;
  out << "  id: " << _id << endl;
  out << "  type: " << dataType2Str(_type) << endl;
  
  int index = 0;
  for (int j = 0; j < _nLevels; j++) {
    if (_nLevels > 1) {
      out << "    Level " << j << ", fields: ";
    } else {
      out << "    Fields: ";
    }
    for (int i = 0; i < (int)_fieldInfo.size(); i++, index++) {
      out << _vals[index];
      if (i == (int)_fieldInfo.size() - 1) {
	out << endl;
      } else {
	out << ", ";
      }
    } // i
  } // j
  
  out << "  name: " << _name << endl;
  string fieldInfoStr;
  _combineFieldInfo(fieldInfoStr);
  out << "  fieldInfo: " << fieldInfoStr << endl;
  out << "  text: " << _text << endl;
}

////////////////////////////////////////////////
// combine the field info into a single string

void GenPt::_combineFieldInfo(string &fieldInfoStr) const
{  
  for (size_t i = 0; i < _fieldInfo.size(); i++) {
    fieldInfoStr += _fieldInfo[i].name;
    fieldInfoStr += ":";
    fieldInfoStr += _fieldInfo[i].units;
    if (i != _fieldInfo.size() - 1) {
      fieldInfoStr += ",";
    }
  }
}

/////////////////////////////////////////////////////////
// byte swapping routines

void GenPt::_BE_from_header(header_t &hdr)

{
  BE_from_array_32(&hdr, sizeof(header_t));
}

void GenPt::_BE_to_header(header_t &hdr)
  
{
  BE_to_array_32(&hdr, sizeof(header_t));
}

/////////////////////////////////////////////////////////
// return string representation of data type

char *GenPt::dataType2Str(const data_type_t data_type) const {

  static char dataTypes[4][16] = {
    "Surface\0", "Soundings\0", "Mobile\0", "Unknown\0"};

  switch (data_type) {

    case DATA_SURFACE:
      return(dataTypes[0]);

    case DATA_SOUNDING:
      return(dataTypes[1]);

    case DATA_MOBILE:
      return(dataTypes[2]);

    default:
     return(dataTypes[3]);
  }

}
