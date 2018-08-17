#include <rapmath/MathParser.hh>
#include <Params.hh>
#include "CircularLookupHandler.hh"
#include "RayData.hh"
#include <radar/RadxApp.hh>
#include <radar/RadxAppTemplate.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/LogStream.hh>
#include <vector>
#include <string>
#include <algorithm>



static CircularLookupHandler *_lookup = NULL;

static void _initializeInputs(const MathParser &P, const Params &parm, 
			      const std::vector<std::string> &fixedConst,
			      RadxApp &_alg);
static void _createLookups(MathParser &P, double r, RadxVol &vol);
static void _processVolume(MathParser &P, RadxVol &vol,
			   const std::vector<std::string> &outputKeep,
			   bool outputAll);

static bool _matches_fixed_const(const Params &parm,
				 const std::string &s);

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
  RadxApp _alg;           /**< The algorithm object used to run filters */
  Params params;
  parmAppInit(params, _alg, argc, argv);

  std::vector<std::string> outputKeep;
  for (int i=0; i<params.output_fields_n; ++i)
  {
    outputKeep.push_back(params._output_fields[i]);
  }

  MathParser P;

  // add in the user unary filters
  for (int i=0; i<params.userUnaryFilters_n; ++i)
  {	 
    P.addUserUnaryOperator(params._userUnaryFilters[i].interface,
			   params._userUnaryFilters[i].description);
  }
  // add in the user unary volume filters
  for (int i=0; i<params.userVolumeFilters_n; ++i)
  {	 
    P.addUserUnaryOperator(params._userVolumeFilters[i].interface,
			   params._userVolumeFilters[i].description);
  }

  // figure out what the fixed constants are
  vector<string> fixedConst;
  for (int j=0; j<params.fixed_const_n; ++j)
  {
    fixedConst.push_back(params._fixed_const[j]);
  }

  vector<string> userData;

  // parse the volume filters
  for (int i=0; i<params.vol_filter_n; ++i)
  {
    P.parse(params._vol_filter[i], MathParser::VOLUME_BEFORE, fixedConst,
	    userData);
  }

  // parse the non volume filters, 2d
  for (int i=0; i<params.sweep_filter_n; ++i)
  {
    P.parse(params._sweep_filter[i], MathParser::LOOP2D, fixedConst, userData);
  }

  // parse the non volume filters, 1d
  for (int i=0; i<params.filter_n; ++i)
  {
    P.parse(params._filter[i], MathParser::LOOP1D, fixedConst, userData);
  }

  // parse the volume filters
  for (int i=0; i<params.vol_filter_after_n; ++i)
  {
    P.parse(params._vol_filter_after[i], MathParser::VOLUME_AFTER, fixedConst,
	    userData);
  }

  // make sure inputs are good
  _initializeInputs(P, params, fixedConst, _alg);

  RadxVol vol;
  time_t t;
  bool last;
  bool first = true;
  while (_alg.trigger(vol, t, last))
  {
    if (first)
    {
      first = false;
      _createLookups(P, params.variance_radius_km, vol);
    }
    _processVolume(P, vol, outputKeep, params.output_all_fields);
    _alg.write(vol, t);
  }

  P.cleanup();
  LOG(DEBUG) << "Done";
  return 0;
}

//-----------------------------------------------------------------------
static void _createLookups(MathParser &P, double r, RadxVol &vol)
{
  // pass in # as arg
  _lookup = new CircularLookupHandler(r, vol);
}

//-----------------------------------------------------------------------
static void _processVolume(MathParser &P, RadxVol &vol,
			   const std::vector<std::string> &outputKeep,
			   bool outputAll)
{
  RayData rdata(&vol, _lookup);

  // do the volume commands first
  P.processVolume(&rdata);

  // then the loop commands, 1d
  for (int ii=0; ii < rdata.numRays(); ++ii)
  {
    P.processOneItem1d(&rdata, ii);
  }

  // then the loop commands, 2d
  for (int ii=0; ii < rdata.numSweeps(); ++ii)
  {
    P.processOneItem2d(&rdata, ii);
  }

  // do the final volume commands
  P.processVolumeAfter(&rdata);


  rdata.trim(outputKeep, outputAll);
}

//-----------------------------------------------------------------------
void _initializeInputs(const MathParser &P, const Params &parm, 
		       const std::vector<std::string> &fixedConst,
		       RadxApp &_alg)
{
  vector<string> input = P.identifyInputs();
  // remove the fixed constants from inputs}
  vector<string>::iterator i;
  for (i=input.begin(); i!=input.end(); )
  {
    if (find(fixedConst.begin(), fixedConst.end(), *i) != fixedConst.end())
    {
      i = input.erase(i);
    }
    else
    {
      i++;
    }
  }

  if (!_alg.init(cleanup, out_of_store, input))
  {
    exit(-1);
  }
}


//-----------------------------------------------------------------------
bool _matches_fixed_const(const Params &parm,
			  const std::string &s)
{
  for (int j=0; j<parm.fixed_const_n; ++j)
  {
    if (parm._fixed_const[j] == s)
    {
      return true;
    }
  }
  return false;
}
