/**
 * @file EllipHandler.cc
 */
#include "EllipHandler.hh"
#include "Params.hh"
#include "Elliptical.hh"
#include "EllipticalInfo.hh"
#include "Bad.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
EllipHandler::EllipHandler(int nx, int ny, int nthread, const Parms &parms) :
  _nx(nx), _ny(ny), _nthread(nthread), _parms(parms)
{  
  // // create the fuzzy function 
  // // _parm._dirName = ap->_parm_line[f.filter_index].direction_field_name;
  // _allow_missing_side = true;
  // _flag_where_flagged = false;
  // _flag_replacement = 0;
  _window = Window(nx, ny);
  
  // bool write = true;
  // _direction = Data(_parm._dirName, Data::GRID3D, write);
  _angle_res = 15;
  _smooth_thresh = 0.4;
  _smooth_background = 0.0;
  _is_min_variance = false;
}

//------------------------------------------------------------------
EllipHandler::~EllipHandler()
{
}

//------------------------------------------------------------------
void EllipHandler::processDir(const Grid2d *input, Grid2d *output)
{
  output->changeMissing(badvalue::ANGLE);
  output->setAllMissing();
  output->setAllToValue(0.0);
  
  Elliptical ellip(_parms, _angle_res, _smooth_thresh,
		   _smooth_background, _is_min_variance,
		   _nx, _ny, _nthread, *input);
  ellip.processOrientation(input, output);
}

//------------------------------------------------------------------
void EllipHandler::process(const Grid2d *input, const Grid2d *dir,
			   Grid2d *output)
{
  output->changeMissing(badvalue::INTEREST);
  output->setAllMissing();
  output->setAllToValue(0.0);

  Elliptical ellip(_parms, _angle_res, _smooth_thresh,
		   _smooth_background, _is_min_variance,
		   _nx, _ny, _nthread, *input);
  ellip.process(input, dir, output);
}

//------------------------------------------------------------------
void EllipHandler::processConf(const Grid2d *input, const Grid2d *dir,
				 Grid2d *output)
{
  output->changeMissing(badvalue::INTEREST);
  output->setAllMissing();
  output->setAllToValue(0.0);

  Elliptical ellip(_parms, _angle_res, _smooth_thresh,
		   _smooth_background, _is_min_variance,
		   _nx, _ny, _nthread, *input);
  ellip.processConfidence(input, dir, output);
}

// //------------------------------------------------------------------
// bool EllipHandler::create_inputs(const time_t &t,
// 				  const vector<Data> &input,
// 				  const vector<Data> &output)
// {
//   _direction.clear();
//   return true;
// }

// //------------------------------------------------------------------
// void EllipHandler::create_extra(FiltInfo &info) const
// {
//   EllipHandlerExtra *e = new EllipHandlerExtra();
//   info.getOutput().storeExtra((void *)e);
// }

// //------------------------------------------------------------------
// bool EllipHandler::store_outputs(const Data &o, Info *info,
// 			      vector<FiltInfo> &extra,
// 			      vector<Data> &output)
// {
//   _direction.clear();

//   for (size_t i = 0; i<extra.size(); ++i)
//   {
//     EllipHandlerExtra *e = (EllipHandlerExtra *)extra[i].getOutput().getExtra();
//     _direction.add(e->_direction, e->_vlevel, e->_vlevel_index, e->_gp);
//     delete e;
//   }

//   output.push_back(o);
//   output.push_back(_direction);

//   return true;
// }

// //------------------------------------------------------------------
// void EllipHandler::set_info(Info **info) const
// {
// }

// //------------------------------------------------------------------
// void EllipHandler::set_input_info(Info **info) const
// {
// }

// //------------------------------------------------------------------
// void EllipHandler::vertical_level_change(void)
// {
//   // default is to do nothing
// }

// //------------------------------------------------------------------
// void EllipHandler::share_mdv_info(const FiltAlgMdvInfo &mdv_info)
// {
// }

// /*----------------------------------------------------------------*/
// double EllipHandler::_line_direct_xy(const int x, const int y, const int nx,
// 					 const int ny, const Grid2d &input,
// 					 const LineDetect &ld,
// 					 const LineDetectOffsets &o) const
// {
//   // check for good conditions to compute detection.
//   if (!ld.ok(x, y, nx, ny, input))
//   {
//     // direction = dir_missing;
//     if ((!input.isMissing(x, y) && _flag_where_flagged))
//       return _flag_replacement;
//     else
//       return badvalue::ANGLE;
//   }
    
//   /* find the best offset table and corresponding detection value
//    * and direction */
//   double direction;

//   ld.best_direction(input, -1.0, x, y, nx, ny, o, direction);
//   return direction;
// }  

// /*----------------------------------------------------------------*/
// double EllipHandler::_line_detect_xy(const int x, const int y, const int nx,
// 				     const int ny, const Grid2d &input,
// 				     const Grid2d &dir,
// 				     const LineDetect &ld,
// x				     const LineDetectOffsets &o) const
// 				 // 	 double dir_missing, 
// 				 // double &direction) const
// {
//   // check for good conditions to compute detection.
//   if (dir.isMissing(x,y))
//   {
//     return badvalue::INTEREST;
//   }
//   if (!ld.ok(x, y, nx, ny, input))
//   {
//     if ((!input.isMissing(x, y) && _flag_where_flagged))
//       return _flag_replacement;
//     else
//       return badvalue::INTEREST;
//   }
    
//   /* find the best offset table and corresponding detection value
//    * and direction */
//   double det;
//   double direction = dir.getValue(x,y);
//   if (!ld.valueAtDirection(input, direction, -1.0, x, y, nx, ny, o, det))
//   {
//     // direction = dir_missing;
//     if (!input.isMissing(x, y) && _flag_where_flagged)
//     {
//       return _flag_replacement;
//     }
//     else
//       return badvalue::INTEREST;
//   }
    
//   if (det == -1.0)
//   {
//     // direction = dir_missing;
//     if (_flag_where_flagged)
//     {
//       // direction = dir_missing;
//       det = _flag_replacement;
//     }
//   }
//   else
//   {
//     if (det < 0) det = 0.0;
//     if (det > 1.0) det = 1.0;
//   }
//   return det;
// }  

