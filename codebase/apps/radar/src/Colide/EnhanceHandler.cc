/**
 * @file EnhanceHandler.cc
 */
#include "EnhanceHandler.hh"
// #include "Params.hh"
#include "Enhance.hh"
// #include "Bad.hh"
// #include <toolsa/pmu.h>
// #include <toolsa/LogStream.hh>

//------------------------------------------------------------------
EnhanceHandler::EnhanceHandler(int len, int width, int numAngles,
			       const FuzzyF &f,
			       int num_thread)
{
  _parm._allow_missing_side = true;
  _parm._out_orient = "";
  _parm._window = Window(len, width, numAngles);
  _parm._enhanceF = f;
  _parm._numThread = num_thread;
}


//------------------------------------------------------------------
EnhanceHandler::~EnhanceHandler()
{
}

//------------------------------------------------------------------
void EnhanceHandler::processDir(const Grid2d *input, Grid2d *output)
{
  Enhance enhance(_parm);

  output->changeMissing(badvalue::ANGLE);
  output->setAllMissing();
  enhance.line_enhance_direction(input, output);
}

//------------------------------------------------------------------
void EnhanceHandler::process(const Grid2d *input, Grid2d *output)
{
  Enhance enhance(_parm);

  output->setAllMissing();
  enhance.line_enhance(input, output);
}

