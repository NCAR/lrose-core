/**
 * @file RegHandler.cc
 */
#include "RegHandler.hh"
#include "Reg.hh"
#include <toolsa/LogStream.hh>
#include <euclid/Grid2dClump.hh>
#include <euclid/PointList.hh>

/*----------------------------------------------------------------*/
static bool _hot_enough(const PointList &r, const Grid2d &hot,
			const double min_area)
{
  return r.numPointsAboveThresh(hot, 0.5) >= min_area;
}

//------------------------------------------------------------------
RegHandler::RegHandler(double min_mean_len, double min_max_len,
		       double min_min_len, double min_area,
		       double min_hot_area)
{
  _parm._min_mean_length = min_mean_len;
  _parm._min_max_length = min_max_len;
  _parm._min_min_length = min_min_len;
  _parm._min_area = min_area;
  _parm._min_hot_area = min_hot_area;
  _parm._hole_max_fill_area = 120;
  _parm._min_percent_for_circle = 0.65;
}

//------------------------------------------------------------------
RegHandler::~RegHandler()
{
}

//------------------------------------------------------------------
bool RegHandler::process(const Grid2d *hot, const Grid2d *region,
			 const Grid2d *full, Grid2d *out)
{
  Grid2dClump C(*region, 0.0);
  std::vector<PointList> r = C.buildRegionPointlists();

  out->setAllMissing();
  // vector<Reg> _region;
  int j=1;
  for (size_t i=0; i<r.size(); ++i)
  {
    if (r[i].size() >= _parm._min_area)
    {
      // see if it is hot enough
      if (_hot_enough(r[i], *hot, _parm._min_hot_area))
      {
	// see if it is too circular
	if (!r[i].isCircular(_parm._min_percent_for_circle))
	{
	  // here is a clump, build the region object from that
	  Reg R(r[i], *full, *region, j++, _parm);
	  R.toImage(*out);
	}
      }
    }
  }

  return true;
}


