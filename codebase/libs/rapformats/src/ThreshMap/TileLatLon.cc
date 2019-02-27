// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

/**
 * @file TileLatLon.cc
 */
#include <cstdio>
#include <rapformats/TileLatLon.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>

const std::string TileLatLon::_tag = "LatLons";

//----------------------------------------------------------------
TileLatLon::TileLatLon(void)  : _ok(false)
{
}

//----------------------------------------------------------------
TileLatLon::TileLatLon(const std::string &xml)  : _ok(true)
{
  string block;
  if (TaXml::readString(xml, _tag, block))
  {
    _ok = false;
    LOG(ERROR) << "No Tag in data " << _tag;
    return;
  }
  vector<string> latlons;
  if (TaXml::readStringArray(block, "LatLon", latlons))
  {
    LOG(ERROR) << "Reading latlon array with tag LatLon";
    _ok = false;
    return;
  }
  for (size_t i=0; i<latlons.size(); ++i)
  {
    int index;
    double lat, lon;
    bool ithIsOk = true;
    if (TaXml::readInt(latlons[i], "TileIndex", index))
    {
      LOG(ERROR) << "Reading " << i << "th TileIndex";
      _ok = false;
      ithIsOk = false;
    }
    if (TaXml::readDouble(latlons[i], "Lat", lat))
    {
      LOG(ERROR) << "Reading " << i << "th Lat";
      _ok = false;
      ithIsOk = false;
    }
    if (TaXml::readDouble(latlons[i], "Lon", lon))
    {
      LOG(ERROR) << "Reading " << i << "th Lon";
      _ok = false;
      ithIsOk = false;
    }
    if (ithIsOk)
    {
      _latlon[index] = pair<double,double>(lat, lon);
    }
  }
}

//----------------------------------------------------------------
TileLatLon::~TileLatLon()
{
}

//----------------------------------------------------------------
bool TileLatLon::operator==(const TileLatLon &t) const
{
  if (_ok != t._ok)
  {
    return false;
  }
  if (_ok)
  {
    return _latlon == t._latlon;
  }
  else
  {
    return true;
  }
}

//----------------------------------------------------------------
void TileLatLon::printDiffs(const TileLatLon &t) const
{
  if (_ok != t._ok)
  {
    LOG(ERROR) << "OKs diff";
  }
  int lsize = (int)_latlon.size();
  int isize = (int)(t._latlon.size());
  if (lsize != isize)
  {
    LOG(ERROR) << "Sizes diff " << isize << " " << lsize;
    if (lsize < isize)
    {
      isize = lsize;
    }
  }

  std::map<int, std::pair<double,double> >::const_iterator i, i2;
  for (i=_latlon.begin(), i2=t._latlon.begin();
       i!=_latlon.end() && i2 != t._latlon.end(); ++i, ++i2)
  {
    if (i->first != i2->first || i->second != i2->second)
    {
      LOG(ERROR) << "Diffs[" << i->first << "," << i->second.first
		 << "," << i->second.second << "]  "
		 << "[" << i2->first << "," << i2->second.first
		 << "," << i2->second.second << "]";
    }
  }
}

//----------------------------------------------------------------
void TileLatLon::add(int index, double lat, double lon)
{
  _latlon[index] = std::pair<double,double>(lat, lon);
}

//----------------------------------------------------------------
std::string TileLatLon::getXml(void) const
{
  std::string v = "";
  
  std::map<int, std::pair<double,double> >::const_iterator i;
  for (i=_latlon.begin(); i != _latlon.end(); ++i)
  {
    int index = i->first;
    pair<double,double> latlon = i->second;

    std::string vi = TaXml::writeInt("TileIndex", 1, index);
    vi += TaXml::writeDouble("Lat", 1, latlon.first);
    vi += TaXml::writeDouble("Lon", 1, latlon.second);
    v += TaXml::writeString("LatLon", 0, vi);
  }
  return TaXml::writeString(_tag, 0, v);
}

//-------------------------------------------------------------------------
std::string TileLatLon::debugString(int tileIndex) const
{
  string ret = "           ";
  std::map<int, std::pair<double,double> >::const_iterator i;
  for (i=_latlon.begin(); i!=_latlon.end(); ++i)
  {
    if (i->first == tileIndex)
    {
      char buf[100];
      sprintf(buf, "%5.1lf %5.1lf", i->second.first, i->second.second);
      ret = buf;
      break;
    }
  }
  return ret;
}
