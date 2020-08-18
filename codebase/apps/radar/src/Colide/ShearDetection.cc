/**
 * @file ShearDetection.cc
 */
#include "ShearDetection.hh"
#include "Params.hh"
#include "ShearDetect.hh"
#include "ShearDetectOffsets.hh"
#include "ShearDetectInfo.hh"
#include "Bad.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
TaThread *ShearDetection::ShearDetectionThreads::clone(const int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(ShearDetection::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

//------------------------------------------------------------------
ShearDetection::ShearDetection(int nx, int ny, int nthread,
			       const Parms &parms, const MdvxProj &proj) :
  _nx(nx), _ny(ny), _nthread(nthread), _parms(parms), _proj(proj)
{  
  _window = Window(nx, ny);
  
  _shearMin = 0.5;
  _cutAngle = 20;
  _testCircleDiameter = 0.4;
  _secondaryRange[0] = -17;
  _secondaryRange[1] = 96;
  _isConvergent = true;

  // bool write = true;
  // _direction = Data(_parm._dirName, Data::GRID3D, write);
}

//------------------------------------------------------------------
ShearDetection::~ShearDetection()
{
}

//------------------------------------------------------------------
void ShearDetection::processDir(const Grid2d *input, const Grid2d *secondary,
			       Grid2d *output)
{
  output->changeMissing(badvalue::ANGLE);
  output->setAllMissing();
  output->setAllToValue(0.0);
  int nx = input->getNx();
  int ny = input->getNy();
  double missing = input->getMissing();
  
  // // algorithm here
  ShearDetect *sd = new ShearDetect();
  sd->init(_parms, _proj, nx, missing, _testCircleDiameter);
  ShearDetectOffsets *off = new ShearDetectOffsets(_parms, _window, _nx, _ny,
						   nx, missing);

  ShearDetectionThreads _thread;
  _thread.init(_nthread, false);

  for (int y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    ShearDetectInfo *info = new ShearDetectInfo(this, y, nx, ny, input,
						secondary, NULL, 
						sd, off, output);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();

  delete sd;
  delete off;
}

//------------------------------------------------------------------
void ShearDetection::processDet(const Grid2d *input,
				const Grid2d *dir,
				const Grid2d *secondary,
				Grid2d *output)
{
  output->changeMissing(badvalue::INTEREST);
  output->setAllMissing();
  output->setAllToValue(0.0);
  int nx = input->getNx();
  int ny = input->getNy();
  double missing = input->getMissing();
  
  // // algorithm here
  ShearDetect *sd = new ShearDetect();
  sd->init(_parms, _proj, nx, missing, _testCircleDiameter);
  ShearDetectOffsets *off = new ShearDetectOffsets(_parms, _window, _nx, _ny,
						   nx, missing);

  ShearDetectionThreads _thread;
  _thread.init(_nthread, false);

  for (int y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    ShearDetectInfo *info = new ShearDetectInfo(this, y, nx, ny, input,
						secondary, dir, 
						sd, off, output);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();

  delete sd;
  delete off;
}

//------------------------------------------------------------------
void ShearDetection::compute(void *ti)
{
  ShearDetectInfo *info = static_cast<ShearDetectInfo *>(ti);
  for (int x=0; x<info->_nx; x++)
  {
    double v;
    if (info->_direction == NULL)
    {
      v = info->_alg->_shear_direct_xy(x, info->_y, info->_nx, info->_ny,
				       *info->_inData, *info->_secondary,
				       *info->_sd,
				       *info->_off);
    }
    else
    {
      v = info->_alg->_shear_detect_xy(x, info->_y, info->_nx, info->_ny,
				       *info->_inData, *info->_secondary,
				       *info->_direction,
				       *info->_sd,
				       *info->_off);
    }
    info->_out->setValue(x, info->_y, v);
    // info->_outDir->setValue(x, info->_y, dir);
  }
  delete info;
}

/*----------------------------------------------------------------*/
double ShearDetection::_shear_direct_xy(const int x, const int y, const int nx,
					const int ny, const Grid2d &input,
					const Grid2d &secondary,
					const ShearDetect &ld,
					const ShearDetectOffsets &o) const
{
  if (!ld.secondary_ok(x, y, nx, ny, secondary, _secondaryRange))
  {
    return badvalue::ANGLE;
  }
  /* find the best direction */
  double dir;

  if (!ld.best_direction(input, x, y, nx, ny, o, _shearMin, dir))
  {
    return  badvalue::ANGLE;
  }
  return dir;
}  

/*----------------------------------------------------------------*/
double ShearDetection::_shear_detect_xy(const int x, const int y, const int nx,
					const int ny, const Grid2d &input,
					const Grid2d &secondary,
					const Grid2d &dir,
					const ShearDetect &ld,
					const ShearDetectOffsets &o) const
				 // 	 double dir_missing, 
				 // double &direction) const
{
  if (dir.isMissing(x,y))
  {
    return badvalue::INTEREST;
  }
  double direction = dir.getValue(x,y);

  double det;
  if (!ld.valueAtDirection(input, direction, x, y, nx, ny, o, _cutAngle,
			   _isConvergent, det))
  {
    return badvalue::INTEREST;
  }			     
  if (det < 0) det = 0;
  if (det > 1) det = 1;
  return det;
}  

