/**
 * @file Volume.cc
 */
#include "Volume.hh"
#include "RadxPersistentClutter.hh"
#include "RadxPersistentClutterFirstPass.hh"
#include "RadxPersistentClutterSecondPass.hh"
#include <rapmath/StatusUserData.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Volume::Volume(void) : VolumeBase(), _p(NULL)
{
}

//------------------------------------------------------------------
Volume::Volume(const Parms *parms, int argc, char **argv):
  VolumeBase(parms, argc, argv), _p(NULL)
{
}

//------------------------------------------------------------------
Volume::~Volume(void)
{
  if (_p != NULL)
  {
    delete _p;
    _p = NULL;
  }
}

//------------------------------------------------------------------
MathUserData *Volume::volumeInit(void)
{
  if (_state == FIRST)
  {
    // _first = false;
    RadxPersistentClutterFirstPass *p =
      new RadxPersistentClutterFirstPass(*_parms);
    _p = (RadxPersistentClutter *)p;
    _p->initFirstTime(this);
    for (size_t ii = 0; ii < _rays->size(); ii++)
    {
      RadxRay *ray = (*_rays)[ii];
      _p->preProcessRay(*ray);
    }
    _state = FIRST_PASS;
    _converged = false;
  }
  else if (_state == FIRST_SECOND_PASS)
  {
    _state = SECOND_PASS;
    _p->initFirstTime(this);
    for (size_t ii = 0; ii < _rays->size(); ii++)
    {
      RadxRay *ray = (*_rays)[ii];
      _p->preProcessRay(*ray);
    }
  }
  StatusUserData *s = new StatusUserData(true);
  return (MathUserData *)s;
}

//------------------------------------------------------------------
MathUserData *Volume::volumeFinish(void)
{
  _doWrite = _p->processFinishVolume(this);
  if (_p->isDone())
  {
    _p->finishLastTimeGood(this);
    if (_state == FIRST_PASS)//_firstPass)
    {
      rewind();
      RadxPersistentClutterSecondPass *p =
	new RadxPersistentClutterSecondPass(*_p);
      delete _p;
      _p = (RadxPersistentClutter *)p;
      _state = FIRST_SECOND_PASS;
      _converged = true;
    }
    else
    {
      _state = DONE;
      fastForward();
    }
  }
  StatusUserData *s = new StatusUserData(true);
  return (MathUserData *)s;
}

//------------------------------------------------------------------
RadxPersistentClutter *Volume::algPtr(void) const
{
  return _p;
}

