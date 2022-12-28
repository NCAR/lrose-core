/**
 * @file VolumeTemplate.cc
 */
#include "VolumeTemplate.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
VolumeTemplate::VolumeTemplate(void) : VolumeBase()
{
}

//------------------------------------------------------------------
VolumeTemplate::~VolumeTemplate(void)
{
}

//------------------------------------------------------------------
MathUserData *VolumeTemplate::volumeInit(void)
{
  return NULL;
}

//------------------------------------------------------------------
MathUserData *VolumeTemplate::volumeFinish(void)
{
  return NULL;
}

//------------------------------------------------------------------
RadxPersistentClutter *VolumeTemplate::algPtr(void) const
{
  return NULL;
}
