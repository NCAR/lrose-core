/**
 * @file Alg.cc
 */
#include "Alg.hh"
#include "RayData1.hh"
#include "Volume.hh"
#include <rapmath/MathDataSimple.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Alg::Alg(void) : _ok(false)
{
  Volume v;
  RayData1 r1;
  MathDataSimple r2;
  _alg = new RadxApp(r1, r2, v);
}

//------------------------------------------------------------------
Alg::Alg(const Parms &parms, void cleanExit(int)) : _ok(true), _parms(parms)
{
  RadxApp::algInit("Radx2PersistentClutter", parms, cleanExit);

  Volume v;
  RayData1 r1;
  MathDataSimple r2;
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

//------------------------------------------------------------------
bool Alg::run(Volume *volume)
{
  volume->initialize();
  return _alg->update(_parms, volume);
}

//------------------------------------------------------------------
bool Alg::write(Volume *volume)
{
  return _alg->write(volume);
}

//------------------------------------------------------------------
bool Alg::write(Volume *volume, const std::string &url)
{
  return _alg->write(volume, url);
}
