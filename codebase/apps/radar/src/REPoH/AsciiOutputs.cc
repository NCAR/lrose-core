/**
 * @file AsciiOutputs.cc
 */
#include "AsciiOutputs.hh"
#include "RepohParams.hh"
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <toolsa/LogStream.hh>

AsciiOutputs::AsciiOutputs()
{
}
AsciiOutputs::~AsciiOutputs()
{
}

void AsciiOutputs::clear(void)
{
  _clear();
}

bool AsciiOutputs::initialize(const Parms &parms, const time_t &t)
{	
  _clear();

  for (int i=0; i<parms.ascii_output_n; ++i)
  {
    if (!_setupAsciiOutput(parms._ascii_output[i], parms, t))
    {
      return false;
    }
  }

  for (size_t i=0; i<_asciiOutput.size(); ++i)
  {
    // in case of rerun of this time
    _asciiOutput[i]->clear();
  }
  return true;
}

//------------------------------------------------------------------
AsciiOutput *AsciiOutputs::refToAsciiOutput(const std::string &name,
					    bool suppressWarn)
{
  for (size_t i=0; i<_asciiOutput.size(); ++i)
  {
    if (_asciiOutput[i]->nameMatch(name))
    {
      return _asciiOutput[i];
    }
  }
  if (!suppressWarn)
  {
    printf("ERROR retrieving ascii output data for %s\n", name.c_str());
  }
  return NULL;
}


void AsciiOutputs::_clear(void)
{
  for (size_t i=0; i<_asciiOutput.size(); ++i)
  {
    delete _asciiOutput[i];
  }
  _asciiOutput.clear();
}

bool AsciiOutputs::_setupAsciiOutput(const RepohParams::Ascii_output_t &p,
				     const VirtVolParms &vparms,
				     const time_t &t)
{
  for (size_t j=0; j<vparms._virtvol_outputs.size(); ++j)
  {
    if (vparms._virtvol_outputs[j].internalNameMatch(p.name))
    {
      if (vparms._virtvol_outputs[j]._type != VirtVolParams::ASCII)
      {
	LOG(ERROR) << "Inconsistent use of kernel ascii output data";
	return false;
      }
      _asciiOutput.push_back(new AsciiOutput(p.name, p.path, t));
      return true;
    }
  }
  LOG(ERROR) << "Did not find a match for " << p.name;
  return false;
}
