/**
 * @file VertData2d.cc
 */
#include <FiltAlgVirtVol/VertData2d.hh>
#include <euclid/Grid2d.hh>
#include <euclid/Grid2dClump.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <cmath>

//------------------------------------------------------------------
VertData2d::VertData2d(void) : MathUserData(), _ok(false), _vlevel(-1)
{
}

//------------------------------------------------------------------
VertData2d::VertData2d(const Grid2d &g, double vlevel) :
  MathUserData(), _ok(true), _vlevel(vlevel), _data(g)
{
}

/*----------------------------------------------------------------*/
bool VertData2d::getFloat(double &v) const
{
  return 0.0;
}

/*----------------------------------------------------------------*/
VertData2d *VertData2d::clump(double thresh, double pct) const
{
  Grid2d g(_data);
  g.setAllMissing();

  Grid2dClump c(_data);
  std::vector<clump::Region_t> clumps = c.buildRegions();
  for (size_t i=0; i<clumps.size(); ++i)
  {
    clump::Region_citer_t c;
    double count = 0;
    double good = 0;
    for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
    {
      double v;
      if (_data.getValue(c->first, c->second, v))
      {
	count ++;
	if (v >= thresh)
	{
	  ++good;
	}
      }
    }
    if (count == 0)
    {
      // don't even save this clump
    }
    else
    {
      double pcti = good/count;
      if (pcti >= pct)
      {
	// keep this clump
	for (c=clumps[i].begin(); c!=clumps[i].end(); ++c)
	{
	  double v;
	  if (_data.getValue(c->first, c->second, v))
	  {
	    g.setValue(c->first, c->second, v);
	  }
	}
      }
    }	  
  }
  VertData2d *ret = new VertData2d(g, _vlevel);
  return ret;
}
