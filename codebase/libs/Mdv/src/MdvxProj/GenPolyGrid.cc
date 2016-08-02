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
/**
 * @file GenPolyGrid.cc
 */

#include <Mdv/GenPolyGrid.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <euclid/Grid2dPolyFinder.hh>
#include <euclid/Grid2dEdgeBuilder.hh>
#include <euclid/Grid2dInside.hh>
#include <euclid/sincos.h>

#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>

#define BAD -99.0
#define GOOD 1.0

//----------------------------------------------------------------
static void _smooth(vector<double> &lat, vector<double> &lon, 
		    const int npt_smooth, const double max_pcnt_smooth)
{
  vector<double> lat2, lon2;

  int nll = static_cast<int>(lat.size());
  int npt;
  double p = static_cast<double>(npt_smooth)/static_cast<double>(nll);
  if (p > max_pcnt_smooth)
  {
    npt = static_cast<int>(max_pcnt_smooth*static_cast<double>(nll));
  }
  else
  {
    npt = npt_smooth;
  }

  for (int i=0; i<nll; ++i)
  {
    int imin = i - npt;
    int imax = i + npt;
    double alat=0.0;
    double alon=0.0;
    double n = 0.0;
    for (int j=imin; j<=imax; ++j)
    {
      int k;
      if (j < 0)
      {
	k = j + nll;
      }
      else if (j >= nll)
      {
	k = j - nll;
      }
      else
      {
	k = j;
      }
      alat += lat[k];
      alon += lon[k];
      ++n;
    }
    alat /= n;
    alon /= n;
    lat2.push_back(alat);
    lon2.push_back(alon);
  }

  lat = lat2;
  lon = lon2;
}

//----------------------------------------------------------------
/**
 * @param[in] P  Polyfinder
 * @param[in] grid  Grid with one clump of non-missing data
 */
static bool _polybuild(Grid2dPolyFinder &P, const Grid2d &grid)
{
  if (!P.init(grid, 1))
  {
    return false;
  }
  while (P.next())
  {
    ;
  }
  P.removeLines();
  return true;
}

//----------------------------------------------------------------
static bool _polybuild_boxes(Grid2dPolyFinder &P, const Grid2d &grid)
{
  // make a different grid where expanded into 4 points at each point
  GridAlgs gbox(grid);
  gbox.boxExpand();

  if (!P.init(gbox, 1))
  {
    return false;
  }
  while (P.next())
  {
    ;
  }
  P.removeLines();
  return true;
}

//----------------------------------------------------------------
static void _get_one_latlon(Grid2dPolyFinder &P, const int i, const MdvxProj &proj,
			    double &dlat, double &dlon)
{
  int ix, iy;
  P.getIth(i, ix, iy);
  proj.xyIndex2latlon(ix, iy, dlat, dlon);
}

//----------------------------------------------------------------
static void _get_one_latlon_box(Grid2dPolyFinder &P, const int i,
				const MdvxProj &proj,
				const double delta_lat, 
				const double delta_lon,
				double &dlat, double &dlon)
{

  int ix, iy;
  P.getIth(i, ix, iy);
  proj.xyIndex2latlon(ix, iy, dlat, dlon);
  //   dlat -= delta_lat;
  //   dlon -= delta_lon;
}

//----------------------------------------------------------------
GenPolyGrid::GenPolyGrid(void) : GenPoly()
{
}

//----------------------------------------------------------------
GenPolyGrid::GenPolyGrid(const GenPoly &b) : GenPoly(b)
{
}

//----------------------------------------------------------------
GenPolyGrid::GenPolyGrid(const GenPolyGrid &b) 
{
  _time = b._time;
  _expireTime = b._expireTime;
  _nLevels = b._nLevels;
  _id = b._id;
  _closed = b._closed;
  _name = b._name;
  _text = b._text;
  _errStr = b._errStr;
  _vertices = b._vertices;
  _vals = b._vals;
  _fieldInfo = b._fieldInfo;
  _memBuf = b._memBuf;
}

//----------------------------------------------------------------
GenPolyGrid::~GenPolyGrid()
{

}

//----------------------------------------------------------------
GenPolyGrid & GenPolyGrid::operator=(const GenPolyGrid &b)
{
  if (this == &b)
  {
    return *this;
  }
  _time = b._time;
  _expireTime = b._expireTime;
  _nLevels = b._nLevels;
  _id = b._id;
  _closed = b._closed;
  _name = b._name;
  _text = b._text;
  _errStr = b._errStr;
  _vertices = b._vertices;
  _vals = b._vals;
  _fieldInfo = b._fieldInfo;
  _memBuf = b._memBuf;
  return *this;
}

//----------------------------------------------------------------
bool GenPolyGrid::operator==(const GenPolyGrid &b) const
{
  if (!isRedundant(b))
  {
    return false;
  }

  if (b._fieldInfo.size() != _fieldInfo.size())
  {    
    return false;
  }

  vector<FieldInfo>::const_iterator j, jb;
  for (jb=b._fieldInfo.begin(),j=_fieldInfo.begin(); j!=_fieldInfo.end();
       ++j,++jb)
  {
    if (jb->name != j->name || jb->units != j->units)
    {
      return false;
    }
  }
  return (_time == b._time &&
	  _expireTime == b._expireTime &&
	  _nLevels == b._nLevels &&
	  _id == b._id &&
	  _closed == b._closed &&
	  _name == b._name &&
	  _text == b._text &&
	  _errStr == b._errStr &&
	  _vals == b._vals);
  // 	  _memBuf == b._memBuf);// not sure about this!
}

//----------------------------------------------------------------
void GenPolyGrid::print(const bool full) const
{
  print("", full);
}

//----------------------------------------------------------------
void GenPolyGrid::print(const char *indent, const bool full) const
{
  if (full)
  {
    GenPoly::print(stdout);
  }
  else
  {
    printf("%sGenPoly id:%d  time:%s npt:%d\n", indent, getId(),
	   DateTime::strn(getTime()).c_str(), getNumVertices());
  }
}

//----------------------------------------------------------------
bool GenPolyGrid::set(const time_t &t, const time_t &t_expire,
		      const int id, const GridAlgs &grid, const MdvxProj &proj)
{
  Grid2dPolyFinder P;

  // init with input values
  _setInit("Poly", id, t, t_expire, true);

  // set up the index values
  if (!_polybuild(P, grid))
  {
    return false;
  }
  // store as lat/lons
  GenPoly::vertex_t vertex;
  double dlat, dlon;
  for (int i=0; i<P.num(); ++i)
  {
    _get_one_latlon(P, i, proj, dlat, dlon);
    vertex.lat = dlat;
    vertex.lon = dlon;
    GenPoly::addVertex(vertex);
  }
  return true;
}

//----------------------------------------------------------------
bool GenPolyGrid::setBoxes(const time_t &t, const time_t &t_expire,
			   const int id, const GridAlgs &grid,
			   const MdvxProj &proj)
{
  Grid2dPolyFinder P;

  // init
  _setInit("Poly", id, t, t_expire, true);

  // set up the index values
  if (!_polybuild_boxes(P, grid))
  {
    return false;
  }

  // store as lat/lons
  GenPoly::vertex_t vertex;
  double dlat, dlon;

  // get grid spacing at center of grid for reference (half width)
  int nx, ny;
  nx = grid.getNx();
  ny = grid.getNy();
  double delta_lat, delta_lon;
  proj.xyIndex2latlon(nx/2, ny/2, dlat, dlon);
  proj.xyIndex2latlon(nx/2+1, ny/2+1, delta_lat, delta_lon);
  delta_lat = (delta_lat - dlat)/2.0;
  delta_lon = (delta_lon - dlon)/2.0;

  for (int i=0; i<P.num(); ++i)
  {
    _get_one_latlon_box(P, i, proj, delta_lat, delta_lon, dlat, dlon);
    vertex.lat = dlat;
    vertex.lon = dlon;
    GenPoly::addVertex(vertex);
  }
  return true;
}

//----------------------------------------------------------------
bool GenPolyGrid::setAndSmooth(const time_t &t, const time_t &t_expire,
			       const int id, const GridAlgs &grid,
			       const MdvxProj &proj, const int npt_smooth,
			       const double max_pcnt_smooth)
{
  Grid2dPolyFinder P;

  // init
  _setInit("Poly", id, t, t_expire, true);

  // set up the index values
  if (!_polybuild(P, grid))
  {
    return false;
  }

  // put all the vertices to vectors.
  vector<double> vlat, vlon;

  for (int i=0; i<P.num(); ++i)
  {
    double dlat, dlon;
    _get_one_latlon(P, i, proj, dlat, dlon);
    vlat.push_back(dlat);
    vlon.push_back(dlon);
  }

  // smooth the lat/lons.
  _smooth(vlat, vlon, npt_smooth, max_pcnt_smooth);

  // store all unique smoothed latlons/
  GenPoly::vertex_t vertex, vlast;
  for (size_t i=0; i<vlat.size(); ++i)
  {
    vertex.lat = vlat[i];
    vertex.lon = vlon[i];
    if (i == 0)
    {
      GenPoly::addVertex(vertex);
      vlast = vertex;
    }
    else
    {
      if (vertex.lat != vlast.lat || vertex.lon != vlast.lon)
      {
	GenPoly::addVertex(vertex);
	vlast = vertex;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------
bool GenPolyGrid::get(const MdvxProj &p, Grid2d &g, int &num) const
{
  num = 0;

  // for each lat/lon map to an x,y in the grid
  if (isEmpty())
  {
    LOG(WARNING) << "Read in an empty polygon?";
    return false;
  }

  fl32 id = static_cast<fl32>(getId());

  Grid2dEdgeBuilder E(g);
  for (int i=0; i<getNumVertices(); ++i)
  {
    int x, y;
    GenPoly::vertex_t v = getVertex(i);
    p.latlon2xyIndex(v.lat, v.lon, x, y);
    E.addVertex(x, y);
  }  
  if (E.bad())
  {
    LOG(ERROR) << "REALLY BAD ERROR";
    return false;
  }
  
  // use range of clump to define an 'inside' object.
  // note assumed the input lat/lons define an edge.
  Grid2dInside I(E);
  for (int y=0; y<I.ny(); ++y)
  {
    for (int x=0; x<I.nx(); ++x)
    {
      int xi, yi;
      if (I.inside(x, y, xi, yi))
      {
	g.setValue(xi, yi, id);
	++num;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------
bool GenPolyGrid::isRedundant(const GenPolyGrid &b) const
{
  if (b._vertices.size() != _vertices.size())
  {
    return false;
  }
  vector<vertex_t>::const_iterator i, ib;
  for (ib=b._vertices.begin(),i=_vertices.begin(); i!=_vertices.end();
       ++i,++ib)
  {
    if (ib->lat != i->lat || ib->lon != i->lon)
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------
void GenPolyGrid::fillEmpty(const int id, const time_t t)
{
  _setInit("emptypoly", id, t, t, false);
  string s = "empty";
  setText(s);

  // make 2 vertices far far away from anything.
  vertex_t v;
  v.lat = 0.0;
  v.lon = 0.0;
  addVertex(v);

  v.lat = 1.0;
  v.lon = 1.0;
  addVertex(v);
}

//----------------------------------------------------------------
bool GenPolyGrid::isEmpty(void) const
{
  if (getNumVertices() != 2)
  {
    return false;
  }
  GenPoly::vertex_t v;
  v = getVertex(0);
  if (v.lat != 0 || v.lon != 0)
  {
    return false;
  }
  v = getVertex(1);
  if (v.lat != 1 || v.lon != 1)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------
void GenPolyGrid::setNoScore(void)
{
  clearVals();
  clearFieldInfo();
}

//----------------------------------------------------------------
void GenPolyGrid::setScore(const double score, const string &name,
			   const string &units)
{
  clearVals();
  clearFieldInfo();
  addFieldInfo(name, units);
  addVal(score);
}

//----------------------------------------------------------------
bool GenPolyGrid::getScore(double &score) const
{
  score = get1DVal(0);
  return true;  // no way to know!
}

//----------------------------------------------------------------
void GenPolyGrid::saveGenpolygridState(string &buf, int indent) const
{
  string tag = stateTag();
  buf += TaXml::writeStartTag(tag, indent);

  buf += TaXml::writeTime("Time", indent+1, _time);
  buf += TaXml::writeTime("ExpireTime", indent+1, _expireTime);
  buf += TaXml::writeInt("nLevels", indent+1, _nLevels);
  buf += TaXml::writeInt("Id", indent+1, _id);
  buf += TaXml::writeBoolean("Closed", indent+1, _closed);
  buf += TaXml::writeString("Name", indent+1, _name);
  buf += TaXml::writeString("Text", indent+1, _text);
  buf += TaXml::writeString("ErrStr", indent+1, _errStr);
  buf += TaXml::writeStartTag("Vertices", indent+1);
  for (size_t i=0; i < _vertices.size(); ++i)
  {
    buf += TaXml::writeStartTag("Vertex", indent+2);
    buf += TaXml::writeDouble("Lat", indent+3, _vertices[i].lat, "%10.5f");
    buf += TaXml::writeDouble("Lon", indent+3, _vertices[i].lon, "%10.5f");
    buf += TaXml::writeEndTag("Vertex", indent+2);
  }
  buf += TaXml::writeEndTag("Vertices", indent+1);

  buf += TaXml::writeStartTag("Vals", indent+1);
  for (size_t i=0;  i < _vals.size(); ++i)
  {
    buf += TaXml::writeDouble("Val", indent+2, _vals[i], "%10.5f");
  }
  buf += TaXml::writeEndTag("Vals", indent+1);

  buf += TaXml::writeStartTag("FieldInfo", indent+1);
  for (size_t i=0; i<_fieldInfo.size(); ++i)
  {
    buf += TaXml::writeStartTag("FieldInfo1", indent+2);
    buf += TaXml::writeString("FI_Name", indent+3, _fieldInfo[i].name);
    buf += TaXml::writeString("Units", indent+3, _fieldInfo[i].units);
    buf += TaXml::writeEndTag("FieldInfo1", indent+2);
  }
  buf += TaXml::writeEndTag("FieldInfo", indent+1);

  tag = stateTag();
  buf += TaXml::writeEndTag(tag, indent);
}

//----------------------------------------------------------------
bool GenPolyGrid::retrieveGenpolygridState(const string &s)
{
  bool stat = true;
  if (TaXml::readTime(s, "Time", _time) == -1)
  {
    LOG(ERROR) << "reading Time";
    stat = false;
    _time = 0;
  }

  if (TaXml::readTime(s, "ExpireTime", _expireTime)  == -1)
  {
    LOG(ERROR) << "reading ExpireTime";
    stat = false;
    _expireTime = 0;
  }

  if (TaXml::readInt(s, "nLevels", _nLevels)  == -1)
  {
    LOG(ERROR) << "reading nLevels";
    stat = false;
    _nLevels = 1;
  }

  if (TaXml::readInt(s, "Id", _id)  == -1)
  {
    LOG(ERROR) << "reading Id";
    stat = false;
    _id = 0;
  }

  if (TaXml::readBoolean(s, "Closed", _closed)  == -1)
  {
    LOG(ERROR) << "reading Closed";
    stat = false;
    _closed = false;
  }

  if (TaXml::readString(s, "Name", _name)  == -1)
  {
    LOG(ERROR) << "reading Name";
    stat = false;
    _name = "";
  }

  if (TaXml::readString(s, "Text", _text)  == -1)
  {
    LOG(ERROR) << "reading Text";
    stat = false;
    _text = "";
  }

  if (TaXml::readString(s, "ErrStr", _errStr)  == -1)
  {
    LOG(ERROR) << "reading ErrStr";
    stat = false;
    _errStr = "";
  }

  string s2;
  _vertices.clear();
  if (TaXml::readString(s, "Vertices", s2) == -1)
  {
    LOG(ERROR) << "reading Vertices";
    stat = false;
  }
  else
  {
    vector<string> v;
    if (TaXml::readStringArray(s2, "Vertex", v) == -1)
    {
      LOG(ERROR) << "reading Vertex array";
      stat = false;
    }
    else
    {
      vector<string>::iterator i;
      for (i=v.begin(); i!=v.end(); ++i)
      {
	double lat, lon;
	if (TaXml::readDouble(*i, "Lat", lat) == -1)
	{
	  LOG(ERROR) << "reading Lat";
	  stat = false;
	}
	else
	{
	  if (TaXml::readDouble(*i, "Lon", lon) == -1)
	  {
	    LOG(ERROR) << "reading Lon";
	    stat = false;
	  }
	  else
	  {
	    vertex_t vertex;
	    vertex.lat = lat;
	    vertex.lon = lon;
	    _vertices.push_back(vertex);
	  }
	}
      }
    }
  }


  _vals.clear();
  if (TaXml::readString(s, "Vals", s2) == -1)
  {
    LOG(ERROR) << "reading Vals";
    stat = false;
  }
  else
  {
    vector<string> v;
    if (TaXml::readStringArray(s2, "Val", v) == -1)
    {
      LOG(ERROR) << "No Vals";
    }
    else
    {
      vector<string>::iterator i;
      for (i=v.begin(); i!=v.end(); ++i)
      {
	double val;
	if (sscanf(i->c_str(), "%lf", &val) != 1)
	{
	  LOG(ERROR) << "reading a val " <<  *i;
	  stat = false;
	}
	else
	{
	  _vals.push_back(val);
	}
      }
    }
  }

  _fieldInfo.clear();
  if (TaXml::readString(s, "FieldInfo", s2) == -1)
  {
    LOG(ERROR) << "reading FieldInfo";
    stat = false;
  }
  else
  {
    vector<string> v;
    if (TaXml::readStringArray(s2, "FieldInfo1", v) == -1)
    {
      LOG(ERROR) << "No Field info";
    }
    else
    {
      vector<string>::iterator i;
      for (i=v.begin(); i!=v.end(); ++i)
      {
	string name, units;
	if (TaXml::readString(*i, "FI_Name", name) == -1)
	{
	  LOG(ERROR) << "reading FI_Name";
	  stat = false;
	}
	else
	{
	  if (TaXml::readString(*i, "Units", units) == -1)
	  {
	    LOG(ERROR) << "reading Units";
	    stat = false;
	  }
	  else
	  {
	    FieldInfo info;
	    info.name = name;
	    info.units = units;
	    _fieldInfo.push_back(info);
	  }
	}
      }
    }
  }
  return stat;
}

//----------------------------------------------------------------
bool GenPolyGrid::mdvToGrid2d(const Mdvx &M, const std::string &fieldName,
			      Grid2d &G)
{
  MdvxField *f = M.getFieldByName(fieldName);
  if (f == NULL)
  {
    LOG(ERROR) << "reading field " << fieldName;
    return false;
  }
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  Mdvx::field_header_t hdr = f->getFieldHeader();
  if (hdr.nz > 1)
  {
    LOG(ERROR) << "Cannot convert 3d MDV to a Grid2d object";
    return false;
  }
  G = Grid2d(fieldName, hdr.nx, hdr.ny, hdr.missing_data_value);

  fl32 *data = (fl32 *)f->getVol();
  for (int j=0; j<hdr.nx*hdr.ny; ++j)
  {
    if (data[j] != hdr.bad_data_value && data[j] != hdr.missing_data_value)
    {
      G.setValue(j, static_cast<double>(data[j]));
    }
  }
  return true;
}

//----------------------------------------------------------------
bool GenPolyGrid::
grid2dToDsMdvx(DsMdvx &D, const std::vector<Grid2d> &grids,
	       const std::vector<std::pair<std::string,std::string> > &nU)
{
  MdvxField *fin = D.getField(0);
  if (fin == NULL)
  {
    LOG(ERROR) << "reading a field";
    return false;
  }
  Mdvx::field_header_t hdr = fin->getFieldHeader();
  if (hdr.nz > 1)
  {
    LOG(ERROR) << "Cannot use 3d DsMdvx input";
    return false;
  }

  // make a copy of this field to be deleted later
  MdvxField *fcopy = new MdvxField(*fin);


  // want to clear out all fields, and substutute new fields
  // created from input Grid2d data

  D.clearFields();

  for (size_t i=0; i<grids.size(); ++i)
  {
    if (!_addField(grids[i], nU, fcopy, D))
    {
      delete fcopy;
      return false;
    }
  }

  delete fcopy;
  return true;
}

//----------------------------------------------------------------
bool GenPolyGrid::
_addField(const Grid2d &grid,
	  const std::vector<std::pair<std::string,std::string> > &nU,
	  const MdvxField *field, 
	  DsMdvx &D)
{
  int nx = grid.getNx();
  int ny = grid.getNy();
  Mdvx::field_header_t hdr = field->getFieldHeader();

  if (nx != hdr.nx || ny != hdr.ny)
  {
    LOG(ERROR) << "Inconsistency in (nx,ny)  Mdv:("
	       << hdr.nx << "," << hdr.ny << ") Grid:("
	       << nx << "," << ny << ")";
    return false;
  }

  string name = grid.getName();
  string units = "bad bad bad";
  for (size_t j=0; j<nU.size(); ++j)
  {
    if (nU[j].first == name)
    {
      units = nU[j].second;
      break;
    }
  }
  if (units == "bad bad bad")
  {
    LOG(ERROR) << "Units for " << name << " not set";
    return false;
  }

  // create a field to pass in
  MdvxField *f = new MdvxField(*field);
  Mdvx::field_header_t fh = f->getFieldHeader();
  fh.bad_data_value = grid.getMissing();
  fh.missing_data_value = grid.getMissing();
  f->setFieldHeader(fh);
  f->setFieldName(name.c_str());
  f->setFieldNameLong(name.c_str());
  f->setUnits(units.c_str());

  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  fl32 *data = (fl32 *)f->getVol();
  for (int i=0; i<nx*ny; ++i)
  {
    data[i] = (fl32)grid.getValue(i);
  }
  f->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_GZIP);

  // add this field
  D.addField(f);
  return true;
}

//----------------------------------------------------------------
void GenPolyGrid::_setInit(const string &name, const int id, const time_t &t,
			   const time_t &t_expire, const bool closed)
{
  GenPoly::clear();
  GenPoly::setName("Poly");
  GenPoly::setId(id);
  GenPoly::setTime(t);
  GenPoly::setExpireTime(t_expire);
  GenPoly::setNLevels(1);
  GenPoly::setClosedFlag(closed);
  GenPoly::clearVertices();
  setNoScore();
}
