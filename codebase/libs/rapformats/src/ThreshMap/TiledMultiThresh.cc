/**
 * @file TiledMultiThresh.cc
 */

//------------------------------------------------------------------
#include <rapformats/TiledMultiThresh.hh>
#include <rapformats/MultiThreshItem.hh>
#include <rapformats/TileInfo.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>

const std::string TiledMultiThresh::_tag = "Lt";

//------------------------------------------------------------------
TiledMultiThresh::TiledMultiThresh(void) :
  _ok(false)
{
}

//------------------------------------------------------------------
TiledMultiThresh::
TiledMultiThresh(const std::string &xml,
		 const std::vector<std::string> &fields,
		 const TileInfo &tiling) :
  _ok(true)
{
  // read in tiles array, and compare to input.
  vector<string> vstring;
  if (TaXml::readStringArray(xml, MultiThresh::_tag, vstring))
  {
    LOG(ERROR) << "Reading tag as array " << MultiThresh::_tag;
    _ok = false;
    return;
  }

  if (static_cast<int>(vstring.size()) != tiling.numTiles())
  {
    LOG(ERROR) << "Inconsistent tiling "
	       << "input:" << tiling.numTiles() 
	       << " xml:" << vstring.size();
    _ok = false;
    return;
  }

  // for every element,
  // parse it as a MultiThresh object.
  for (size_t i=0; i<vstring.size(); ++i)
  {
    int tileIndex;
    MultiThresh m(vstring[i], fields, tileIndex);
    _map[tileIndex] = m;
    if (!m.ok())
    {
      _ok = false;
    }
  }
}

//------------------------------------------------------------------
TiledMultiThresh::TiledMultiThresh(int numTiles,
				   const std::vector<FieldThresh2> &fieldThresh)
{
  for (int i=0; i<numTiles; ++i)
  {
    bool isMother = TileInfo::isMotherTile(i);
    _map[i] = MultiThresh(fieldThresh, isMother);
  }
}

//------------------------------------------------------------------
TiledMultiThresh::~TiledMultiThresh(void)
{
}


//------------------------------------------------------------------
const MultiThresh *TiledMultiThresh::mapFromTileIndex(int tileInd) const
{
  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (i->first == tileInd)
    {
      return &(i->second);
    }
  }
  return NULL;
}

//------------------------------------------------------------------
std::string TiledMultiThresh::toXml(int indent) const
{
  string s = TaXml::writeStartTag(_tag, indent);
  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    s += i->second.toXml(i->first);
  }
  s += TaXml::writeEndTag(_tag, indent);
  return s;
}

//------------------------------------------------------------------
bool TiledMultiThresh::
checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
	       const std::vector<FieldThresh2> &coldstartThresh)
{
  bool ret = true;
  std::map<int, MultiThresh>::iterator i;
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
bool TiledMultiThresh::update(const MultiThreshItem &item)
{
  MultiThresh *m = _mapFromTile(item._tileIndex);
  if (m == NULL)
  {
    LOG(ERROR) << "Tile index out of range " << item._tileIndex;
    return false;
  }
  return m->update(item._multiThresh);
}

//------------------------------------------------------------------
bool TiledMultiThresh::filterFields(const std::vector<std::string> &fieldNames)
{
  std::map<int, MultiThresh>::iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (!i->second.filterFields(fieldNames))
    {
      LOG(ERROR) << "Filtering fields for tile index " << i->first;
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------
bool
TiledMultiThresh::replaceValues(const TiledMultiThresh &filtMap,
				const std::vector<std::string> &filterFields)
{
  if (_map.size() != filtMap._map.size())
  {
    LOG(ERROR) << "Inconsistent number of tiles";
    return false;
  }
  const MultiThresh *inp;
  MultiThresh *loc;

  // assumes tiles are indexed 0,1,...
  bool ret = true;
  for (int i=0; i<static_cast<int>(_map.size()); ++i)
  {
    inp = filtMap._constMapFromTile(i);
    loc = _mapFromTile(i);
    if (inp == NULL || loc == NULL)
    {
      LOG(ERROR) << "No tile at index " << i;
      ret = false;
    }
    else
    {
      loc->replaceValues(*inp, filterFields);
    }
  }
  return ret;
}

//------------------------------------------------------------------
void TiledMultiThresh::print(int leadTime, const TileInfo &info,
			     bool verbose) const
{
  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    i->second.print(leadTime, i->first, info, verbose);
  }
}

//------------------------------------------------------------------
void TiledMultiThresh::print(int leadTime, const std::vector<int> &tiles,
			     const TileInfo &info, bool verbose) const
{
  if (tiles.empty())
  {
    print(leadTime, info, verbose);
  }

  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (find(tiles.begin(), tiles.end(), i->first) != tiles.end())
    {
      i->second.print(leadTime, i->first, info, verbose);
    }
  }
}

//------------------------------------------------------------------
bool
TiledMultiThresh::getDebugTiledGrids(const TileInfo &tiling,
				     const std::string &field,
				     double centerWeight, double edgeWeight,
				     int nptSmooth,
				     std::vector<Grid2d> &item) const
{
  const MultiThresh *mt = _constMapFromTile(0);
  if (mt == NULL)
  {
    LOG(ERROR) << "No content, no grid";
    return false;
  }
  int fieldIndex = mt->getThresholdIndex(field);
  if (fieldIndex < 0)
  {
    LOG(ERROR) << " Field not found " << field;
    return false;
  }

  // now construct non-overlapping return grids
  vector<double> v = _tileThresh(fieldIndex, tiling.numTiles());
  if (v.empty())
  {
    return false;
  }
  Grid2d grid;
  if (!tiling.constructTiledGridNoOverlap("Thresh", v, grid))
  {
    return false;
  }
  item.push_back(grid);

  if (!tiling.constructWeightedTiledGrid("WtThresh", v, centerWeight, 
					 edgeWeight, nptSmooth, grid))
  {
    return false;
  }
  item.push_back(grid);

  v = _tileBias(tiling.numTiles());
  if (!tiling.constructTiledGridNoOverlap("Bias", v, grid))
  {
    return false;
  }
  item.push_back(grid);
  
  v = _tileObs(tiling.numTiles());
  if (!tiling.constructTiledGridNoOverlap("Obs", v, grid))
  {
    return false;
  }
  item.push_back(grid);

  v = _tileFcst(tiling.numTiles());
  if (!tiling.constructTiledGridNoOverlap("Fcst", v, grid))
  {
    return false;
  }
  item.push_back(grid);
  
  v = _tileIsMother(tiling.numTiles());
  if (!tiling.constructTiledGridNoOverlap("Mother", v, grid))
  {
    return false;
  }
  item.push_back(grid);
  return true;
}

//------------------------------------------------------------------
bool TiledMultiThresh::getTiledGrids(const TileInfo &tiling,
				     double centerWeight, double edgeWeight,
				     int nptSmooth,
				     std::vector<Grid2d> &item) const
{
  item.clear();

  // assume naming same for all tiles
  const MultiThresh *mt = _constMapFromTile(0);
  if (mt == NULL)
  {
    LOG(ERROR) << "No content, no grid";
    return false;
  }

  bool status = true;
  for (int i=0; i<mt->num(); ++i)
  {
    string name;
    if (mt->getIthName(i, name))
    {
      Grid2d grid;
      if (constructTiledGrid(name, tiling, centerWeight, edgeWeight, 
			     nptSmooth, grid))
      {
	item.push_back(grid);
      }
      else
      {
	LOG(ERROR) << "Could not construct grid";
	status = false;
      }
    }
    else
    {
      LOG(ERROR) << "Name indexing error";
      status = false;
    }
  }
  return status;
}



//------------------------------------------------------------------
bool
TiledMultiThresh::constructTiledGrid(const std::string &fieldName,
				     const TileInfo &tiling,
				     double centerWeight, double edgeWeight,
				     int nptSmooth, Grid2d &grid) const
{
  // assume naming same for all tiles
  const MultiThresh *mt = _constMapFromTile(0);
  if (mt == NULL)
  {
    LOG(ERROR) << "No content, no grid";
    return false;
  }
  int fieldIndex = mt->getThresholdIndex(fieldName);
  if (fieldIndex < 0)
  {
    LOG(ERROR) << " Field not found " << fieldName;
    return false;
  }

  vector<double> tileThresh = _tileThresh(fieldIndex, tiling.numTiles());
  if (tileThresh.empty())
  {
    return false;
  }

  return tiling.constructWeightedTiledGrid(fieldName, tileThresh, 
					   centerWeight, edgeWeight,
					   nptSmooth, grid);
}

//------------------------------------------------------------------
MultiThresh *TiledMultiThresh::_mapFromTile(int tileIndex)
{
  std::map<int, MultiThresh>::iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (i->first == tileIndex)
    {
      return &(i->second);
    }
  }
  return NULL;
}
      
//------------------------------------------------------------------
const MultiThresh *TiledMultiThresh::_constMapFromTile(int tileIndex) const
{
  std::map<int, MultiThresh>::const_iterator i;
  for (i=_map.begin(); i!=_map.end(); ++i)
  {
    if (i->first == tileIndex)
    {
      return &(i->second);
    }
  }
  return NULL;
}
      
//------------------------------------------------------------------
std::vector<double> TiledMultiThresh::_tileThresh(int fieldIndex,
						  int numTiles) const
{
  vector<double> ret;
  for (int tileIndex=0; tileIndex<numTiles; ++tileIndex)
  {
    const MultiThresh *mt = _constMapFromTile(tileIndex);
    if (mt == NULL)
    {
      LOG(ERROR) << "Tiling mismatch";
      ret.clear();
      return ret;
    }
    double threshold;
    if (!mt->getIthThreshold(fieldIndex, threshold))
    {
      LOG(ERROR) << "Tiling mismatch no field " << fieldIndex;
      ret.clear();
      return ret;
    }
    ret.push_back(threshold);
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<double> TiledMultiThresh::_tileBias(int numTiles) const
{
  vector<double> ret;
  for (int tileIndex=0; tileIndex<numTiles; ++tileIndex)
  {
    const MultiThresh *mt = _constMapFromTile(tileIndex);
    if (mt == NULL)
    {
      LOG(ERROR) << "Tiling mismatch";
      ret.clear();
      return ret;
    }
    ret.push_back(mt->getBias());
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<double> TiledMultiThresh::_tileObs(int numTiles) const
{
  vector<double> ret;
  for (int tileIndex=0; tileIndex<numTiles; ++tileIndex)
  {
    const MultiThresh *mt = _constMapFromTile(tileIndex);
    if (mt == NULL)
    {
      LOG(ERROR) << "Tiling mismatch";
      ret.clear();
      return ret;
    }
    ret.push_back(mt->getObs());
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<double> TiledMultiThresh::_tileFcst(int numTiles) const
{
  vector<double> ret;
  for (int tileIndex=0; tileIndex<numTiles; ++tileIndex)
  {
    const MultiThresh *mt = _constMapFromTile(tileIndex);
    if (mt == NULL)
    {
      LOG(ERROR) << "Tiling mismatch";
      ret.clear();
      return ret;
    }
    ret.push_back(mt->getFcst());
  }
  return ret;
}

//------------------------------------------------------------------
std::vector<double> TiledMultiThresh::_tileIsMother(int numTiles) const
{
  vector<double> ret;
  for (int tileIndex=0; tileIndex<numTiles; ++tileIndex)
  {
    const MultiThresh *mt = _constMapFromTile(tileIndex);
    if (mt == NULL)
    {
      LOG(ERROR) << "Tiling mismatch";
      ret.clear();
      return ret;
    }
    ret.push_back(mt->getIsMother(1, 0));
  }
  return ret;
}

