/**
 * @file MultiThreshFcstBiasMapping.cc
 */

//------------------------------------------------------------------
#include <rapformats/MultiThreshFcstBiasMapping.hh>
#include <rapformats/MultiThreshItem.hh>
#include <rapformats/TileInfo.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
#include <algorithm>

const std::string MultiThreshFcstBiasMapping::_tag = "Forecast";

//------------------------------------------------------------------
MultiThreshFcstBiasMapping::
MultiThreshFcstBiasMapping(const std::string &xml,
			   const std::vector<std::string> &fields,
			   const std::vector<int> &leadSeconds,
			   const TileInfo &tiling) :
  _ok(true)
{
  // read in hour minute second
  if (TaXml::readInt(xml, "hour", _genHour))
  {
    LOG(ERROR) << "Reading tag hour";
    _ok = false;
  }
  if (TaXml::readInt(xml, "min", _genMinute))
  {
    LOG(ERROR) << "Reading tag min";
    _ok = false;
  }
  if (TaXml::readInt(xml, "sec", _genSecond))
  {
    LOG(ERROR) << "Reading tag sec";
    _ok = false;
  }
  
  // read in array for TiledMultiFcst array, and compare to lead length
  // (should be same)
  vector<string> vstring;
  if (TaXml::readStringArray(xml, TiledMultiThresh::_tag, vstring))
  {
    LOG(ERROR) << "Reading tag as array " << TiledMultiThresh::_tag;
    _ok = false;
    return;
  }
  if (vstring.size() != leadSeconds.size())
  {
    LOG(ERROR) << "Inconsistent lead second sizes "
	       << "input:" << leadSeconds.size()
	       << " xml:" << vstring.size();
    _ok = false;
    return;
  }

  // for every element in lead time array
  // parse it as a TiledMultiThresh object
  for (size_t i=0; i<vstring.size(); ++i)
  {
    TiledMultiThresh m(vstring[i], fields, tiling);
    _map[leadSeconds[i]] = m;
    if (!m.ok())
    {
      _ok = false;
    }
  }
}

//------------------------------------------------------------------
MultiThreshFcstBiasMapping::
MultiThreshFcstBiasMapping(int hour, int minute, int second,
			   const std::vector<int> &leadSeconds,
			   const TileInfo &tiling,
			   const std::vector<FieldThresh2> &fieldThresh) :

  _ok(true),
  _genHour(hour), _genMinute(minute), _genSecond(second)
{
  for (size_t ilt=0; ilt<leadSeconds.size(); ++ilt)
  {
    // the thresholds are all coldstart i.e. same for all tiles
    _map[leadSeconds[ilt]] = TiledMultiThresh(tiling.numTiles(),
					      fieldThresh);
  }
}

//------------------------------------------------------------------
MultiThreshFcstBiasMapping::~MultiThreshFcstBiasMapping()
{
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::hmsMatch(const time_t &genTime) const
{
  DateTime dt(genTime);
  int h = dt.getHour();
  int m = dt.getMin();
  int s = dt.getSec();
  return hmsMatch(h, m, s);
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::hmsMatch(int h, int m, int s) const
{
  return h == _genHour && m == _genMinute && s == _genSecond;
}

//------------------------------------------------------------------
std::string MultiThreshFcstBiasMapping::toXml(int indent) const
{
  string s = TaXml::writeStartTag(_tag, indent);
  s += TaXml::writeInt("hour", indent+1, _genHour, "%02d");
  s += TaXml::writeInt("min", indent+1, _genMinute, "%02d");
  s += TaXml::writeInt("sec", indent+1, _genSecond, "%02d");

  std::map<int, TiledMultiThresh>::const_iterator i;
  for (i = _map.begin(); i!= _map.end(); ++i)
  {
    s += i->second.toXml(indent+1);
  }
  s += TaXml::writeEndTag(_tag, indent);
  return s;
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::
checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
	       const std::vector<FieldThresh2> &coldstartThresh)
{
  bool ret = true;
  std::map<int, TiledMultiThresh>::iterator i;
  for (i = _map.begin(); i!= _map.end(); ++i)
  {
    if (!i->second.checkColdstart(t, maxSecondsBeforeColdstart,
				  coldstartThresh))
    {
      ret = false;
    }
  }
  return ret;
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::update(const MultiThreshItem &item)
{
  return _map[item._leadTime].update(item);
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::
filterFields(const std::vector<std::string> &fieldNames)
{
  std::map<int, TiledMultiThresh>::iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (!i->second.filterFields(fieldNames))
    {
      LOG(ERROR) << "ERROR filtering for lead time " << i->first;
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::
replaceValues(const MultiThreshFcstBiasMapping &filtMap,
	      const std::vector<std::string> &filterFields)
{
  if (hmsMatch(filtMap._genHour, filtMap._genMinute, filtMap._genSecond))
  {
    if (_map.size() != filtMap._map.size())
    {
      LOG(ERROR) << "Inconsistent number of lead times";
      return false;
    }
    std::map<int, TiledMultiThresh>::iterator loc_i;
    std::map<int, TiledMultiThresh>::const_iterator inp_i;
    bool ret = true;
    for (loc_i=_map.begin(), inp_i= filtMap._map.begin();
	 loc_i != _map.end() && inp_i != _map.end(); ++loc_i, ++inp_i)
    {
      if (loc_i->first != inp_i->first)
      {
	LOG(ERROR) << "Inconsistent lead time map key";
	ret = false;
      }
      else
      {
	if (!loc_i->second.replaceValues(inp_i->second, filterFields))
	{
	  ret = false;
	}
      }
    }
    return ret;
  }
  else
  {
    LOG(ERROR) << "Gen time mismatch";
    return false;
  }
}

//------------------------------------------------------------------
void MultiThreshFcstBiasMapping::
setColdstart(int leadTime, int numTiles,
	     const std::vector<FieldThresh2> &thresh)
{
  _map[leadTime] = TiledMultiThresh(numTiles, thresh);
}

//------------------------------------------------------------------
void MultiThreshFcstBiasMapping::print(const TileInfo &info, bool verbose) const
{
  printf("     Gt_hms:%02d:%02d:%02d\n", _genHour, _genMinute, _genSecond);
  std::map<int, TiledMultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    i->second.print(i->first, info, verbose);
  }
}

//------------------------------------------------------------------
void MultiThreshFcstBiasMapping::print(const std::vector<int> &gtHour,
				       const std::vector<int> &ltSec,
				       const std::vector<int> &tiles,
				       const TileInfo &tileInfo,
				       bool verbose) const
{
  if (!gtHour.empty())
  {
    if (find(gtHour.begin(), gtHour.end(), _genHour) == gtHour.end())
    {
      return;
    }
  }

  printf("     Gt_hms:%02d:%02d:%02d\n", _genHour, _genMinute, _genSecond);
  std::map<int, TiledMultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (!ltSec.empty())
    {
      if (find(ltSec.begin(), ltSec.end(), i->first) != ltSec.end())
      {
	i->second.print(i->first, tiles, tileInfo, verbose);
      }
    }
    else
    {
      i->second.print(i->first, tiles, tileInfo, verbose);
    }
  }
}
      
//------------------------------------------------------------------
bool MultiThreshFcstBiasMapping::get(const time_t &genTime, int leadTime,
				     int tileInd, MultiThreshItem &item) const
{
  DateTime dt(genTime);
  if (dt.getHour() == _genHour && dt.getMin() == _genMinute &&
      dt.getSec() == _genSecond)
  {
    const TiledMultiThresh *mt = _mapFromLeadTime(leadTime);
    if (mt == NULL)
    {
      LOG(ERROR) << "Lead time not in state " << leadTime;
      return false;
    }    
    const MultiThresh *m = mt->mapFromTileIndex(tileInd);
    if (m == NULL)
    {
      LOG(ERROR) << "Tile index not in state " << tileInd;
      return false;
    }

    item = MultiThreshItem(*m, _genHour, _genMinute, _genSecond, leadTime,
			   tileInd);
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool
MultiThreshFcstBiasMapping::getTiledGrids(const time_t &genTime, int leadTime,
					  const TileInfo &tiling,
					  double centerWeight,
					  double edgeWeight,
					  int nptSmooth,
					  std::vector<Grid2d> &item) const
{
  DateTime dt(genTime);
  if (dt.getHour() == _genHour && dt.getMin() == _genMinute &&
      dt.getSec() == _genSecond)
  {
    const TiledMultiThresh *mt = _mapFromLeadTime(leadTime);
    if (mt == NULL)
    {
      LOG(ERROR) << "Lead time not in state " << leadTime;
      return false;
    }    
    return mt->getTiledGrids(tiling, centerWeight, edgeWeight, nptSmooth, item);
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool
MultiThreshFcstBiasMapping::
getDebugTiledGrids(const time_t &genTime, int leadTime,
		   const std::string &field,
		   const TileInfo &tiling, double centerWeight,
		   double edgeWeight, int nptSmooth,
		   std::vector<Grid2d> &item) const
{
  DateTime dt(genTime);
  if (dt.getHour() == _genHour && dt.getMin() == _genMinute &&
      dt.getSec() == _genSecond)
  {
    const TiledMultiThresh *mt = _mapFromLeadTime(leadTime);
    if (mt == NULL)
    {
      LOG(ERROR) << "Lead time not in state " << leadTime;
      return false;
    }    
    return mt->getDebugTiledGrids(tiling, field, centerWeight, edgeWeight,
				  nptSmooth, item);
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool
MultiThreshFcstBiasMapping::constructTiledGrid(const std::string &fieldName,
					       int leadTime,
					       const TileInfo &tiling,
					       double centerWeight,
					       double edgeWeight, int nptSmooth,
					       Grid2d &grid) const
{
  const TiledMultiThresh *mt = _mapFromLeadTime(leadTime);
  if (mt == NULL)
  {
    LOG(ERROR) << "Lead time not in state " << leadTime;
    return false;
  }
  return mt->constructTiledGrid(fieldName, tiling, centerWeight, edgeWeight,
				nptSmooth, grid);
}

//------------------------------------------------------------------
const TiledMultiThresh *
MultiThreshFcstBiasMapping::_mapFromLeadTime(int leadTime) const
{
  std::map<int, TiledMultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (i->first == leadTime)
    {
      return &(i->second);
    }
  }
  return NULL;
}
