// # include <stdio.h>
// # include <stdlib.h>
// # include <math.h>
# include "Reg.hh"
// # include <cospa/InterfaceLL.hh>
// # include <Colide/cldHoleFill.hh>

/*----------------------------------------------------------------*/
// build  from input l=locs, other inupts as given., used in junction
// building step as rebuild.
Reg::Reg(//const time_t &itime, 
	 const PointList &l, const Grid2d &data, const Grid2d &lineData,
	 int iid, const ParmRegion &parm) : _uloc(l), 
					    _att(l, data, lineData, parm),
					    _loc(l), _iloc(l)
{
  // _log = LogMsg("Reg");
  _clear();
  // __time = itime;
  _id = iid;
  _loc = l;
  _uloc = l;
  _att = RegAtt(_uloc, data, lineData, parm);
  if (_att.is_empty())
  {
    // Failure in something done so far
    _clear();
    _att.clear();
    return;
  }
}

/*----------------------------------------------------------------*/
Reg::~Reg()
{
}

/*----------------------------------------------------------------*/
void Reg::toImage(Grid2d &g) const
{
  _uloc.toGrid(g, _id);
}

#ifdef NOTDEF
/*----------------------------------------------------------------*/
void Reg::print(int index) const
{
  print(stdout, index);
}

/*----------------------------------------------------------------*/
void Reg::print(void) const
{
  print(stdout, 0);
}

/*----------------------------------------------------------------*/
void Reg::print(FILE *fp, int index) const
{
  char buf[1000];

  sprintf(buf, "region %d:  time:%s id:%d\n", index,
	  InterfaceLL::stime(_time).c_str(), _id);
  printf(buf);
  fprintf(fp, "Attributes:\n");
  _att.print(fp);

  fprintf(fp, "\tlen:%d ilen:%d\n", _loc.num(), _iloc.num());
  fprintf(fp, "locations:\n");
  _loc.print(fp);
  fprintf(fp, "ilocations:\n");
  _iloc.print(fp);
}

/*----------------------------------------------------------------*/
// return true if region looks good.
bool Reg::ok_check_hot(const FiltRegion &parm, bool do_print) const
{
  double a;

  // Are the region attributes ok?
  if (!_att.ok(parm, do_print))
    return false;
    
  // are the hot spots big enough?
  a = _total_hot_area(parm);
 if (a > parm._parm.min_hot_area)
    return true;
  else
  {
    if (do_print) 
      _log.logf(LogMsg::DEBUG_VERBOSE, "ok_check_hot", 
		"failed..area=%f above hot, need area=%f",
		a, parm._parm.min_hot_area);
    return false;
  }
}

/*----------------------------------------------------------------*/
// Merge regions by adding lines and points from m to this.
// NOTE this is only a partial implementation based on need
void Reg::merge(const cldGrid &g, const Reg &m, const FiltRegion &parm)
{
  // Time and id don't change

  // Locations are merge of both locations
  _loc.form_xy_union(m._loc);
  _iloc.form_xy_union(m._iloc);
  _uloc.form_xy_union(m._uloc);

  // redo attributes.
  _att.recompute(g, _uloc);
}

/*----------------------------------------------------------------*/
// fill holes in location points. a slow alg.
void Reg::fill_holes(const int nx, const int ny, int max_fill_area)
{
  cldPointList l(nx, ny);
    
  // Make the hole filler
  cldHoleFill h(nx, ny, _loc, _iloc, max_fill_area);
    
  // Pull out the set of hole fill points to add to the region
  l = h.filled_points();
  _loc.form_xy_union(l);
  l = h.filled_ipoints();
  _iloc.form_xy_union(l);
  l = h.filled_upoints();
  _uloc.form_xy_union(l);
}

/*----------------------------------------------------------------*/
void Reg::set_loc(const cldPointList &l, const cldPointList &il)
{
  _loc = l;
  _iloc = il;
  _uloc = l;
  _uloc.form_xy_union(il);
}

/*----------------------------------------------------------------*/
double Reg::_total_hot_area(const FiltRegion &parm) const
{
  int l, x, y;
  double a;

  a = 0.0;
  for (l=0; l<_uloc.num(); ++l)
  {
    _uloc.ith_int_element(l, x, y);
    if (parm._hotspots.is_nbrhd_point(x, y))
      ++a;
  }
  return a;
}

#endif

/*----------------------------------------------------------------*/
void Reg::_clear(void)
{
  // _att.clear();
  // _time = 0;
  _id = -1;
  _loc.clear();
  _iloc.clear();
  _uloc.clear();
}

