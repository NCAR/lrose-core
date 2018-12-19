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
  for (size_t i=0; i<p.numInputs(); ++i)
  {
    if (!_isInput(p.ithInputRef(i)))
    {
      LOG(ERROR) << "Input to algorithm not configured in driver " 
		 << p.ithInputRef(i);
      ok = false;
    }
  }

  // make sure each parameterized output is actually an output of some
  // processing step
  for (size_t i=0; i<_virtvol_outputs.size(); ++i)
  {
    // one output for each URL
    vector<string> names = _virtvol_outputs[i].internalFieldNames();
    for (size_t j=0; j<names.size(); ++j)
    {
      if (!p.isOutput(names[j]))
      {
	LOG(ERROR) << "Output in params is not an output from the algorithm "
		   << names[j];
	ok = false;
      }
    }
  }

  // make sure each parameterized input is actually an input 
  for (size_t i=0; i<_virtvol_inputs.size(); ++i)
  {
    vector<string> names = _virtvol_inputs[i].internalFieldNames();
    for (size_t j=0; j<names.size(); ++j)
    {
      if (!p.isInput(names[j]))
      {
	LOG(ERROR) << "Input in params is not an input to the algorithm "
		   << names[j];
	ok = false;
      }
    }
  }
  return ok;
}

//------------------------------------------------------------------
bool VirtVolParms::inputExternal2InternalName(const std::string externalName,
					      std::string &internalName) const
{
  for (size_t i=0; i<_virtvol_inputs.size(); ++i)
  {
    if (_virtvol_inputs[i].external2Internal(externalName, internalName))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool VirtVolParms::outputInternal2ExternalName(const std::string internalName,
					       std::string &externalName) const
{
  for (size_t i=0; i<_virtvol_outputs.size(); ++i)
  {
    if (_virtvol_outputs[i].internal2External(internalName, externalName))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
std::string
VirtVolParms::matchingOutputUrl(const std::string &internalFieldName) const
{
  for (size_t i=0; i<_virtvol_outputs.size(); ++i)
  {
    if (_virtvol_outputs[i].internalNameMatch(internalFieldName))
    {
      return _virtvol_outputs[i]._url;
    }
  }
  return "";
}

//------------------------------------------------------------------
std::string VirtVolParms::sprintUrl(VirtVolParams::Url_t t)
{
  string s = "";
  switch (t)
  {
  case VirtVolParams::VIRTUAL_VOLUME:
    s = "VIRTUAL_VOLUME";
    break;
  case VirtVolParams::DATABASE:
    s = "DATABASE";
    break;
  case VirtVolParams::ASCII:
    s = "ASCII";
    break;
  default:
    s = "UNKNOWN";
    break;
  }
  return s;
}

//------------------------------------------------------------------
std::string VirtVolParms::sprintData(VirtVolParams::Data_t t)
{
  string s = "";
  switch (t)
  {
  case VirtVolParams::GRID:
    s = "GRID";
    break;
  case VirtVolParams::VALUE:
    s = "VALUE";
    break;
  case VirtVolParams::NOT_SET:
  default:
    s = "NOT_SET";
    break;
  }
  return s;
}

//------------------------------------------------------------------
bool VirtVolParms::_init(void)
{
  _ok = true;
  _virtvol_inputs.clear();
  _virtvol_outputs.clear();

  for (int i=0; i<virtvol_input_n; ++i)
  {
    string url = _virtvol_input[i].url;
    bool good = false;
    for (size_t j=0; j<_virtvol_inputs.size(); ++j)
    {
      if (_virtvol_inputs[j].urlEquals(url))
      {
	if (!_virtvol_inputs[j].add(DataSpec(_virtvol_input[i])))
	{
	  _ok = false;
	}
	good = true;
	break;
      }
    }
    if (!good)
    {
      UrlSpec u(_virtvol_input[i]);
      if (!u.add(_virtvol_input[i]))
      {
	_ok = false;
      }
      else
      {
	_virtvol_inputs.push_back(u);
      }
    }		        
  }
  for (int i=0; i<virtvol_output_n; ++i)
  {
    string url = _virtvol_output[i].url;
    bool good = false;
    for (size_t j=0; j<_virtvol_outputs.size(); ++j)
    {
      if (_virtvol_outputs[j].urlEquals(url))
      {
	if (!_virtvol_outputs[j].add(DataSpec(_virtvol_output[i])))
	{
	  _ok = false;
	}
	good = true;
	break;
      }
    }
    if (!good)
    {
      UrlSpec u(_virtvol_output[i]);
      if (!u.add(DataSpec(_virtvol_output[i])))
      {
	_ok = false;
      }
      else
      {
	_virtvol_outputs.push_back(u);
      }
    }		        
  }
  return _ok;
}

//------------------------------------------------------------------
bool VirtVolParms::_isInput(const std::string &internalName) const
{
  for (size_t i=0; i<_virtvol_inputs.size(); ++i)
  {
    if (_virtvol_inputs[i].internalNameMatch(internalName))
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool VirtVolParms::_isOutput(const std::string &internalName) const
{
  for (size_t i=0; i<_virtvol_outputs.size(); ++i)
  {
    if (_virtvol_outputs[i].internalNameMatch(internalName))
    {
      return true;
    }
  }
  return false;
}

