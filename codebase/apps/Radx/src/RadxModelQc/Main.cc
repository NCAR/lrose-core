#include "RadxModelQc.hh"
#include "Parms.hh"
#include "CircularLookupHandler.hh"
#include "RayData.hh"
#include <radar/RadxAppParmsTemplate.hh>
#include <toolsa/LogStream.hh>

static CircularLookupHandler *_lookup = NULL;

static void _createLookups(double r, RadxVol &vol);
static bool _processVolume(const Parms &parms, RayData &vol,
			   RadxModelQc &alg);

//--------------------------------------------------------------------
// tidy up on exit
static void cleanup(int sig)
{
  if (_lookup != NULL)
  {
    delete _lookup;
  }
  exit(sig);
}

//--------------------------------------------------------------------
// Handle out-of-memory conditions
static void out_of_store()
{
  exit(-1);
}

//--------------------------------------------------------------------
int main(int argc, char **argv)
{
  Parms params;
  if (!parmAppInit(params, argc, argv))
  {
    exit(0);
  }

  RadxModelQc alg(params, cleanup);
  if (!alg.ok())
  {
    exit(1);
  }
  
  RayData volume(&params, argc, argv);
  string path;
  while (volume.triggerRadxVolume(path))
  {
    LOG(DEBUG) << " processing  " << path;
    _createLookups(params.variance_radius_km, volume.getVolRef());
    if (!_processVolume(params, volume, alg))
    {
      LOG(ERROR) << "Processing this volume. No output";
    }
    else
    {
      alg.write(&volume);
    }
  }

  LOG(DEBUG) << "Done";
  return 0;
}

//-----------------------------------------------------------------------
static void _createLookups(double r, RadxVol &vol)
{
  if (_lookup != NULL)
  {
    delete _lookup;
    _lookup = NULL;
  }

  // pass in # as arg
  _lookup = new CircularLookupHandler(r, vol);
}

//-----------------------------------------------------------------------
static bool _processVolume(const Parms &parms, RayData &volume,
			   RadxModelQc &alg)
{
  volume.initialize(_lookup);
  return alg.run(&volume);
}

