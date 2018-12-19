/**
 * @file AlgorithmParms.cc
 */
#include <FiltAlgVirtVol/AlgorithmParms.hh>
#include <toolsa/LogStream.hh>
using std::vector;
using std::string;
using std::pair;

//------------------------------------------------------------------
AlgorithmParms::AlgorithmParms() : AlgorithmParams(), _ok(false)
{
}

//------------------------------------------------------------------
AlgorithmParms::AlgorithmParms(const AlgorithmParams &P) : 
  AlgorithmParams(P), _ok(true)
{
}

//------------------------------------------------------------------
AlgorithmParms::~AlgorithmParms()
{
}

//------------------------------------------------------------------
void AlgorithmParms::printParams(tdrp_print_mode_t mode)
{
  AlgorithmParams::print(stdout, mode);
}  

//------------------------------------------------------------------
void AlgorithmParms::set(const AlgorithmParams &a)
{
  *((AlgorithmParams *)this) = a;
}

//------------------------------------------------------------------
bool AlgorithmParms::isOutput(const std::string &name) const
{
  for (int i=0; i<output_n; ++i)
  {
    if (_output[i] == name)
    {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------
bool AlgorithmParms::isInput(const std::string &name) const
{
  for (int i=0; i<input_n; ++i)
  {
    if (_input[i] == name)
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------
bool AlgorithmParms::matchesFixedConst(const std::string &s) const
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

