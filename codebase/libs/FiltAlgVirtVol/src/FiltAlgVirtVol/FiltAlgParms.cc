/**
 * @file FiltAlgParms.cc
 */
#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <toolsa/LogStream.hh>
using std::vector;
using std::string;
using std::pair;

//------------------------------------------------------------------
FiltAlgParms::FiltAlgParms() : AlgorithmParms(), VirtVolParms(), _ok(false)
{
}

//------------------------------------------------------------------
FiltAlgParms::FiltAlgParms(const std::string &fileName, bool expandEnv) :
  AlgorithmParms(), VirtVolParms(), _ok(true)
{
  TDRP_warn_if_extra_params(FALSE);

  AlgorithmParams a;
  char **x = NULL;
  int envs;
  if (expandEnv)
  {
    envs = 1;
  }
  else
  {
    envs = 0;
  }
  if (a.load(fileName.c_str(), x, envs, false) == 1)
  {
    LOG(ERROR) << "LOading algorithm params from " << fileName;
    _ok = false;
  }
  AlgorithmParms::set(a);
  
  VirtVolParams v;
  if (v.load(fileName.c_str(), x, envs, false) == 1)
  {
    LOG(ERROR) << "LOading virtvol params from " << fileName;
    _ok = false;
  }
  VirtVolParms::set(v);

  TDRP_warn_if_extra_params(TRUE);
}

//------------------------------------------------------------------
FiltAlgParms::~FiltAlgParms()
{
}

//------------------------------------------------------------------
bool FiltAlgParms::isPrintParams(int argc, char **argv,
				 tdrp_print_mode_t &printMode,
				 int &expandEnv)
{
  for (int i=0; i<argc; ++i)
  {
    string s = argv[i];
    if (s == "-print_params")
    {
      printMode = PRINT_LONG;
      expandEnv = 0;
      return true;
    }
    else if (s == "--print_params")
    {
      if (i >= argc-1)
      {
	LOG(ERROR) << "Need a mode param with --print_params option";
	return false;
      }
      string mode = argv[i+1];
      if (mode == "short")
      {
	printMode = PRINT_SHORT;
	expandEnv = 0;
      }
      else if (mode == "norm")
      {
	printMode = PRINT_NORM;
	expandEnv = 0;
      }
      else if (mode == "long")
      {
	printMode = PRINT_LONG;
	expandEnv = 0;
      }
      else if (mode == "verbose")
      {
	printMode = PRINT_VERBOSE;
	expandEnv = 0;
      }
      else if (mode == "shortexpandEnv")
      {
	printMode = PRINT_SHORT;
	expandEnv = 1;
      }
      else if (mode == "normexpandEnv")
      {
	printMode = PRINT_NORM;
	expandEnv = 1;
      }
      else if (mode == "longexpandEnv")
      {
	printMode = PRINT_LONG;
	expandEnv = 1;
      }
      else if (mode == "verboseexpandEnv")
      {
	printMode = PRINT_VERBOSE;
	expandEnv = 1;
      }
      else
      {
	LOG(ERROR) << "Unknown print mode " << mode;
	printMode = NO_PRINT;
	expandEnv = 0;
      }
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool FiltAlgParms::isPrintOperators(int argc, char **argv)
{
  for (int i=0; i<argc; ++i)
  {
    string s = argv[i];
    if (s == "-print_operators")
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
void FiltAlgParms::printHelp(void)
{
  std::cout << "FiltAlgParms options:\n"
	    << " [-print_operators] Print out all binary and unary operators\n"
	    << " [-interval yyyymmddhhmmss yyyymmddhhmmss] Archive mode\n";
  AlgorithmParams::usage(std::cout);
}

//------------------------------------------------------------------
void FiltAlgParms::printParams(tdrp_print_mode_t mode)
{
  AlgorithmParms::printParams(mode);
  VirtVolParms::printParams(mode);
}

//------------------------------------------------------------------
bool FiltAlgParms::isSetParams(int argc, char **argv, std::string &fileName)
{
  for (int i=0; i<argc; ++i)
  {
    string s = argv[i];
    if (s == "-params")
    {
      if (i < argc-1)
      {
	fileName = argv[i+1];
	return true;
      }
      else
      {
	LOG(ERROR) << "Bad arg list";
	return false;
      }
    }
  }
  return false;
}

//------------------------------------------------------------------
bool FiltAlgParms::isHelp(int argc, char **argv)
{
  for (int i=0; i<argc; ++i)
  {
    string s = argv[i];
    if (s == "--" || s == "-h" || s == "-help" || s == "-man")
    {
      return true;
    }
  }
  return false;
}

