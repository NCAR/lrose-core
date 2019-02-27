/**
 * @file RadxAppParms.cc
 */
#include <radar/RadxAppParms.hh>
#include <toolsa/LogStream.hh>
#include <vector>
#include <string>
using std::vector;
using std::string;
using std::pair;

//------------------------------------------------------------------
RadxAppParms::RadxAppParms() : RadxAppParams(), _inputs(), 
			       _isFileList(false), _outputAllFields(false),
			       _ok(false)
{
}

//------------------------------------------------------------------
RadxAppParms::RadxAppParms(const std::string &fileName, bool expandEnv) :
  RadxAppParams(), _inputs(), _isFileList(false), _outputAllFields(false),
  _ok(true)
{
  TDRP_warn_if_extra_params(FALSE);
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
  if (RadxAppParams::load(fileName.c_str(), x, envs, false) == 1)
  {
    LOG(ERROR) << "Loading RadxApp params from " << fileName;
    _ok = false;
  }
  TDRP_warn_if_extra_params(TRUE);

  _inputs = RadxAppConfig(*this);
  if (_inputs._primaryGroup.isClimo)
  {
    LOG(ERROR) << "The primary path cannot be climo data";
    exit(-1);
  }

  _outputAllFields = output_all_fields;
  for (int i=0; i<output_fields_n; ++i)
  {
    _outputFieldList.push_back(_output_fields[i]);
  }
}

//------------------------------------------------------------------
RadxAppParms::RadxAppParms(const std::string &fileName, 
			   const std::vector<std::string> &fileList,
			   bool expandEnv) :
  RadxAppParams(), _inputs(), _isFileList(true), _fileList(fileList),
  _ok(true)
{
  TDRP_warn_if_extra_params(FALSE);
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
  if (RadxAppParams::load(fileName.c_str(), x, envs, false) == 1)
  {
    LOG(ERROR) << "Loading RadxApp params from " << fileName;
    _ok = false;
  }
  TDRP_warn_if_extra_params(TRUE);

  _inputs = RadxAppConfig(*this);
  if (_inputs._primaryGroup.isClimo)
  {
    LOG(ERROR) << "The primary path cannot be climo data";
    exit(-1);
  }
  _outputAllFields = output_all_fields;
  for (int i=0; i<output_fields_n; ++i)
  {
    _outputFieldList.push_back(_output_fields[i]);
  }
}

//------------------------------------------------------------------
RadxAppParms::~RadxAppParms()
{
}

//------------------------------------------------------------------
bool RadxAppParms::isPrintOperators(int argc, char **argv)
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
bool RadxAppParms::isHelp(int argc, char **argv)
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

//------------------------------------------------------------------
bool RadxAppParms::isPrintParams(int argc, char **argv,
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
void RadxAppParms::printParams(tdrp_print_mode_t mode)
{
  RadxAppParams::print(stdout, mode);
}

//------------------------------------------------------------------
void RadxAppParms::printHelp(void)
{
  std::cout << "RadxAppParms options:\n"
	    << " [-print_operators] Print out all binary and unary operators\n"
	    << " [-interval yyyymmddhhmmss yyyymmddhhmmss] Archive mode\n";

  RadxAppParams::usage(std::cout);
}

//------------------------------------------------------------------
bool RadxAppParms::isSetParams(int argc, char **argv, std::string &fileName)
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
bool RadxAppParms::isFileList(int argc, char **argv,
			      std::vector<std::string> &files)
{
  files.clear();
  for (int i=0; i<argc; ++i)
  {
    string s = argv[i];
    if (s == "-f" || s == "-path")
    {
      if (i < argc-1)
      {
	for (int j=i+1; j<argc; ++j)
	{
	  files.push_back(argv[j]);
	}
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

//-----------------------------------------------------------------------
bool RadxAppParms::matchesFixedConst(const std::string &s) const
{
  for (size_t j=0; j<_fixedConstants.size(); ++j)
  {
    if (_fixedConstants[j] == s)
    {
      return true;
    }
  }
  return false;
}

