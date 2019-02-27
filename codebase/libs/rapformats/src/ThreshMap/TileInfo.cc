/**
 * @file TileInfo.cc
 */

#include <rapformats/TileInfo.hh>
#include <rapformats/TileRange.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <cstdio>
#include <cstdlib>
#include <cmath>

const std::string TileInfo::_tag = "Tiling";

//-------------------------------------------------------------------------
TileInfo::TileInfo(const std::string &xml) :
  _ok(true)
{
  string block;
  if (TaXml::readString(xml, _tag, block))
  {
    _ok = false;
    LOG(ERROR) << "No Tag in data " << _tag;
    return;
  }

  if (TaXml::readBoolean(block, "HasLatLon", _hasLatlon))
  {
    // this is ok
  }
  else
  {
    _latlon = TileLatLon(block);
    if (!_latlon.isOk())
    {
      LOG(ERROR) << "Reading XML for lat lons";
      _ok = false;
    }
  }

  if (TaXml::readInt(block, "TileNptX", _tileNptX))
  {
    LOG(ERROR) << "No tag TileNptX in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "TileNptY", _tileNptY))
  {
    LOG(ERROR) << "No tag TileNptY in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "NptOverlapX", _tileNptOverlapX))
  {
    LOG(ERROR) << "No tag NptOverlapX in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "NptOverlapY", _tileNptOverlapY))
  {
    LOG(ERROR) << "No tag NptOverlapY in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "GridNptX", _gridNptX))
  {
    LOG(ERROR) << "No tag GridNptX in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "GridNptY", _gridNptY))
  {
    LOG(ERROR) << "No tag GridNptY in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "NTiles", _nTiles))
  {
    LOG(ERROR) << "No tag NTiles in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "NTilesX", _nTilesX))
  {
    LOG(ERROR) << "No tag NTilesX in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "NTilesY", _nTilesY))
  {
    LOG(ERROR) << "No tag NTilesY in string";
    _ok = false;
  }
  if (TaXml::readBoolean(block, "MotherTileIsSubset", _motherSubset))
  {
    LOG(ERROR) << "No tag MotherTileIsSubset in string";
    _ok = false;
  }

  if (TaXml::readInt(block, "MotherMinX", _motherMinX))
  {
    LOG(ERROR) << "No tag MotherMinX in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "MotherMaxX", _motherMaxX))
  {
    LOG(ERROR) << "No tag MotherMaxX in string";
    _ok = false;
  }

  if (TaXml::readInt(block, "MotherMinY", _motherMinY))
  {
    LOG(ERROR) << "No tag MotherMinY in string";
    _ok = false;
  }
  if (TaXml::readInt(block, "MotherMaxY", _motherMaxY))
  {
    LOG(ERROR) << "No tag MotherMaxY in string";
    _ok = false;
  }
}

//-------------------------------------------------------------------------
bool TileInfo::equalExceptLatlons(const TileInfo &t) const
{
  return (_tileNptX == t._tileNptX &&
	  _tileNptY == t._tileNptY &&
	  _tileNptOverlapX == t._tileNptOverlapX &&
	  _tileNptOverlapY == t._tileNptOverlapY &&
	  _gridNptX == t._gridNptX &&
	  _gridNptY == t._gridNptY &&
	  _nTiles == t._nTiles &&
	  _nTilesX == t._nTilesX &&
	  _nTilesY == t._nTilesY &&
	  _motherSubset == t._motherSubset &&
	  _motherMinX == t._motherMinX &&
	  _motherMaxX == t._motherMaxX &&
	  _motherMinY == t._motherMinY &&
	  _motherMaxY == t._motherMaxY);
}

//-------------------------------------------------------------------------
bool TileInfo::operator==(const TileInfo &t) const
{
  if (!equalExceptLatlons(t))
  {
    return false;
  }

  if (_hasLatlon == t._hasLatlon)
  {
    if (_hasLatlon)
    {
      return _latlon == t._latlon;
    }
    else
    {
      return true;
    }
  }
  else
  {
    return false;
  }      
}

//-------------------------------------------------------------------------
void TileInfo::printDiffs(const TileInfo &t) const
{
  if (_hasLatlon != t._hasLatlon)
  {
    LOG(ERROR) << "_hasLatLon local:" << _hasLatlon
	       << "input:" << t._hasLatlon;
  }
  else
  {
    if (_hasLatlon)
    {
      if (!(_latlon == t._latlon))
      {
	LOG(ERROR) << "latlons differ";
	_latlon.printDiffs(t._latlon);
      }
    }
  }
  if (_tileNptX != t._tileNptX)
  {
    LOG(ERROR) << "_tileNptX != t._tileNptX";
  }
  if (_tileNptY != t._tileNptY)
  {
    LOG(ERROR) << "_tileNptY != t._tileNptY";
  }
  if (_tileNptOverlapX != t._tileNptOverlapX)
  {
    LOG(ERROR) << "_tileNptOverlapX != t._tileNptOverlapX";
  }
  if (_tileNptOverlapY != t._tileNptOverlapY)
  {
    LOG(ERROR) << "_tileNptOverlapY != t._tileNptOverlapY";
  }
  if (_gridNptX != t._gridNptX)
  {
    LOG(ERROR) << "_gridNptX != t._gridNptX";
  }
  if (_gridNptY != t._gridNptY)
  {
    LOG(ERROR) << "_gridNptY != t._gridNptY";
  }
  if (_nTiles != t._nTiles)
  {
    LOG(ERROR) << "_nTiles != t._nTiles";
  }
  if (_nTilesX != t._nTilesX)
  {
    LOG(ERROR) << "_nTilesX != t._nTilesX";
  }
  if (_nTilesY != t._nTilesY)
  {
    LOG(ERROR) << "_nTilesY != t._nTilesY";
  }
  if (_motherSubset != t._motherSubset)
  {
    LOG(ERROR) << "_motherSubset != t._motherSubset";
  }
  if (_motherMinX != t._motherMinX)
  {
    LOG(ERROR) << "_motherMinX != t._motherMinX";
  }
  if (_motherMaxX != t._motherMaxX)
  {
    LOG(ERROR) << "_motherMaxX != t._motherMaxX";
  }
  if (_motherMinY != t._motherMinY)
  {
    LOG(ERROR) << "_motherMinY != t._motherMinY";
  }
  if (_motherMaxY != t._motherMaxY)
  {
    LOG(ERROR) << "_motherMaxY != t._motherMaxY";
  }
}


//-------------------------------------------------------------------------
void TileInfo::setLatLons(const TileLatLon &latlon)
{
  _latlon = latlon;
  _hasLatlon = (int)(_latlon.size()) == _nTiles;
  if (!_hasLatlon)
  {
    LOG(ERROR) << "Wrong number of tiles expect " << _nTiles << " got "
	       << _latlon.size();
  }
}

//-------------------------------------------------------------------------
void TileInfo::addLatlons(const TileInfo &t)
{
  if (_hasLatlon)
  {
    return;
  }
  _latlon = t._latlon;
  _hasLatlon = t._hasLatlon;
  if (!_hasLatlon)
  {
    LOG(WARNING) << "No latlons to add to state";
  }
}


//-------------------------------------------------------------------------
std::string TileInfo::toXml(void) const
{
  string xml = TaXml::writeStartTag(_tag, 0);
  xml += TaXml::writeBoolean("HasLatLon", 1, _hasLatlon);
  if (_hasLatlon)
  {
    xml += _latlon.getXml();
  }
  xml += TaXml::writeInt("TileNptX", 1, _tileNptX);
  xml += TaXml::writeInt("TileNptY", 1, _tileNptY);
  xml += TaXml::writeInt("NptOverlapX", 1, _tileNptOverlapX);
  xml += TaXml::writeInt("NptOverlapY", 1, _tileNptOverlapY);
  xml += TaXml::writeInt("GridNptX", 1, _gridNptX);
  xml += TaXml::writeInt("GridNptY", 1, _gridNptY);
  xml += TaXml::writeInt("NTiles", 1, _nTiles);
  xml += TaXml::writeInt("NTilesX", 1, _nTilesX);
  xml += TaXml::writeInt("NTilesY", 1, _nTilesY);
  xml += TaXml::writeBoolean("MotherTileIsSubset", 1, _motherSubset);
  xml += TaXml::writeInt("MotherMinX", 1, _motherMinX);
  xml += TaXml::writeInt("MotherMaxX", 1, _motherMaxX);
  xml += TaXml::writeInt("MotherMinY", 1, _motherMinY);
  xml += TaXml::writeInt("MotherMaxY", 1, _motherMaxY);
  xml += TaXml::writeEndTag(_tag, 0);
  return xml;
}

//-------------------------------------------------------------------------
TileRange TileInfo::range(int tileIndex) const
{
  if (tileIndex < 0 || tileIndex >= _nTiles)
  {
    LOG(ERROR) << "Tile Index out of Range " << tileIndex  << " " 
	       << _nTiles;
    return TileRange();
  }

  if (tileIndex == 0)
  {
    // always the mother tile
    if (_motherSubset)
    {
      return TileRange(_motherMinX, _motherMinY, _motherMaxX - _motherMinX + 1,
		       _motherMaxY - _motherMinY + 1, tileIndex);
    }
    else
    {
      return TileRange(0, 0, _gridNptX, _gridNptY, tileIndex);
    }
  }
  else
  {
    // get the tile index for x and y first
    int nincx = _tileNptX - _tileNptOverlapX;
    int nincy = _tileNptY - _tileNptOverlapY;
    int iy = (tileIndex-1)/_nTilesX;
    int ix = (tileIndex-1) - iy*_nTilesX;
    return TileRange(ix*nincx, iy*nincy, _tileNptX, _tileNptY, tileIndex);
  }
}

//-------------------------------------------------------------------------
TileRange TileInfo::rangeNoOverlap(int tileIndex) const
{
  if (tileIndex < 0 || tileIndex >= _nTiles)
  {
    LOG(ERROR) << "Tile Index out of Range " << tileIndex  << " " 
	       << _nTiles;
    return TileRange();
  }
  if (tileIndex == 0)
  {
    // always the mother tile
    if (_motherSubset)
    {
      return TileRange(_motherMinX, _motherMinY, _motherMaxX - _motherMinX + 1,
		       _motherMaxY - _motherMinY + 1, tileIndex);
    }
    else
    {
      return TileRange(0, 0, _gridNptX, _gridNptY, tileIndex);
    }
  }
  else
  {
    // get the tile index for x and y first
    int nincx = _tileNptX - _tileNptOverlapX;
    int nincy = _tileNptY - _tileNptOverlapY;
    int iy = (tileIndex-1)/_nTilesX;
    int ix = (tileIndex-1) - iy*_nTilesX;
    return TileRange(ix*nincx, iy*nincy, nincx, nincy, tileIndex);
  }
}

//-------------------------------------------------------------------------
int TileInfo::tileFromTileIndex(int tileIndexX, int tileIndexY) const
{
  if (_nTiles == 1)
  {
    // mother only
    return 0;
  }
  else
  {
    return tileIndexY*_nTilesX + tileIndexX + 1; // add one for mother tile
  }
}

//-------------------------------------------------------------------------
bool TileInfo::outOfBoundsY(int tIndex, int &belowTile) const
{
  belowTile = tIndex;
  TileRange tileRange = range(belowTile);
  if (tileRange.inBoundsY(_gridNptY))
  {
    return false;
  }

  TileRange tTest(tileRange);
  // int original_y0 = y0;
  while (!tTest.inBoundsY(_gridNptY))  //y0 + ny >= _gridNptY)
  {
    belowTile = tileBelow(tTest.getTileIndex());
    if (belowTile < 0)
    {
      LOG(ERROR) << "Logic problem";
      return true;
    }
    tTest = range(belowTile);
    if (tTest.isAbove(tileRange))
    {
      LOG(ERROR) << "Expect y to decrease, it increased, LOGIC error";
      belowTile = -1;
      return true;
    }
  }
  if (belowTile < 0 || belowTile >= _nTiles)
  {
    LOG(ERROR) << "Logic error, index went out of bounds";
  }
  return true;
}

//-------------------------------------------------------------------------
void TileInfo::print(bool verbose) const
{
  printf("Tiling: NptXY:(%d,%d) OverlapNpt:(%d,%d) NumTiles:(%d,%d) [%d]\n",
	 _tileNptX, _tileNptY, _tileNptOverlapX, _tileNptOverlapY,
	 _nTilesX, _nTilesY, _nTiles);
  printf("        Grid:(%d,%d)\n", _gridNptX, _gridNptY);
  if (_motherSubset)
  {
    printf("        Mother:[%d,%d] to [%d,%d]\n", _motherMinX, _motherMinY,
	   _motherMaxX, _motherMaxY);
  }
  else
  {
    printf("        Mother:Full grid\n");
  }
}

//------------------------------------------------------------------
bool TileInfo::constructTiledGrid(const std::string &fieldName,
				  const std::vector<double> &tileThresh,
				  Grid2d &grid) const
{
  if (static_cast<int>(tileThresh.size()) != _nTiles)
  {
    LOG(ERROR) << "Tile size mismatch";
    return false;
  }

  // make 2 grids, one with sums, one with counts
  GridAlgs counts("counts", _gridNptX, _gridNptY, -1.0);
  counts.setAllToValue(0.0);

  // maybe access the missing data value somehow
  GridAlgs sums(fieldName, _gridNptX, _gridNptY, -99.99);
  sums.setAllToValue(0.0);
  
  for (int i=0; i<_nTiles; ++i)
  {
    if (isMotherTile(i))
    {
      continue;
    }
    double threshold = tileThresh[i];

    TileRange r = range(i);
    if (!r.isOk())
    {
      LOG(ERROR) << "Ranges not computed";
      return false;
    }
    LOG(DEBUG_VERBOSE) << " Tile i x0,y0 = " << r.getX0() << " " << r.getY0();
    for (int y=r.getY0(); r.inRangeY(y); ++y)
    {
      // allow for wraparound in Y as well, we just assume the
      // thresh was set correctly
      if (y < sums.getNy())
      {
	for (int x=r.getX0(); r.inRangeX(x); ++x)
	{
	  if (x < sums.getNx())
	  {
	    // allow for wraparound, which we allow in x
	    sums.increment(x, y, threshold);
	    counts.increment(x, y, 1.0);
	  }
	}
      }
    }
  }

  sums.divide(counts);
  grid = sums;
  return true;
}

//------------------------------------------------------------------
bool
TileInfo::constructWeightedTiledGrid(const std::string &fieldName,
				     const std::vector<double> &tileThresh,
				     double centerWeight, double edgeWeight,
				     int nptSmooth, Grid2d &grid) const
{
  // make 2 grids, one with weighted sums, one with weights
  GridAlgs counts("weights", _gridNptX, _gridNptY, -1.0);
  counts.setAllToValue(0.0);

  // maybe access the missing data value somehow
  GridAlgs sums(fieldName, _gridNptX, _gridNptY, -99.99);
  sums.setAllToValue(0.0);
  
  // each tile will have the same weight distribution, so make a weights
  // grid as well
  Grid2d weights("W", _tileNptX, _tileNptY, -1.0);
  double yCenter = (double)_tileNptY/2.0;
  double xCenter = (double)_tileNptX/2.0;
  for (int y=0; y<_tileNptY; ++y)
  {
    double dy = (yCenter - fabs(yCenter - double(y)))/yCenter;
    for (int x=0; x<_tileNptX; ++x)
    {
      double dx = (xCenter - fabs(double(_tileNptX)/2.0 - double(x)))/xCenter;
      // dx and dy should be 0 at edges and 1 in center
      // which is smaller?  That is our distance
      double delta;
      if (dx < dy)
      {
	delta = dx;
      }
      else
      {
	delta = dy;
      }
      if (delta < 0)
      {
	delta = 0;
      }
      if (delta > 1)
      {
	delta = 1;
      }
      double weight = edgeWeight + (centerWeight-edgeWeight)*delta;
      weights.setValue(x, y, weight);
    }
  }

  for (int i=0; i<_nTiles; ++i)
  {
    if (isMotherTile(i))
    {
      continue;
    }
    double threshold = tileThresh[i];

    TileRange r = range(i);
    if (!r.isOk())
    {
      LOG(ERROR) << "Ranges not computed";
      return false;
    }
    LOG(DEBUG_VERBOSE) << " Tile i x0,y0 = " << r.getX0() << " " << r.getY0();
    for (int y=r.getY0(); r.inRangeY(y); ++y)
    {
      int dy = y - r.getY0();

      // allow for wraparound in Y as well, we just assume the
      // thresh was set correctly
      if (y < sums.getNy())
      {
	for (int x=r.getX0(); r.inRangeX(x); ++x)
	{
	  if (x < sums.getNx())
	  {
	    int dx = x - r.getX0();
	    double w = weights.getValue(dx, dy);
	    // allow for wraparound, which we allow in x
	    sums.increment(x, y, w*threshold);
	    counts.increment(x, y, w);
	  }
	}
      }
    }
  }

  sums.divide(counts);
  if (nptSmooth > 0)
  {
    sums.smooth(nptSmooth, nptSmooth);
  }
  grid = sums;
  return true;
}


//------------------------------------------------------------------
bool TileInfo::constructTiledGridNoOverlap(const std::string &fieldName,
					   const std::vector<double> &values,
					   Grid2d &grid) const
{
  if (static_cast<int>(values.size()) != _nTiles)
  {
    LOG(ERROR) << "Tile size mismatch";
    return false;
  }

  grid = Grid2d(fieldName, _gridNptX, _gridNptY, -1.0);

  for (int i=0; i<_nTiles; ++i)
  {
    if (isMotherTile(i))
    {
      continue;
    }
    double v = values[i];

    TileRange r = rangeNoOverlap(i);
    if (!r.isOk())
    {
      LOG(ERROR) << "Ranges not computed";
      return false;
    }
    LOG(DEBUG_VERBOSE) << " Tile i x0,y0 = " << r.getX0() << " " << r.getY0();
    for (int y=r.getY0(); r.inRangeY(y); ++y)
    {
      if (y < grid.getNy())
      {
	for (int x=r.getX0(); r.inRangeX(x); ++x)
	{
	  if (x < grid.getNx())
	  {
	    // allow for wraparound, which we allow in x
	    grid.setValue(x, y, v);
	  }
	}
      }
    }
  }
  return true;
}

//-------------------------------------------------------------------------
int TileInfo::tileBelow(int index) const
{
  if (index < 0 || index >= _nTiles)
  {
    LOG(ERROR) << "Input index out of range " << index;
    return -1;
  }
  if (isMotherTile(index))
  {
    LOG(ERROR) << "Nothing below the mother tile";
    return -1;
  }

  int tx = _tileXpt(index);
  int ty = _tileYpt(index);
  if (ty == 0)
  {
    LOG(WARNING) << "No tile below input tile " << index;
    return -1;
  }
  return tileFromTileIndex(tx, ty-1);
}

//-------------------------------------------------------------------------
std::string TileInfo::latlonDebugString(int tileIndex) const
{
  if (_hasLatlon)
  {
    return _latlon.debugString(tileIndex);
  }
  else
  {
    string ret = "           ";
    return ret;
  }
}

//-------------------------------------------------------------------------
void TileInfo::_deriveNumTiles(void)
{
  // number of points to increment as you move through X
  int nincx = _tileNptX - _tileNptOverlapX;

  // number of points to increment as you move through Y
  int nincy = _tileNptY - _tileNptOverlapY;

  _nTilesX = 0;
  for (int i=0; i<_gridNptX; i+= nincx)
  {
    ++_nTilesX;
  }
  _nTilesY = 0;
  for (int i=0; i<_gridNptY; i+= nincy)
  {
    ++_nTilesY;
  }

  // check last one to see if not full
  int last = (_nTilesX-1)*nincx;
  if (last + _tileNptX > _gridNptX)
  {
    LOG(DEBUG) << "Wraparound...Last tile X is short, want " << _tileNptX 
	       << " got " << _gridNptX - last + 1;
  }
  last = (_nTilesY-1)*nincy;
  if (last + _tileNptY > _gridNptY)
  {
    LOG(DEBUG) << "Fill with previous..Last tile Y is short, want " << _tileNptY
	       << " got " << _gridNptY - last + 1;
  }

  _nTiles = _nTilesX*_nTilesY + 1; // add one for the mother tile
}

