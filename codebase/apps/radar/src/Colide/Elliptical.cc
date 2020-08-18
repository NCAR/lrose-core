/**
 * @file Elliptical.cc
 */
#include "Elliptical.hh"
#include "EllipticalInfo.hh"
#include "Bad.hh"
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/pmu.h>
#include <toolsa/TaThreadSimple.hh>

/*----------------------------------------------------------------*/
static double _set_conf(const Grid2dOffset &o, const Grid2dOffset &opposite,
			int x, int y, int nx, int ny, 
			const Grid2d &input, const FuzzyF &conf_f)
{
  if (o.outOfRange(x, y, nx, ny))
    return 0.0;
  if (opposite.outOfRange(x, y, nx, ny))
    return 0.0;

  double v1 = o.percentBad(input, x, y);
  double v2 = opposite.percentBad(input, x, y);
  double v3;

  // printf("percent bad = %f %f\n", v1, v2);


  // convert to percent good
  v1 = 1.0 - v1;
  v2 = 1.0 - v2;
  if (v1 == 0 && v2 == 0)
  {
    v3 = 0.0;
  }
  else
  {
    // average of the two?
    v3 = (v1 + v2) / 2.0;

    // if (v1 < v2)
    // {
    //   v3 = v2/(v1+v2);
    // }
    // else
    // {
    //   v3 = v1/(v1+v2);
    // }
  }
      
  return conf_f.apply(v3);
}

/*----------------------------------------------------------------*/
static bool _adjust_1(const Grid2dOffset &o, const int x, const int y,
		      const bool is_min_variance, 
		      const Grid2d &input, bool *first,  double *vbest)
{
  double v;
  bool is_bad;

  if (is_min_variance)
    is_bad = !o.variance(input, x, y, v);
  else
    is_bad = !o.average(input, x, y, v);
  if (is_bad)
    return false;

  if (*first)
  {
    *first = false;
    *vbest = v;
    return true;
  }

  if ( (is_min_variance && (v < *vbest)) || 
       (!is_min_variance && (*vbest < v)))
  {
    *vbest = v;
    return true;
  }
  return false;
}

//------------------------------------------------------------------
TaThread *Elliptical::EllipThreads::clone(const int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(Elliptical::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

/*----------------------------------------------------------------*/
// default constructor
Elliptical::Elliptical(const Parms &p, double angleRes, double smoothThresh,
		       double smoothBackground, bool isMinVariance,
		       int nx, int ny,
		       int nthread, const Grid2d &g):
  _parm(p), _angleRes(angleRes), _smoothThresh(smoothThresh),
  _smoothBackground(smoothBackground), _isMinVariance(isMinVariance)
{
  double a;
    
  int gridNx = g.getNx();
  double missing = g.getMissing();
  for (a=0.0; a<180.0; a+= angleRes)
  {
    Grid2dOffset o(nx, ny, a, gridNx, missing);
    _offsets.push_back(o);
    // if (_parm._is_conf)
    // {
    //   Grid2dOffset o(_parm._conf_window_len, _parm._conf_window_wid, a,
    // 		     gridNx, missing);
    //   _conf_offsets.push_back(o);
    // }
  }

  // square box of radius length at each point, no rotation.
  _neighbor = Grid2dOffset(nx, 0.0, gridNx, missing);
  _thread.init(nthread, false);
}

/*----------------------------------------------------------------*/
// destructor
Elliptical::~Elliptical()
{
}

/*----------------------------------------------------------------*/
void Elliptical::processOrientation(const Grid2d *input, Grid2d *out)
{
  int y;

  int nx, ny;
  nx = input->getNx();
  ny = input->getNy();
  out->setAllMissing();

  // Grid2d *conf = NULL;

  for (y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    EllipticalInfo *info = new EllipticalInfo(this, y, nx,
					      EllipticalInfo::ORIENTATION,
					      input, NULL, out);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();
  // orient_median = orientation;
  // _median(orient_median, 5, 5, 1.0);
//     for (x=0; x<nx; x++)
//     {
//       v = _process_xy(x, y, orientation, input);
//       out.setValue(x, y, v);
//     }
//   }

// }

}

/*----------------------------------------------------------------*/
void Elliptical::process(const Grid2d *input, const Grid2d *orientation,
			 Grid2d *out)
{
  int y;

  int nx, ny;
  nx = input->getNx();
  ny = input->getNy();
  out->setAllMissing();

  // Grid2d *conf = NULL;

  for (y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    EllipticalInfo *info = new EllipticalInfo(this, y, nx,
					      EllipticalInfo::INTEREST,
					      input, orientation, out);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();
  // orient_median = orientation;
  // _median(orient_median, 5, 5, 1.0);
//     for (x=0; x<nx; x++)
//     {
//       v = _process_xy(x, y, orientation, input);
//       out.setValue(x, y, v);
//     }
//   }

// }

}

/*----------------------------------------------------------------*/
void Elliptical::processConfidence(const Grid2d *input,
				   const Grid2d *orientation,
				   Grid2d *out)
{
  int y;

  int nx, ny;
  nx = input->getNx();
  ny = input->getNy();
  out->setAllMissing();

  // Grid2d *conf = NULL;

  for (y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    EllipticalInfo *info = new EllipticalInfo(this, y, nx,
					      EllipticalInfo::CONFIDENCE,
					      input, orientation, out);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();
  // orient_median = orientation;
  // _median(orient_median, 5, 5, 1.0);
//     for (x=0; x<nx; x++)
//     {
//       v = _process_xy(x, y, orientation, input);
//       out.setValue(x, y, v);
//     }
//   }

// }

}

//------------------------------------------------------------------------
void Elliptical::compute(void *ti)
{
  EllipticalInfo *info = static_cast<EllipticalInfo *>(ti);
  for (int x=0; x<info->_nx; x++)
  {
    double v;
    if (info->_info == EllipticalInfo::ORIENTATION)
    {
      v = info->_alg->_process_orientation_xy(x, info->_y, *info->_input);
    }
    else if (info->_info == EllipticalInfo::INTEREST)
    {
      v = info->_alg->_process_xy(x, info->_y, *info->_input,
				  *info->_orientation);
    }
    else if (info->_info == EllipticalInfo::CONFIDENCE)
    {
      v = info->_alg->_process_confidence_xy(x, info->_y, *info->_input,
					     *info->_orientation);
					     
    }
    // if (info->_orientation != NULL && info->_conf != NULL)
    // {
    //   v = info->_alg->_process_xy(x, info->_y, *info->_orientation,
    // 				  *info->_conf, *info->_input);
    // }
    // else if (info->_orientation != NULL && info->_conf == NULL)
    // {
    //   v = info->_alg->_process_xy(x, info->_y, *info->_orientation,
    // 				  *info->_input);
    // }
    // else if (info->_orientation == NULL && info->_conf != NULL)
    // {
    //   v = info->_alg->_process_xy_with_conf(x, info->_y, 
    // 					    *info->_conf,
    // 					    *info->_input);
    // }
    // else
    // {
    //   v = info->_alg->_process_xy(x, info->_y, *info->_input);
    // }
    info->_output->setValue(x, info->_y, v);
  }
  delete info;
}

// /*----------------------------------------------------------------*/
// // process the input data, filtering it elliptically.
// void Elliptical::process(const Grid2d &input, Grid2d &out)
// {
//   int y;
//   int nx, ny;
//   nx = input.getNx();
//   ny = input.getNy();
//   out.setAllToValue(_parm._smooth_background);

//   EllipThreads thread;
//   thread.init(_parm._numThread, false);
//   Grid2d *orientation = NULL;
//   Grid2d *conf = NULL;

//   for (y=0; y<ny; y++)
//   {
//     // register with procmap here
//     PMU_auto_register("Processing y subgrid");
//     EllipticalInfo *info = new EllipticalInfo(this, y, nx, conf, orientation,
// 					      &input, &out);
//     thread.thread(y, (void *)info);
//   }
//   thread.waitForThreads();
//   // for (y=0; y<ny; ++y)
//   // {
//   //   // register with procmap here
//   //   PMU_auto_register("Processing y subgrid");

//   //   for (x=0; x<nx; ++x)
//   //   {
//   //     v = _process_xy(x, y, input);
//   //     out.setValue(x, y, v);
//   //   }
//   // }
// }

// /*----------------------------------------------------------------*/
// void Elliptical::process(const Grid2d &input, Grid2d &out,
// 			 Grid2d &conf,  Grid2d &orientation,
// 			 Grid2d &orient_median)

// {
//   int y;
//   int nx, ny;
//   nx = input.getNx();
//   ny = input.getNy();
//   // out.set_all_to_value(_parm._smooth_background);
//   out.setAllMissing();
//   EllipThreads thread;
//   thread.init(_parm._numThread, false);

//   for (y=0; y<ny; y++)
//   {
//     // register with procmap here
//     PMU_auto_register("Processing y subgrid");
//     EllipticalInfo *info = new EllipticalInfo(this, y, nx, &conf, &orientation,
// 					      &input, &out);
//     thread.thread(y, (void *)info);
//   }
//   thread.waitForThreads();

//   orient_median = orientation;
//   _median(orient_median, 5, 5, 1.0);
//   // for (y=0; y<ny; ++y)
//   // {
//   //   // register with procmap here
//   //   PMU_auto_register("Processing y subgrid");

//   //   for (x=0; x<nx; ++x)
//   //   {
//   //     v = _process_xy(x, y, orientation, conf, input);
//   //     out.setValue(x, y, v);
//   //   }
//   // }
// }

// /*----------------------------------------------------------------*/
// // process the input data, filtering it elliptically.
// void Elliptical::process(const Grid2d &input, Grid2d &out, Grid2d &conf)
// {
//   int y;
//   int nx, ny;

//   nx = input.getNx();
//   ny = input.getNy();
//   out.setAllToValue(_parm._smooth_background);
//   EllipThreads thread;
//   thread.init(_parm._numThread, false);
//   Grid2d *orientation = NULL;
//   for (y=0; y<ny; y++)
//   {
//     // register with procmap here
//     PMU_auto_register("Processing y subgrid");
//     EllipticalInfo *info = new EllipticalInfo(this, y, nx, &conf, orientation,
// 					      &input, &out);
//     thread.thread(y, (void *)info);
//   }
//   thread.waitForThreads();


//   // for (y=0; y<ny; ++y)
//   // {
//   //   // register with procmap here
//   //   PMU_auto_register("Processing y subgrid");

//   //   for (x=0; x<nx; ++x)
//   //   {
//   //     v = _process_xy_with_conf(x, y, conf, input);
//   //     out.setValue(x, y, v);
//   //   }
//   // }
// }

/*----------------------------------------------------------------*/
double Elliptical::_process_orientation_xy(int x, int y,
					   const Grid2d &input) const
{
  double vbest;
  bool first;
  int best;

  int nx, ny;
  nx = input.getNx();
  ny = input.getNx();

  // if the image value is missing, set outputs to missing
  if (input.isMissing(x, y))
  {
    return badvalue::ANGLE;

    // drc.setMissing(x, y);
    // return input.getMissing();
  }
    
  /* find the best offset table and corresponding detection value
   * and direction */
  first = true;
  best = -1;
  vbest = _smoothBackground;
  if (!_neighbor.outOfRange(x, y, nx, ny))
  {
    double v;
    if (_neighbor.maxValue(input, x, y, v))
    {
      if (v > _smoothThresh)
      {
	for (int i=0; i<(int)_offsets.size(); ++i)
	{
	  if (_offsets[i].outOfRange(x, y, nx, ny))
	    continue;
	  if (_adjust_1(_offsets[i], x, y, _isMinVariance, input, &first,
			&vbest))
	  {
	    best = i;
	  }
	}
      }
    }
  }
  if (first)
  {
    return badvalue::ANGLE;
  }
  else
  {
    return _offsets[best].getAngle();
  }
}


/*----------------------------------------------------------------*/
double Elliptical::_process_xy(int x, int y, const Grid2d &input,
			       const Grid2d &orientation) const
{
  double angle;
  if (!orientation.getValue(x, y, angle))
  {
    return badvalue::INTEREST;
  }

  // figure out which offset goes with this angle
  int best = -1;
  for (size_t i=0; i<_offsets.size(); ++i)
  {
    if (_offsets[i].getAngle() == angle)
    {
      best = i;
      break;
    }
  }
  if (best == -1)
  {
    return badvalue::INTEREST;
  }
  bool first = true;
  double vbest = _smoothBackground;
  if (!_adjust_1(_offsets[best], x, y, _isMinVariance,
		 input, &first, &vbest))
  {
    return badvalue::INTEREST;
  }    
  if (_isMinVariance)
  {
    double v;
    if (_offsets[best].average(input, x, y, v))
    {
      vbest = v;
    }
  }
  return vbest;
}

/*----------------------------------------------------------------*/
double Elliptical::_process_confidence_xy(int x, int y, const Grid2d &input,
					  const Grid2d &orientation) const
{
  if (input.isMissing(x,y))
  {
    return badvalue::INTEREST;
  }

  double angle;
  if (!orientation.getValue(x, y, angle))
  {
    return badvalue::INTEREST;
  }

  // figure out which offset goes with this angle
  int best = -1;
  for (size_t i=0; i<_offsets.size(); ++i)
  {
    if (_offsets[i].getAngle() == angle)
    {
      best = i;
      break;
    }
  }
  if (best == -1)
  {
    return badvalue::INTEREST;
  }

  int n = (int)_offsets.size();
  int k = best;
  int k2 = n/2;
  int kopposite;
  if (k < k2)
    kopposite = k + k2;
  else
    kopposite = k + k2 - n;
    
  int nx = input.getNx();
  int ny = input.getNy();
  
  return _set_conf(_offsets[k], _offsets[kopposite],
		   x, y, nx, ny, input, _parm._ellipConf);
}

#ifdef NOTDEF

/*----------------------------------------------------------------*/
double Elliptical::_process_xy(int x, int y, Grid2d &drc,
			       Grid2d &conf, const Grid2d &input) const
{
  double vbest;
  bool first;
  int best;
  vector<double> values;
  int nx, ny;
  nx = input.getNx();
  ny = input.getNy();

  if (_conf_offsets.size() == 0)
  {
    printf("ERROR wrong method called in Elliptical\n");
    return input.getMissing();
  }

  // if the image value is missing, set outputs to missing
  if (input.isMissing(x, y))
  {
    drc.setMissing(x, y);
    conf.setMissing(x, y);
    return input.getMissing();
  }
    
  /* find the best offset table and corresponding detection value
   * and direction */
  first = true;
  best = -1;
  vbest = _parm._smooth_background;
  if (!_neighbor.outOfRange(x, y, nx, ny))
  {
    double v;
    if (_neighbor.maxValue(input, x, y, v))
    {
      if (v > _parm._smooth_thresh)
      {
	for (int i=0; i<(int)_offsets.size(); ++i)
	{
	  if (_offsets[i].outOfRange(x, y, nx, ny))
	    continue;
	  if (_adjust_1(_offsets[i], x, y, _parm._is_min_variance, input,
			&first, &vbest))
	    best = i;
	}
      }
    }
  }

  // if (vbest > 0.4)
  // {
  //   printf("HERE\n");
  // }

  if (first)
  {
    drc.setMissing(x, y);
    conf.setMissing(x, y);
  }
  else
  {
    drc.setValue(x, y, _offsets[best].getAngle());
    if (_parm._is_min_variance)
    {
      double v;
      if (_offsets[best].average(input, x, y, v))
	vbest = v;
    }
    int n = (int)_offsets.size();
    int k = best;
    int k2 = n/2;
    int kopposite;
    if (k < k2)
      kopposite = k + k2;
    else
      kopposite = k + k2 - n;
    
    double c = _set_conf(_conf_offsets[k], _conf_offsets[kopposite],
			 x, y, nx, ny, input, _parm._confF);
    conf.setValue(x, y, c);
    //printf("[%d,%d]=%lf\n", x, y, c);
  }
  return vbest;
}

/*----------------------------------------------------------------*/
double Elliptical::_process_xy(int x, int y, const Grid2d &input) const
{
  double vbest, v;
  bool first;
  int best, nx, ny;

  nx = input.getNx();
  ny = input.getNy();

  // if the image value is missing, set outputs to missing
  if (input.isMissing(x,  y))
    return input.getMissing();
    
  /* find the best offset table and corresponding detection value
   * and direction */
  first = true;
  best = -1;
  vbest = _parm._smooth_background;
  if (!_neighbor.outOfRange(x, y, nx, ny))
  {
    if (_neighbor.maxValue(input, x, y, v))
    {
      if (v > _parm._smooth_thresh)
      {
	for (int i=0; i<(int)_offsets.size(); ++i)
	{
	  if (_offsets[i].outOfRange(x, y, nx, ny))
	    continue;
	  if (_adjust_1(_offsets[i], x, y, _parm._is_min_variance, input,
			&first, &vbest))
	    best = i;
	}
      }
    }
  }
  if ((!first) && _parm._is_min_variance)
  {
    if (_offsets[best].average(input, x, y, v))
      vbest = v;
  }

  return vbest;
}

/*----------------------------------------------------------------*/
double Elliptical::_process_xy_with_conf(int x, int y, Grid2d &conf,
					 const Grid2d  &input) const

{
  double vbest, v;
  bool first;
  int best;
  int nx, ny;

  nx = input.getNx();
  ny = input.getNy();

  if (_conf_offsets.size() == 0)
  {
    printf("ERROR wrong method called in Elliptical\n");
    return input.getMissing();
  }

  // if the image value is missing, set outputs to missing
  if (input.isMissing(x, y))
  {
    conf.setMissing(x, y);
    return input.getMissing();
  }
    
  /* find the best offset table and corresponding detection value
   * and direction */
  first = true;
  best = -1;
  vbest = _parm._smooth_background;
  if (!_neighbor.outOfRange(x, y, nx, ny))
  {
    if (_neighbor.maxValue(input, x, y, v))
    {
      if (v > _parm._smooth_thresh)
      {
	for (int i=0; i<(int)_offsets.size(); ++i)
	{
	  if (_offsets[i].outOfRange(x, y, nx, ny))
	    continue;
	  if (_adjust_1(_offsets[i], x, y, _parm._is_min_variance, input,
			&first, &vbest))
	    best = i;
	}
      }
    }
  }

  if (first)
    conf.setMissing(x, y);
  else
  {
    if (_parm._is_min_variance)
    {
      if (_offsets[best].average(input, x, y, v))
	vbest = v;
    }
    int n = (int)_offsets.size();
    int k = best;
    int k2 = n/2;
    int kopposite;
    if (k < k2)
      kopposite = k + k2;
    else
      kopposite = k + k2 - n;
    double c =  _set_conf(_conf_offsets[k], _conf_offsets[kopposite],
			  x, y, nx, ny, input, _parm._confF);
    conf.setValue(x, y, c);
  }
  return vbest;
}

#endif
