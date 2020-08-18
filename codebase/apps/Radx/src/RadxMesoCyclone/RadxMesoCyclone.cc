#include "RadxMesoCyclone.hh"
#include "Sweep.hh"
#include "Volume.hh"
#include <FiltAlgVirtVol/InterfaceAlgorithm.hh>
#include <toolsa/LogStream.hh>



//------------------------------------------------------------------
RadxMesoCyclone::RadxMesoCyclone(void) : _ok(false)
{
  Sweep s;
  Volume v;
  _alg = new Algorithm(s, v);
}

//------------------------------------------------------------------
RadxMesoCyclone::RadxMesoCyclone(const Parms &parms, void cleanExit(int)) :
  _ok(true), _parms(parms)
{
  Sweep s;
  Volume v;

  InterfaceAlgorithm::algInit("RadxMesoCyclone", _parms, cleanExit);

  // initiate the algorithm
  _alg = new Algorithm(_parms, s, v);
  if (!_alg->ok())
  {
    printf("ERROR building algorithm object\n");
    cleanExit(-1);
  }
}

//------------------------------------------------------------------
RadxMesoCyclone::~RadxMesoCyclone(void)
{
  InterfaceAlgorithm::algFinish();
  if (_alg != NULL)
  {
    delete _alg;
    _alg = NULL;
  }
}

//------------------------------------------------------------------
bool RadxMesoCyclone::run(VolumeData *inputs)
{
  return _alg->update(_parms, inputs);
}
