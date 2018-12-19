#include "RadxModelQc.hh"
#include "RayData1.hh"
#include "RayData2.hh"
#include "RayData.hh"
#include <radar/RadxApp.hh>
#include <toolsa/LogStream.hh>



//------------------------------------------------------------------
RadxModelQc::RadxModelQc(void) : _ok(false)
{
  RayData v;
  RayData1 r1;
  RayData2 r2;
  _alg = new RadxApp(r1, r2, v);
}

//------------------------------------------------------------------
RadxModelQc::RadxModelQc(const Parms &parms, void cleanExit(int)) :
  _ok(true), _parms(parms)
{
  RadxApp::algInit("RadxModelQc", _parms, cleanExit);

  RayData v;
  RayData1 r1;
  RayData2 r2;
  // initiate the algorithm
  _alg = new RadxApp(_parms, r1, r2, v);

  // _alg = new Algorithm(_parms, s, v);
  if (!_alg->ok())
  {
    printf("ERROR building algorithm object\n");
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
bool RadxModelQc::run(RayData *inputs)
{
  return _alg->update(_parms, inputs);
}

//------------------------------------------------------------------
bool RadxModelQc::write(RayData *data)
{
  return _alg->write(data);
}
