/**
 * @file Colide.cc
 */
#include "Colide.hh"
#include "Sweep.hh"
#include "Volume.hh"
#include <FiltAlgVirtVol/InterfaceAlgorithm.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Colide::Colide(void) : _ok(false)
{
  Sweep s;
  Volume v;
  _alg = new Algorithm(s, v);
}

//------------------------------------------------------------------
Colide::Colide(const Parms &parms, void cleanExit(int)) : _ok(true),
							  _parms(parms)
{
  Sweep s;
  Volume v;
  
  // _parms = Parms(parmFileName);
  // _parms.printInputOutputs();

  InterfaceAlgorithm::algInit("Colide", _parms, cleanExit);

  // initiate the algorithm
  _alg = new Algorithm(_parms, s, v);
  if (!_alg->ok())
  {
    printf("ERROR building algorithm object\n");
    cleanExit(-1);
  }
}

//------------------------------------------------------------------
Colide::~Colide(void)
{
  if (_alg != NULL)
  {
    delete _alg;
    _alg = NULL;
  }
}

//------------------------------------------------------------------
bool Colide::run(VolumeData *inputs)
{
  return _alg->update(_parms, inputs);
}
