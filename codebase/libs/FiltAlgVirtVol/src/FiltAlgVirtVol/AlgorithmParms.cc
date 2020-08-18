/**
 * @file AlgorithmParms.cc
 */
#include <FiltAlgVirtVol/AlgorithmParms.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>
using std::vector;
using std::string;
using std::pair;

//-------------------------------------------------------------------------------
static void _findAndReplaceAll(std::string & data, const std::string &toSearch,
			       const std::string &replaceStr)
 {
   // Get the first occurrence
   size_t pos = data.find(toSearch);
   // Repeat till end is reached
   while( pos != std::string::npos)
   {
     // Replace this occurrence of Sub String
     data.replace(pos, toSearch.size(), replaceStr);
     // Get the next occurrence from the current position
     pos =data.find(toSearch, pos + replaceStr.size());
   }
 }

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

//-----------------------------------------------------------------------
bool AlgorithmParms::matchesFixedConst(const std::string &s) const
{
  for (size_t j=0; j<_fixedConstants.size(); ++j)
  {
    if (_fixedConstants[j].first == s)
    {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------
void  AlgorithmParms::substituteFixedConst(void)
{
  for (size_t i=0; i<_volumeBeforeFilters.size(); ++i)
  {
    _subsituteFixedConst(_volumeBeforeFilters[i]);
  }
  for (size_t i=0; i<_sweepFilters.size(); ++i)
  {
    _subsituteFixedConst(_sweepFilters[i]);
  }
  for (size_t i=0; i<_volumeAfterFilters.size(); ++i)
  {
    _subsituteFixedConst(_volumeAfterFilters[i]);
  }
}

//-----------------------------------------------------------------------
bool AlgorithmParms::addFixedConstant(const std::string &item)
{
  // remove all whitespace first
  string loc(item);
  loc.erase(remove(loc.begin(), loc.end(), ' '), loc.end());
  loc.erase(remove(loc.begin(), loc.end(), '\t'), loc.end());

  // now look for =
  std::size_t found = loc.find_first_of("=");
  if (found == std::string::npos)
  {
    LOG(ERROR) << "fixed constant not of format const=value, got " << loc;
    return false;
  }
  string variable = loc.substr(0, found);
  string value = loc.substr(found+1);
  _fixedConstants.push_back(pair<string,string>(variable, value));
  _fixedConstantNames.push_back(variable);
  LOG(DEBUG) << "Added fixed constant " << variable << " with value " << value;
  return true;
}

//-----------------------------------------------------------------------
void AlgorithmParms::_subsituteFixedConst(std::string &filterStr)
{
  for (size_t i=0; i<_fixedConstants.size(); ++i)
  {
    _findAndReplaceAll(filterStr, _fixedConstants[i].first,
		       _fixedConstants[i].second);
  }
}


