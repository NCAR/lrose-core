/**
 * @file Main.cc
 * @mainpage RadiaFilt
 */

#include "Volume.hh"
#include "Sweep.hh"
#include "RadxMesoCyclone.hh"
#include "Parms.hh"
#include <FiltAlgVirtVol/FiltAlgParmsTemplate.hh>

//------------------------------------------------------------------
static void tidyAndExit(int i)
{
  exit(i);
}

//------------------------------------------------------------------
int main(int argc, char **argv)
{
  Parms parms;
  if (!parmAppInit(parms, argc, argv))
  {
    exit(0);
  }
  RadxMesoCyclone alg(parms, tidyAndExit);
  if (!alg.ok())
  {
    exit(1);
  }

  if (!parms.checkConsistency(*alg.getAlgorithm()))
  {
     exit(1);
  }

  Volume volume(parms, argc, argv);

  time_t t;
  while (volume.trigger(t))
  {
    alg.run((VolumeData *)(&volume));
    volume.mesoOutput(t);
    volume.clear();
  }

  printf("Done\n");
  tidyAndExit(0);
}
