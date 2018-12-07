#include "Alg.hh"
#include "RayData1.hh"
#include "RayData2.hh"
#include "RayData.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Alg::Alg(void) : _ok(false)
{
  RayData v;
  RayData1 r1;
  RayData2 r2;
  _alg = new RadxApp(r1, r2, v);
}

//------------------------------------------------------------------
Alg::Alg(const Parms &parms, void cleanExit(int)) :
  _ok(true), _parms(parms)
{
  RadxApp::algInit("Alg", _parms, cleanExit);

  RayData v;
  RayData1 r1;
  RayData2 r2;

  _alg = new RadxApp(_parms, r1, r2, v);
  if (!_alg->ok())
  {
    delete _alg;
    _alg = NULL;
    LOG(FATAL) << "Building algorithm object";
    cleanExit(-1);
  }
}

//------------------------------------------------------------------
Alg::~Alg(void)
{
  RadxApp::algFinish();
  if (_alg != NULL)
  {
    delete _alg;
    _alg = NULL;
  }
}

//------------------------------------------------------------------
void Alg::printOperators(void) const
{
  _alg->printOperators();
}

