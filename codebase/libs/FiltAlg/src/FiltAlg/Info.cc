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
 * @file Info.cc
 */
#include <FiltAlg/Info.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#ifndef NO_DS
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <Spdb/DsSpdb.hh>
#endif

//------------------------------------------------------------------
Info::Info(void)
{
  _well_formed = false;
  _time = 0;
  _radar_name = "unknown";
  _radar_loc = "unknown";
  _radar_lat = 0;
  _radar_lon = 0;
  _radar_alt_km = 0;
  _data1.clear();
  _data2.clear();
}

//------------------------------------------------------------------
Info::~Info()
{
}

//------------------------------------------------------------------
void Info::clear(void)
{
  _well_formed = false;
  _time = 0;
  _radar_name = "unknown";
  _radar_loc = "unknown";
  _radar_lat = 0;
  _radar_lon = 0;
  _radar_alt_km = 0;
  _data1.clear();
  _data2.clear();
  dclear();
}

//------------------------------------------------------------------
void Info::info_print(const LogStream::Log_t e) const
{
  LOGV(e) << DateTime::strn(_time);
  LOGV(e) << "radar:" << _radar_name;
  LOGV(e) << "      " << _radar_loc;
  LOGV(e) << "      lat:" << _radar_lat << ", lon:" << _radar_lon 
	  << ", alt:" << _radar_alt_km;
  dprint(e);
}

//------------------------------------------------------------------
void Info::init(const time_t &t, const double lat,
		const double lon, const double alt,
		const string &radar_name, const string &radar_loc)
{
  _time = t;
  _radar_name = radar_name;
  _radar_loc = radar_loc;
  _radar_lat = lat;
  _radar_lon = lon;
  _radar_alt_km = alt;
  _data1.clear();
  _data2.clear();
  dinit();
}

//------------------------------------------------------------------
void Info::add_data2d(const Data2d &d, const Data2d &vlevel)
{
  string vname = data2d_vlevel_name();
  bool has_v = false;
  for (int i=0; i<static_cast<int>(_data2.size()); ++i)
  {
    if (_data2[i].get_name() == vname)
    {
      has_v = true;
      break;
    }
  }
  if (!has_v)
  {
    _data2.push_back(vlevel);
  }
  _data2.push_back(d);
}

//------------------------------------------------------------------
bool Info::add_data1d(const Data1d &d)
{
  if (d.is_bad())
  {
    LOG(ERROR) << "bad input";
    return false;
  }
  else
  {
    _data1.push_back(d);
    return true;
  }
}

//------------------------------------------------------------------
void Info::store(DsMdvx &M) const
{
#ifdef NO_DS
  LOG(ERROR) << "DsMdvx not implemented";
  return;
#else
  Mdvx::chunk_header_t h;
  h.chunk_id = Mdvx::CHUNK_COMMENT;
  string s = write_xml();
  h.size = static_cast<int>(s.size()) + 1;  //make sure null terminated
  sprintf(h.info, "Info");
  MdvxChunk *c = new MdvxChunk(h, s.c_str());
  M.addChunk(c);
#endif
}

//------------------------------------------------------------------
bool Info::retrieve(DsMdvx &D)
{
#ifdef NO_DS
  LOG(ERROR) << "DsMdv not implemented";
  return false;
#else
  // get the chunk
  int nc = D.getNChunks();
  for (int i=0; i<nc; ++i)
  {
    MdvxChunk *c;
    c = new MdvxChunk(*D.getChunkByNum(i));

    // this is a little weak as there could be other CHUNK_COMMENT chunks
    if (c->getId() != Mdvx::CHUNK_COMMENT)
    {
      delete c;
      continue;
    }
    int n = c->getSize();
    const char *d = (const char *)c->getData();
    // make sure it is null terminated by a copy
    char *buf = new char[n+1];
    strncpy(buf, d, n);
    bool ret = set_from_xml(buf);
    delete [] buf;
    if (!ret)
    {
      LOG(WARNING) << "Expected comment chunk to be Info, failure";
    }
    else
    {
      return true;
    }
  }
  LOG(ERROR) << "No comment chunk parsed as Info";
  return false;
#endif
}

//------------------------------------------------------------------
void Info::output(const string &spdb_url)
{
#ifdef NO_DS
  LOG(ERROR) << "DsSpdb not implemented";
  return;
#else
  DsSpdb s;
  s.clearPutChunks();
  s.setPutMode(Spdb::putModeOver);
  s.clearUrls();
  s.addUrl(spdb_url);

  assemble();
  if (s.put(SPDB_XML_ID, SPDB_XML_LABEL, 1, _time, _time + 300,
	    getBufLen(), (void *)getBufPtr()))
  {
    LOG(ERROR) << "problems writing out SPDB";
  }
#endif
}

//------------------------------------------------------------------
string Info::write_xml(void) const
{
  string s;

  s = TaXml::writeStartTag("Info", 0);
  s += TaXml::writeTime("Time", 0, _time);
  s += TaXml::writeString("Radar", 0, _radar_name);
  s += TaXml::writeString("RadarLoc", 0, _radar_loc);
  s += TaXml::writeDouble("Latitude", 0, _radar_lat, "%.5lf");
  s += TaXml::writeDouble("Longitude", 0, _radar_lon, "%.5lf");
  s += TaXml::writeDouble("Altitude_km", 0, _radar_alt_km, "%.5lf");
  if (!_data1.empty() || !_data2.empty())
  {
    s += TaXml::writeStartTag("GenericInfo", 0);
    for (int i=0; i<static_cast<int>(_data2.size()); ++i)
    {
      s += _data2[i].write_xml();
    }
    for (int i=0; i<static_cast<int>(_data1.size()); ++i)
    {
      s += _data1[i].write_xml();
    }
    s += TaXml::writeEndTag("GenericInfo", 0);
  }
  s += dwrite_xml();
  s += TaXml::writeEndTag("Info", 0);
  return s;
}

//------------------------------------------------------------------
void Info::write_xml(const string &path) const
{
  FILE *fp = fopen(path.c_str(), "w");
  if (fp != NULL)
  {
    string s = write_xml();
    fprintf(fp, "%s\n", s.c_str());
    fclose(fp);
  }
  else
  {
    LOG(ERROR) << "Opening file " << path;
  }
}

//------------------------------------------------------------------
void Info::append_xml(const string &pth) const
{
  FILE *fp = fopen(pth.c_str(), "a");
  if (fp != NULL)
  {
    string s = write_xml();
    fprintf(fp, "%s\n", s.c_str());
    fclose(fp);
  }
  else
  {
    LOG(ERROR) << "Opening file " << pth;
  }
}

//------------------------------------------------------------------
bool Info::set_from_xml(const string &s)
{
  clear();
  _well_formed = true;
  if (TaXml::readTime(s, "Time", _time) != 0)
  {
    LOG(ERROR) << "ERROR reading tag Time";
    _well_formed = false;
  }
  if (TaXml::readString(s, "Radar", _radar_name) != 0)
  {
    LOG(ERROR) << "ERROR reading tag Radar";
    _well_formed = false;
  }
  if (TaXml::readString(s, "RadarLoc", _radar_loc) != 0)
  {
    LOG(ERROR) << "ERROR reading tag RadarLoc";
    _well_formed = false;
  }
  if (TaXml::readDouble(s, "Latitude", _radar_lat) != 0)
  {
    LOG(ERROR) << "ERROR reading tag Latitude";
    _well_formed = false;
  }
  if (TaXml::readDouble(s, "Longitude", _radar_lon) != 0)
  {
    LOG(ERROR) << "ERROR reading tag Longitude";
    _well_formed = false;
  }
  if (TaXml::readDouble(s, "Altitude_km", _radar_alt_km) != 0)
  {
    LOG(ERROR) << "ERROR reading tag Altitude";
    _well_formed = false;
  }
  _data2.clear();
  _data1.clear();
  string locs;
  vector<string> locv;
  if (TaXml::readString(s, "GenericInfo", locs) == 0)
  {
    if (locs.size() > 1)
    {
      if (TaXml::readStringArray(locs, "Data2d", locv) == 0)
      {
	for (int i=0; i<static_cast<int>(locv.size()); ++i)
	{
	  Data2d d;
	  if (d.set_from_xml(locv[i]))
	  {
	    _data2.push_back(d);
	  }
	  else
	  {
	    _well_formed = false;
	  }
	}
      }
      if (TaXml::readStringArray(locs, "Data1d", locv) == 0)
      {
	for (int i=0; i<static_cast<int>(locv.size()); ++i)
	{
	  Data1d d;
	  if (d.set_from_xml(locv[i]))
	  {
	    _data1.push_back(d);
	  }
	  else
	  {
	    _well_formed = false;
	  }
	}
      }
    }
  }
  if (!dset_from_xml(s))
  {
    _well_formed = false;
  }
  return _well_formed;
}

//------------------------------------------------------------------
void Info::assemble()
{
  // check mem buffer is free
  _memBuf.free();
  
  // convert to XML string
  string xml = write_xml();

  // add xml string to buffer, including trailing null
  _memBuf.add(xml.c_str(), xml.size() + 1);
}

//------------------------------------------------------------------
int Info::disassemble(const void *buf, int len)
{
  string xml;
  xml.append((char *)buf, len-1);
  if (set_from_xml(xml))
  {
    return 0;
  }
  else
  {
    return -1;
  }
}
