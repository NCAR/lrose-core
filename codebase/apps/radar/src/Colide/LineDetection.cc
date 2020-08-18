/**
 * @file LineDetection.cc
 */
#include "LineDetection.hh"
#include "Params.hh"
#include "LineDetect.hh"
#include "LineDetectOffsets.hh"
#include "LineDetectInfo.hh"
#include "Bad.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
TaThread *LineDetection::LineDetectionThreads::clone(const int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(LineDetection::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

//------------------------------------------------------------------
LineDetection::LineDetection(int nx, int ny, int nthread, const Parms &parms) :
  _nx(nx), _ny(ny), _nthread(nthread), _parms(parms)
{  
  // create the fuzzy function 
  // _parm._dirName = ap->_parm_line[f.filter_index].direction_field_name;
  _allow_missing_side = true;
  _flag_where_flagged = false;
  _flag_replacement = 0;
  _window = Window(nx, ny);
  
  // bool write = true;
  // _direction = Data(_parm._dirName, Data::GRID3D, write);
}

//------------------------------------------------------------------
LineDetection::~LineDetection()
{
}

//------------------------------------------------------------------
void LineDetection::processDir(const Grid2d *input,
			       Grid2d *output)
{
  output->changeMissing(badvalue::ANGLE);
  output->setAllMissing();
  output->setAllToValue(0.0);
  int nx = input->getNx();
  int ny = input->getNy();
  double missing = input->getMissing();
  
  // // algorithm here
  LineDetect *ld = new LineDetect();
  ld->init(_parms, nx, missing, _allow_missing_side);

  LineDetectOffsets *off = new LineDetectOffsets(_parms, _window, _nx, _ny,
						 nx, missing);

  // double dir_missing = e->_direction.getMissing();

  LineDetectionThreads _thread;
  _thread.init(_nthread, false);

  for (int y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    LineDetectInfo *info = new LineDetectInfo(this, y, nx, ny, missing,
					      input, NULL, ld, off, 
					      output);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();

  delete ld;
  delete off;
}

//------------------------------------------------------------------
void LineDetection::processDet(const Grid2d *input,
				const Grid2d *dir,
				Grid2d *output)
{
  output->changeMissing(badvalue::INTEREST);
  output->setAllMissing();
  output->setAllToValue(0.0);
  int nx = input->getNx();
  int ny = input->getNy();
  double missing = input->getMissing();
  
  // // algorithm here
  LineDetect *ld = new LineDetect();
  ld->init(_parms, nx, missing, _allow_missing_side);

  LineDetectOffsets *off = new LineDetectOffsets(_parms, _window, _nx, _ny,
						 nx, missing);

  // double dir_missing = e->_direction.getMissing();

  LineDetectionThreads _thread;
  _thread.init(_nthread, false);

  for (int y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    LineDetectInfo *info = new LineDetectInfo(this, y, nx, ny, missing,
					      input, dir, ld, off, 
					      output);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();

  delete ld;
  delete off;
}

//------------------------------------------------------------------
void LineDetection::compute(void *ti)
{
  LineDetectInfo *info = static_cast<LineDetectInfo *>(ti);
  for (int x=0; x<info->_nx; x++)
  {
    double v;
    if (info->_dirData == NULL)
    {
      v = info->_alg->_line_direct_xy(x, info->_y, info->_nx, info->_ny,
				      *info->_inData, *info->_ld,
				      *info->_off);
    }
    else
    {
      v = info->_alg->_line_detect_xy(x, info->_y, info->_nx, info->_ny,
				      *info->_inData, *info->_dirData,
				      *info->_ld, *info->_off);
    }
    info->_outData->setValue(x, info->_y, v);
    // info->_outDir->setValue(x, info->_y, dir);
  }
  delete info;
}

// //------------------------------------------------------------------
// bool LineDetection::create_inputs(const time_t &t,
// 				  const vector<Data> &input,
// 				  const vector<Data> &output)
// {
//   _direction.clear();
//   return true;
// }

// //------------------------------------------------------------------
// void LineDetection::create_extra(FiltInfo &info) const
// {
//   LineDetectionExtra *e = new LineDetectionExtra();
//   info.getOutput().storeExtra((void *)e);
// }

// //------------------------------------------------------------------
// bool LineDetection::store_outputs(const Data &o, Info *info,
// 			      vector<FiltInfo> &extra,
// 			      vector<Data> &output)
// {
//   _direction.clear();

//   for (size_t i = 0; i<extra.size(); ++i)
//   {
//     LineDetectionExtra *e = (LineDetectionExtra *)extra[i].getOutput().getExtra();
//     _direction.add(e->_direction, e->_vlevel, e->_vlevel_index, e->_gp);
//     delete e;
//   }

//   output.push_back(o);
//   output.push_back(_direction);

//   return true;
// }

// //------------------------------------------------------------------
// void LineDetection::set_info(Info **info) const
// {
// }

// //------------------------------------------------------------------
// void LineDetection::set_input_info(Info **info) const
// {
// }

// //------------------------------------------------------------------
// void LineDetection::vertical_level_change(void)
// {
//   // default is to do nothing
// }

// //------------------------------------------------------------------
// void LineDetection::share_mdv_info(const FiltAlgMdvInfo &mdv_info)
// {
// }

/*----------------------------------------------------------------*/
double LineDetection::_line_direct_xy(const int x, const int y, const int nx,
					 const int ny, const Grid2d &input,
					 const LineDetect &ld,
					 const LineDetectOffsets &o) const
{
  // check for good conditions to compute detection.
  if (!ld.ok(x, y, nx, ny, input))
  {
    // direction = dir_missing;
    if ((!input.isMissing(x, y) && _flag_where_flagged))
      return _flag_replacement;
    else
      return badvalue::ANGLE;
  }
    
  /* find the best offset table and corresponding detection value
   * and direction */
  double direction;

  ld.best_direction(input, -1.0, x, y, nx, ny, o, direction);
  return direction;
}  

/*----------------------------------------------------------------*/
double LineDetection::_line_detect_xy(const int x, const int y, const int nx,
					 const int ny, const Grid2d &input,
				      const Grid2d &dir,
					 const LineDetect &ld,
					 const LineDetectOffsets &o) const
				 // 	 double dir_missing, 
				 // double &direction) const
{
  // check for good conditions to compute detection.
  if (dir.isMissing(x,y))
  {
    return badvalue::INTEREST;
  }
  if (!ld.ok(x, y, nx, ny, input))
  {
    if ((!input.isMissing(x, y) && _flag_where_flagged))
      return _flag_replacement;
    else
      return badvalue::INTEREST;
  }
    
  /* find the best offset table and corresponding detection value
   * and direction */
  double det;
  double direction = dir.getValue(x,y);
  if (!ld.valueAtDirection(input, direction, -1.0, x, y, nx, ny, o, det))
  {
    // direction = dir_missing;
    if (!input.isMissing(x, y) && _flag_where_flagged)
    {
      return _flag_replacement;
    }
    else
      return badvalue::INTEREST;
  }
    
  if (det == -1.0)
  {
    // direction = dir_missing;
    if (_flag_where_flagged)
    {
      // direction = dir_missing;
      det = _flag_replacement;
    }
  }
  else
  {
    if (det < 0) det = 0.0;
    if (det > 1.0) det = 1.0;
  }
  return det;
}  

