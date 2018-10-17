#include "Repoh.hh"
#include "Sweep.hh"
#include "Volume.hh"
#include <FiltAlgVirtVol/InterfaceAlgorithm.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Repoh::Repoh(void) : _ok(false)
{
  Sweep s;
  Volume v;
  _alg = new Algorithm(s, v);
}

//------------------------------------------------------------------
Repoh::Repoh(const Parms &parms, void cleanExit(int)) : _ok(true),
							_parms(parms)
{
  // _parms = RepohParms(parmFileName);
  _parms.printInputOutputs();

  InterfaceAlgorithm::algInit("Repoh", _parms, cleanExit);

  // initiate the algorithm
  Sweep s;
  Volume v;
  _alg = new Algorithm(_parms, s, v);
  if (!_alg->ok())
  {
    printf("ERROR building algorithm object\n");
    cleanExit(-1);
  }
}

//------------------------------------------------------------------
Repoh::~Repoh(void)
{
  if (_alg != NULL)
  {
    delete _alg;
    _alg = NULL;
  }
}

//------------------------------------------------------------------
bool Repoh::run(VolumeData *inputs)
{
  return _alg->update(_parms, inputs);
}
