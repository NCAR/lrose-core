#include "Repoh.hh"
#include <FiltAlgVirtVol/InterfaceAlgorithm.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Repoh::Repoh(const std::string &parmFileName, void cleanExit(int)) : _ok(true)
{
  _parms = RepohParms(parmFileName);
  _parms.printInputOutputs();

  InterfaceAlgorithm::algInit("Repoh", _parms, cleanExit);

  // initiate the algorithm
  _alg = new Algorithm(_parms);
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
