/**
 * @file Enhance.cc
 */
#include "Enhance.hh"
#include "EnhanceInfo.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/pmu.h>
#include <toolsa/TaThreadSimple.hh>

//------------------------------------------------------------------
TaThread *Enhance::EnhanceThreads::clone(const int index)
{
  // it is a simple thread that uses the Algorithm::compute() as method
  TaThreadSimple *t = new TaThreadSimple(index);
  t->setThreadMethod(Enhance::compute);
  t->setThreadContext(this);
  return (TaThread *)t;
}

/*----------------------------------------------------------------*/
Enhance::Enhance(const ParmEnhance &p) : _parm(p)
{
  _thread.init(_parm._numThread, false);
}

/*----------------------------------------------------------------*/
Enhance::~Enhance()
{
}

/*----------------------------------------------------------------*/
void Enhance::line_enhance_direction(const Grid2d *input,  Grid2d *out)
{
  int y;
  EnhancementD *e = new EnhancementD(_parm);

  int nx, ny;
  nx = input->getNx();
  ny = input->getNy();

  EnhancementOffsetsD *o = new EnhancementOffsetsD(_parm._window, nx,
						   input->getMissing());

  for (y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    EnhanceInfo *info = new EnhanceInfo(this, y, nx, ny, input, e, o, out,
					false);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();

  delete e;
  delete o;
}

/*----------------------------------------------------------------*/
void Enhance::line_enhance(const Grid2d *input,  Grid2d *out)
{
  int y;
  EnhancementD *e = new EnhancementD(_parm);

  int nx, ny;
  nx = input->getNx();
  ny = input->getNy();

  EnhancementOffsetsD *o = new EnhancementOffsetsD(_parm._window, nx,
						   input->getMissing());
  for (y=0; y<ny; y++)
  {
    // register with procmap here
    PMU_auto_register("Processing y subgrid");
    EnhanceInfo *info = new EnhanceInfo(this, y, nx, ny, input, e, o, out,
					true);
    _thread.thread(y, (void *)info);
  }
  _thread.waitForThreads();

  delete e;
  delete o;
}

//------------------------------------------------------------------
void Enhance::compute(void *ti)
{
  EnhanceInfo *info = static_cast<EnhanceInfo *>(ti);
  for (int x=0; x<info->_nx; x++)
  {
    double v;
    if (info->_interest)
    {
      v = info->_alg->_enhancement_xy(*info->_inData, x, info->_y,
				      info->_nx, info->_ny, *info->_e,
				      *info->_o);
    }
    else
    {
      v = info->_alg->_enhancement_angle_xy(*info->_inData, x, info->_y,
					    info->_nx, info->_ny, *info->_e,
					    *info->_o);
      if (v != badvalue::ANGLE)
      {
	// convert angle to an image orientation..
	v = 90 - v;
	while (v < 0.0)
	  v += 180.0;
      }
    }
    info->_outData->setValue(x, info->_y, v);
  }
  delete info;
}

