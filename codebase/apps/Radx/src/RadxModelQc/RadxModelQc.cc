#include "RadxModelQc.hh"
#include "RayData1.hh"
#include "Volume.hh"
#include <radar/RadxAppSweepDataSimple.hh>
#include <radar/RadxApp.hh>
#include <toolsa/LogStream.hh>



//------------------------------------------------------------------
RadxModelQc::RadxModelQc(void) : _ok(false)
{
  Volume v;
  RayData1 r1;
  RadxAppSweepDataSimple r2;
  _alg = new RadxApp(r1, r2, v);
}

//------------------------------------------------------------------
RadxModelQc::RadxModelQc(const Parms &parms, void cleanExit(int)) :
  _ok(true), _parms(parms)
{
  RadxApp::algInit("RadxModelQc", _parms, cleanExit);

  Volume v;
  RayData1 r1;
  RadxAppSweepDataSimple r2;

  // initiate the algorithm
  _alg = new RadxApp(_parms, r1, r2, v);

  if (!_alg->ok())
  {
    LOG(ERROR) << "building algorithm object";
    cleanExit(-1);
  }
}

//------------------------------------------------------------------
RadxModelQc::~RadxModelQc(void)
{
  RadxApp::algFinish();
  if (_alg != NULL)
  {
    delete _alg;
    _alg = NULL;
  }
}

//------------------------------------------------------------------
void RadxModelQc::printOperators(void) const
{
  _alg->printOperators();
}

//------------------------------------------------------------------
bool RadxModelQc::run(Volume *volume)
{
  return _alg->update(_parms, volume);
}

//------------------------------------------------------------------
bool RadxModelQc::write(Volume *volume)
{
  return _alg->write(volume);
}
