/**
 * @file VirtVolParms.cc
 */
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <FiltAlgVirtVol/Algorithm.hh>
#include <toolsa/LogStream.hh>
using std::vector;
using std::string;
using std::pair;

//------------------------------------------------------------------
VirtVolParms::VirtVolParms() : VirtVolParams(), _ok(false)
{
}

//------------------------------------------------------------------
VirtVolParms::VirtVolParms(const VirtVolParams &P) : VirtVolParams(P), _ok(true)
{
  _init();
}

//------------------------------------------------------------------
VirtVolParms::~VirtVolParms()
{
}

//------------------------------------------------------------------
void VirtVolParms::printParams(tdrp_print_mode_t mode)
{
  VirtVolParams::print(stdout, mode);
}  

//------------------------------------------------------------------
bool VirtVolParms::checkConsistency(const Algorithm &p) const
{
  bool ok = true;

  // make sure each parameterized input is actually an input to a
  // processing step
  int num = (int)p.numInputs();
  int numGood=0;
  for (size_t i=0; i<p.numInputs(); ++i)
  {
    if (!_isInput(p.ithInputRef(i)))
    {
      LOG(ERROR) << "Input to algorithm not configured in driver " 
		 << p.ithInputRef(i);
      ok = false;
    }
    else
    {
      LOG(DEBUG) << "Input to algorithm is in Url params " <<  p.ithInputRef(i);
      ++numGood;
    }
  }
  LOG(DEBUG) << numGood << " of " << num << " algorithm inputs found in Url params";

  // make sure each parameterized output is actually an output of some
  // processing step
  num = 0;
  numGood = 0;
  for (size_t i=0; i<_outputUrl.size(); ++i)
  {
    // one output for each URL
    vector<string> names = _outputUrl[i].getNames();
    for (size_t j=0; j<names.size(); ++j)
    {
      ++num;
      if (!p.isOutput(names[j]))
      {
	LOG(ERROR) << "Output in params is not an output from the algorithm "
		   << names[j];
	ok = false;
      }
      else
      {
	++numGood;
	LOG(DEBUG) << "Url param output is an algorithm output " << names[j];
      }
    }
  }
  LOG(DEBUG) << numGood << " of " << num << " URL outputs found in algorithm";

  // make sure each parameterized input is actually an input 
  num = 0;
  numGood = 0;
  for (size_t i=0; i<_inputUrl.size(); ++i)
  {
    vector<string> names = _inputUrl[i].getNames();
    for (size_t j=0; j<names.size(); ++j)
    {
      ++num;
      if (!p.isInput(names[j]))
      {
	LOG(ERROR) << "Input in params is not an input to the algorithm "
		   << names[j];
	ok = false;
      }
      else
      {
	LOG(DEBUG) << "Url param input is an algorithm input " << names[j];
	++numGood;
      }
    }
  }
  LOG(DEBUG) << numGood << " of " << num << " URL inputss found in algorithm";
  return ok;
}

//------------------------------------------------------------------
std::string
VirtVolParms::matchingOutputUrl(const std::string &fieldName) const
{
  for (size_t i=0; i<_outputUrl.size(); ++i)
  {
    if (_outputUrl[i].nameMatch(fieldName))
    {
      return _outputUrl[i].url;
    }
  }
  return "";
}

//------------------------------------------------------------------
bool VirtVolParms::hasOutputField(const std::string &fieldName) const
{
  for (size_t i=0; i<_outputUrl.size(); ++i)
  {
    if (_outputUrl[i].nameMatch(fieldName))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool VirtVolParms::_init(void)
{
  _ok = true;

  // read in the input and output url param files
  for (int i=0; i<input_url_n; ++i)
  {
    UrlParms u(_input_url[i]);
    if (u.isOk())
    {
      _inputUrl.push_back(u);
    }
    else
    {
      _ok = false;
    }
  }
  // read in the input and output url param files
  for (int i=0; i<output_url_n; ++i)
  {
    UrlParms u(_output_url[i]);
    if (u.isOk())
    {
      _outputUrl.push_back(u);
    }
    else
    {
      _ok = false;
    }
  }
  
  return _ok;
}

//------------------------------------------------------------------
bool VirtVolParms::_isInput(const std::string &name) const
{
  for (size_t i=0; i<_inputUrl.size(); ++i)
  {
    if (_inputUrl[i].nameMatch(name))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool VirtVolParms::_isOutput(const std::string &name) const
{
  for (size_t i=0; i<_outputUrl.size(); ++i)
  {
    if (_outputUrl[i].nameMatch(name))
    {
      return true;
    }
  }
  return false;
}
