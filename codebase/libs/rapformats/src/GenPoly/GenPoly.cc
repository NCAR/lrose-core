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
// GenPoly.cc
//
// C++ wrapper for generic polyline/polygon data.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// December 2003
//////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <rapformats/GenPoly.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>

using namespace std;

// Initialize constants

const double GenPoly::missingVal = -9999.0;

// constructor

GenPoly::GenPoly()
{
  clear();
}

// destructor

GenPoly::~GenPoly()
{
}

////////////////////
// clear

void GenPoly::clear()
{
  _time = 0;
  _expireTime = 0;
  _nLevels = 1;
  _id = 0;
  _closed = false;
  _name = "";
  _text = "";
  _errStr = "";
  clearVals();
  clearFieldInfo();
  _vertices.erase(_vertices.begin(), _vertices.end());
}

//////////////////////////////////////////////////////////////////
// Check we have a valid object.
// Use this after the set methods to check the object.
// On failure, use getErrStr() to see error

bool GenPoly::check() const
{
  
  _errStr = "ERROR - GenPoly::check()\n";
  
  if (_nLevels < 1)
  {
    _errStr += "  Must have at least 1 level.\n";
    return false;
  }

  if (_vertices.size() < 2)
  {
    _errStr += "  Must have at least 2 vertices.\n";
    return false;
  }
  
  int nvals = _fieldInfo.size() * _nLevels;
  if (nvals != (int)_vals.size())
  {
    TaStr::AddInt(_errStr, "  Wrong number of values: ", nvals);
    TaStr::AddInt(_errStr, "  nFields: ", _fieldInfo.size());
    TaStr::AddInt(_errStr, "  nLevels: ", _nLevels);
    TaStr::AddInt(_errStr, "  Expected nvals: ", _fieldInfo.size() * _nLevels);

    return false;
  }

  return true;

}

//////////////////////////////////////////////////////////////////
// clear and add fields

void GenPoly::clearFieldInfo()
{
  _fieldInfo.erase(_fieldInfo.begin(), _fieldInfo.end());
}

void GenPoly::addFieldInfo(const string &name,
			   const string &units)
{
  FieldInfo fld;
  fld.name = name;
  fld.units = units;
  _fieldInfo.push_back(fld);
}

// set the field info from a composite string
//
// returns true on success, false on failure

bool GenPoly::setFieldInfo(const string &infoStr)
{
  clearFieldInfo();

  string field;
  size_t start = 0;
  size_t comma = 0;

  while (comma != string::npos)
  {
    comma = infoStr.find_first_of(',', start);

    if (comma == string::npos)
      field.assign(infoStr, start, string::npos);
    else
      field.assign(infoStr, start, comma - start);

    start = comma + 1;

    size_t colon;
    colon = field.find_first_of(':', 0);

    if (colon == string::npos)
    {
      cerr << "ERROR - field: " << field << endl;
      return false;
    }

    FieldInfo info;
    info.name.assign(field, 0, colon);
    info.units.assign(field, colon + 1, string::npos);
    _fieldInfo.push_back(info);
  }

  return true;
}
  
///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the object values.
// Handles byte swapping.
// Returns true on success, false on failure

bool GenPoly::disassemble(const void *buf, int len)
{
  clear();
  _errStr = "ERROR - GenPoly::disassemble()\n";
  
  // check minimum len for header
  
  if (len < (int)sizeof(header_t))
  {
    TaStr::AddInt(_errStr, "  Buffer too short for header, len: ", len);
    TaStr::AddInt(_errStr, "  Minimum valid len: ",
		  sizeof(header_t));
    return false;
  }
  
  // local copy of buffer

  _memBuf.free();
  _memBuf.add(buf, len);
  
  // get header
  
  header_t *hdr = (header_t *)_memBuf.getPtr();
  _BE_to_header(*hdr);

  _time = hdr->time;
  _expireTime = _time;
  _nLevels = hdr->n_levels;
  _id = hdr->id;
  int n_vals = hdr->n_fields * _nLevels;

  // check expected len
  
  if (len != hdr->buf_len)
  {
    TaStr::AddInt(_errStr, "  Buffer wrong length, len: ", len);
    TaStr::AddInt(_errStr, "  Expected len: ", hdr->buf_len);
    TaStr::AddInt(_errStr, "  nVertices: ", hdr->n_vertices);
    TaStr::AddInt(_errStr, "  nFields: ", hdr->n_fields);
    TaStr::AddInt(_errStr, "  nLevels: ", _nLevels);
    return false;
  }
  
  // vertices

  vertex_t *vertex_ptr =
    (vertex_t *)((char *)_memBuf.getPtr() + sizeof(header_t));

  for (int i = 0; i < hdr->n_vertices; ++i)
  {
    _BE_to_vertex(*vertex_ptr);
    _vertices.push_back(*vertex_ptr);
    ++vertex_ptr;
  }
  
  // data values

  fl32 *vals = (fl32 *)vertex_ptr;
  BE_to_array_32(vals, n_vals * sizeof(fl32));
  for (int i = 0; i < n_vals; i++)
    _vals.push_back(vals[i]);

  // name string
  
  char *name = (char *) (vals + n_vals);
  if (hdr->name_len > 0)
  {
    name[hdr->name_len - 1] = '\0';
    _name = name;
  }
  
  // field info
  
  char *field_info = name + hdr->name_len;

  if (hdr->n_fields > 0)
  {
    field_info[hdr->field_info_len - 1] = '\0';
    string infoStr = field_info;

    if (!setFieldInfo(infoStr))
      return false;
  }
  
  if (hdr->n_fields != (int)_fieldInfo.size())
  {
    _errStr += "  Inconsistent number of fields\n";
    TaStr::AddInt(_errStr, "    Number set in header: ", hdr->n_fields);
    TaStr::AddInt(_errStr, "    Number from field info in header: ",
		  _fieldInfo.size());
    return false;
  }
  
  // text string

  char *text = field_info + hdr->field_info_len;
  text[hdr->text_len - 1] = '\0';
  _text = text;

  // id value -- since we just pulled out a text string, this value
  // might not be word-aligned.

  si32 id;
  memcpy(&id, text + hdr->text_len, sizeof(id));
  _id = BE_to_si32(id);
  
  // closed flag -- again, might not be word-aligned

  si32 closed_flag;
  memcpy(&closed_flag, (text + hdr->text_len + sizeof(_id)), sizeof(si32));
  closed_flag = BE_to_si32(closed_flag);
  if (closed_flag == 0)
    _closed = false;
  else
    _closed = true;
  
  return true;

}

//////////////////////////////
// get field info string

string GenPoly::getFieldInfoStr() const
{
  string fieldInfoStr;
  _combineFieldInfo(fieldInfoStr);
  return (fieldInfoStr);
}

///////////////////////////////////////
// Get a field number given the name
// Returns -1 on error.

int GenPoly::getFieldNum(const string &name) const
{
  for (size_t i = 0; i < _fieldInfo.size(); ++i)
  {
    if (_fieldInfo[i].name == name)
      return (int) i;
  }

  return -1;
}

///////////////////////////////////////
// Get a list of field numbers whose field names begin
// with the given string.

vector< int > GenPoly::getFieldListPrefix(const string &prefix) const
{
  vector< int > field_list;
  
  for (size_t i = 0; i < _fieldInfo.size(); ++i)
  {
    if (_fieldInfo[i].name.find(prefix) == 0)
      field_list.push_back((int)i);
  }

  return field_list;
}

////////////////////////////////////////////////////////
// get the field name and units, given the field number

string GenPoly::getFieldName(const int field_num) const
{
  if (field_num > (int)_fieldInfo.size() - 1)
  {
    cerr << "ERROR - GenPoly::getFieldName()" << endl;
    cerr << "  Field number: " << field_num <<  " out of range." << endl;
    cerr << "  Max field number: " << _fieldInfo.size() - 1 << endl;

    return "Invalid field number";
  }

  return _fieldInfo[field_num].name;
}

string GenPoly::getFieldUnits(const int field_num) const
{
  if (field_num > (int)_fieldInfo.size() - 1)
  {
    cerr << "ERROR - GenPoly::getFieldUnits()" << endl;
    cerr << "  Field number: " << field_num <<  " out of range." << endl;
    cerr << "  Max field number: " << _fieldInfo.size() - 1 << endl;

    return "Invalid field number";
  }

  return _fieldInfo[field_num].units;
}

////////////////////////////////////////////////
// get/reset data values

// get a value, given the 1D index
  
double GenPoly::get1DVal(const int index) const
{
  if (!check())
  {
    cerr << "ERROR - GenPoly::get1DVal()" << endl;
    cerr << _errStr;

    return 0.0;
  }

  if (_vals.size() == 0)
  {
    cerr << "ERROR - GenPoly::get1DVal()" << endl;
    cerr << "  Array index: " << index <<  " out of range." << endl;
    cerr << "  Array is empty." << endl;
    
    return 0.0;
  }

  if (index > (int)_vals.size() - 1)
  {
    cerr << "ERROR - GenPoly::get1DVal()" << endl;
    cerr << "  Array index: " << index <<  " out of range." << endl;
    cerr << "  Max index: " << _vals.size() - 1 << endl;
    
    return 0.0;
  }

  return _vals[index];
}
  
void GenPoly::set1DVal(const int index, const double value)
{
  if (!check())
  {
    cerr << "ERROR - GenPoly::set1DVal()" << endl;
    cerr << _errStr;

    return;
  }

  if (index > (int)_vals.size() - 1)
  {
    cerr << "ERROR - GenPoly::set1DVal()" << endl;
    cerr << "  Array index: " << index <<  " out of range." << endl;
    cerr << "  Max index: " << _vals.size() - 1 << endl;

    return;
  }

  _vals[index] = value;
}
  
// get a value, given the 2D indices
  
double GenPoly::get2DVal(const int level_num, const int field_num) const
{
  if (!check())
  {
    cerr << "ERROR - GenPoly::get2DVal()" << endl;
    cerr << _errStr;

    return 0.0;
  }

  int index = level_num * _fieldInfo.size() + field_num;

  if (index > (int)_vals.size() - 1)
  {
    cerr << "ERROR - GenPoly::getVal()" << endl;
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
  
void GenPoly::set2DVal(const int level_num, const int field_num,
		       const double value)
{
  if (!check())
  {
    cerr << "ERROR - GenPoly::set2DVal()" << endl;
    cerr << _errStr;

    return;
  }

  int index = level_num * _fieldInfo.size() + field_num;

  if (index > (int)_vals.size() - 1)
  {
    cerr << "ERROR - GenPoly::set2DVal()" << endl;
    cerr << "  Array index: " << index <<  " out of range." << endl;
    cerr << "  Max index: " << _vals.size() - 1 << endl;
    cerr << "  Requested level_num: " << level_num << endl;
    cerr << "  Max level number: " << _nLevels - 1 << endl;
    cerr << "  Requested field_num: " << field_num << endl;
    cerr << "  Max field number: " << _fieldInfo.size() - 1 << endl;

    return ;
  }

  _vals[index] = value;
}
  
///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.
//
// returns true on success, false on failure
// Use getErrStr() on failure.

bool GenPoly::assemble()
{
  // check if we have a valid object

  if (!check())
  {
    _errStr += "ERROR - GenPoly::assemble()\n";

    return false;
  }

  // combine the field info into a single string
  
  string fieldInfoStr;
  _combineFieldInfo(fieldInfoStr);

  // Initialize the buffer

  _memBuf.free();

  // load the header

  header_t hdr;
  MEM_zero(hdr);

  hdr.time = _time;
  hdr.n_vertices = _vertices.size();
  hdr.n_fields = _fieldInfo.size();
  hdr.n_levels = _nLevels;
  hdr.name_len = _name.size() + 1;
  hdr.field_info_len = fieldInfoStr.size() + 1;
  hdr.text_len = _text.size() + 1;
  hdr.buf_len = 
    sizeof(header_t) + (_vertices.size() * sizeof(vertex_t)) +
    (_vals.size() * sizeof(fl32)) + hdr.name_len + hdr.field_info_len +
    hdr.text_len + sizeof(si32) + sizeof(si32);
  hdr.id = _id;

  _BE_from_header(hdr);

  _memBuf.add(&hdr, sizeof(header_t));

  // Load the vertices

  vector< vertex_t >::iterator vertex_iter;
  
  for (vertex_iter = _vertices.begin();
       vertex_iter != _vertices.end(); ++vertex_iter)
  {
    vertex_t vertex_out = *vertex_iter;
    _BE_from_vertex(vertex_out);
    _memBuf.add(&vertex_out, sizeof(vertex_out));
  }
  
  // Load the data values

  for (size_t i = 0; i < _vals.size(); i++)
  {
    fl32 val = (fl32)_vals[i];
    BE_from_array_32(&val, sizeof(val));
    _memBuf.add(&val, sizeof(val));
  }
  
  _memBuf.add(_name.c_str(), _name.size() + 1);
  _memBuf.add(fieldInfoStr.c_str(), fieldInfoStr.size() + 1);
  _memBuf.add(_text.c_str(), _text.size() + 1);
  
  si32 id_out = BE_from_si32(_id);
  _memBuf.add(&id_out, sizeof(id_out));
  
  si32 closed_flag_out = _closed ? 1 : 0;
  closed_flag_out = BE_from_si32(closed_flag_out);
  _memBuf.add(&closed_flag_out, sizeof(closed_flag_out));
  
  return true;
}

////////////////////////////////////////////////////////
// prints

void GenPoly::print(FILE *out) const
{
  fprintf(out, "  ===============================\n");
  fprintf(out, "  GenPoly - generic polyline data\n");
  fprintf(out, "  ===============================\n");
  fprintf(out, "  time: %s\n", DateTime::str(_time).c_str());
  fprintf(out, "  expire time: %s\n", DateTime::str(_expireTime).c_str());
  fprintf(out, "  nvertices: %d\n", (int)_vertices.size());
  fprintf(out, "  nfields: %d\n", (int)_fieldInfo.size());
  fprintf(out, "  nlevels: %d\n", (int)_nLevels);
  fprintf(out, "  id: %d\n", _id);
  if (_closed)
    fprintf(out, "  closed: true\n");
  else
    fprintf(out, "  closed: false\n");
  
  for (size_t i = 0; i < _vertices.size(); ++i)
    fprintf(out, "    Vertex %d: %g  %g\n",
	    (int) i, _vertices[i].lat, _vertices[i].lon);
  
  fprintf(out, "  name: %s\n", _name.c_str());

  vector< double >::const_iterator value;
  vector< FieldInfo >::const_iterator field_info = _fieldInfo.begin();
  int level_num = 0;
  
  fprintf(out, "\nLevel %d fields:\n", level_num);

  for (value = _vals.begin(); value != _vals.end(); ++value, ++field_info)
  {
    if (field_info == _fieldInfo.end())
    {
      field_info = _fieldInfo.begin();
      ++level_num;
      fprintf(out, "\nLevel %d fields:\n", level_num);
    }
    
    fprintf(out, "     %s = %f %s\n",
	    (*field_info).name.c_str(), *value, (*field_info).units.c_str());
  }
  
  fprintf(out, "  text: %s\n", _text.c_str());
}

void GenPoly::print(ostream &out) const
{
  out << "  ===============================" << endl;
  out << "  GenPoly - generic polyline data" << endl;
  out << "  ===============================" << endl;
  out << "  time: " << DateTime::str(_time) << endl;
  out << "  expire time: " << DateTime::str(_expireTime) << endl;
  out << "  nvertices: " << _vertices.size() << endl;
  out << "  nfields: " << _fieldInfo.size() << endl;
  out << "  nlevels: " << _nLevels << endl;
  out << "  id: " << _id << endl;
  if (_closed)
    out << "  closed: true" << endl;
  else
    out << "  closed: false" << endl;
  
  for (size_t i = 0; i < _vertices.size(); ++i)
    out << "    Vertex " << i << ": " << _vertices[i].lat << " "
	<< _vertices[i].lon << endl;
  
  out << "  name: " << _name << endl;

  vector< double >::const_iterator value;
  vector< FieldInfo >::const_iterator field_info = _fieldInfo.begin();
  int level_num = 0;
  
  out << endl << "Level " << level_num << " fields:" << endl;

  for (value = _vals.begin(); value != _vals.end(); ++value, ++field_info)
  {
    if (field_info == _fieldInfo.end())
    {
      field_info = _fieldInfo.begin();
      ++level_num;
      out << endl << "Level " << level_num << " fields:" << endl;
    }
    
    out << "     " << (*field_info).name << " = " << *value << " "
	<< (*field_info).units << endl;
  }
  
  out << "  text: " << _text << endl;
}

void GenPoly::_printHeader(ostream &out, const header_t hdr)
{
  out << "  ===================" << endl;
  out << "  GenPoly -- header_t" << endl;
  out << "  ===================" << endl;
  out << "  time: " << DateTime::str(hdr.time) << endl;
  out << "  n_vertices: " << hdr.n_vertices << endl;
  out << "  n_fields: " << hdr.n_fields << endl;
  out << "  n_levels: " << hdr.n_levels << endl;
  out << "  name_len: " << hdr.name_len << endl;
  out << "  field_info_len: " << hdr.field_info_len << endl;
  out << "  text_len: " << hdr.text_len << endl;
  out << "  buf_len: " << hdr.buf_len << endl;
  out << "  id: " << hdr.id << endl;
  out << "  closed_flag: " << hdr.closed_flag << endl;
}

////////////////////////////////////////////////
// combine the field info into a single string

void GenPoly::_combineFieldInfo(string &fieldInfoStr) const
{  
  for (size_t i = 0; i < _fieldInfo.size(); i++)
  {
    fieldInfoStr += _fieldInfo[i].name;
    fieldInfoStr += ":";
    fieldInfoStr += _fieldInfo[i].units;
    if (i != _fieldInfo.size() - 1)
      fieldInfoStr += ",";
  }
}

/////////////////////////////////////////////////////////
// byte swapping routines

void GenPoly::_BE_from_header(header_t &hdr)
{
  BE_from_array_32(&hdr, sizeof(header_t));
}

void GenPoly::_BE_to_header(header_t &hdr)
{
  BE_to_array_32(&hdr, sizeof(header_t));
}

void GenPoly::_BE_from_vertex(vertex_t &vertex)
{
  BE_from_array_32(&vertex, sizeof(vertex_t));
}

void GenPoly::_BE_to_vertex(vertex_t &vertex)
{
  BE_to_array_32(&vertex, sizeof(vertex_t));
}

/////////////////////////////////////////////////////////
// calculations

void GenPoly::calcCentroid(double &centroid_lat, double &centroid_lon) const
{
  vector< vertex_t >::const_iterator vertex;
  
  double lat_total = 0.0;
  double lon_total = 0.0;
  
  for (vertex = _vertices.begin(); vertex != _vertices.end(); ++vertex)
  {
    lat_total += vertex->lat;
    lon_total += vertex->lon;
  } /* endfor - vertex */
  
  centroid_lat = lat_total / (double)_vertices.size();
  centroid_lon = lon_total / (double)_vertices.size();
}

void GenPoly::calcCentroid(float &centroid_lat, float &centroid_lon) const
{
  double centroid_lat_double, centroid_lon_double;
  
  calcCentroid(centroid_lat_double, centroid_lon_double);
  
  centroid_lat = centroid_lat_double;
  centroid_lon = centroid_lon_double;
}
