#include "FullTemplateExtent.hh"
#include "Sweep.hh"
#include "IndexMgr.hh"
#include <euclid/Grid2d.hh>
#include <euclid/Grid2dClump.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
void FullTemplateExtent::apply(const Grid2d &data, const Sweep &v, Grid2d &out)
{
  bool circular = v.isCircular();
  out.setAllMissing();

  Grid2dClump c(data);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  for (size_t i=0; i<clumps.size(); ++i)
  {
    _addToOutput(i, clumps[i], circular, out);
  }
}

//------------------------------------------------------------------
void FullTemplateExtent::_addToOutput(int clumpIndex, const clump::Region_t &clump, bool circular, Grid2d &out)
{
  clump::Region_citer_t c;
  for (c=clump.begin(); c!=clump.end(); ++c)
  {
    _addPointToOutput(clumpIndex, c->first, c->second, circular, out);
  }      
}
 
//------------------------------------------------------------------
void FullTemplateExtent::_addPointToOutput(int clumpIndex, int r, int i, bool circular, Grid2d &out)
{
  if (_doExtend)
  {
    const TemplateLookup &l = (*_t)[r];
    int ny = out.getNy();
    int nx = out.getNx();
    for (int j=0; j<l.num1(); ++j)
    {
      int rj = l.ithIndex1R(j);
      int aj = l.ithIndex1A(j);
      int ix, iy;
      if (_getIndices(i, r, rj, aj, circular, nx, ny, ix, iy))
      {
	double v;
	if (out.getValue(ix, iy, v))
	{
	  int prev = (int)v;
	  int newInd = IndexMgr::updateManagedIndex(prev, clumpIndex);
	  out.setValue(ix, iy, (double)newInd);
	}
	else
	{
	  int managedInd = IndexMgr::managedIndex(clumpIndex);
	  out.setValue(ix, iy, (double)managedInd);
	}
      }
    }
    for (int j=0; j<l.num2(); ++j)
    {
      int rj = l.ithIndex2R(j);
      int aj = l.ithIndex2A(j);
      int ix, iy;
      if (_getIndices(i, r, rj, aj, circular, nx, ny, ix, iy))
      {
	double v;
	if (out.getValue(ix, iy, v))
	{
	  int prev = (int)v;
	  int newInd = IndexMgr::updateManagedIndex(prev, clumpIndex);
	  out.setValue(ix, iy, (double)newInd);
	}
	else
	{
	  int managedInd = IndexMgr::managedIndex(clumpIndex);
	  out.setValue(ix, iy, (double)managedInd);
	}
      }
    }
  }
  else
  {
    int managedInd = IndexMgr::managedIndex(clumpIndex);
    out.setValue(r, i, (double)managedInd);
  }
}

//------------------------------------------------------------------
bool FullTemplateExtent::_getIndices(int i, int r, int rj, int aj,
				     bool circular, int nx, int ny,
				     int &ix, int &iy) const
{
  ix = rj;
  if (ix < 0 || ix >= _t->nGates())
  {
    return false;
  }
  iy = i + aj;
  while  (iy < 0)
  {
    if (circular)
    {
      iy = iy + ny;
    }
    else
    {
      return false;
    }
  }
  while (iy >= ny)
  {
    if (circular)
    {
      iy = iy - ny;
    }
    else
    {
      return false;
    }
  }

  return true;
}

