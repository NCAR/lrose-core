/**
 * @file MathParser.cc
 */
#include <rapmath/MathParser.hh>
#include <rapmath/MathLoopData.hh>
#include <rapmath/MathData.hh>
#include <rapmath/MathUserData.hh>
#include <rapmath/Find.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/LogicalArgs.hh>
#include <toolsa/TaThreadQue.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/pmu.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <algorithm>
using std::string;
using std::vector;

//---------------------------------------------------------------------
static void _simpleLogicalMultiplesAssignNumberToVar(const Filter &filter,
						     MathData *rdata)
{
  // pull the pairs of comparison values and numbers (or missing), the and/ors,
  // and the assignment variable and that number (or missing) out of the filter
  string assignName;
  double assignV;
  bool assignMissing;
  LogicalArgs args;
  if (!filter._filter->getMultiCompare(args, assignName, assignV,assignMissing))
  {				       
    LOG(ERROR) << "Could not get data";
    return;
  }

  MathLoopData *assignD = rdata->dataPtr(assignName);
  if (assignD == NULL)
  {
    LOG(ERROR) << "No data for " << assignName;
    return;
  }
  if (assignMissing)
  {
    assignV = assignD->getMissingValue();
  }

  for (size_t i=0; i<args.numArgs(); ++i)
  {
    if (!args[i].synch(rdata))
    {
      return;
    }
  }

  // figure out status left to right
  for (int i=0; i< assignD->numData(); ++i)
  {
    bool ret = args[0].satisfiesCondition(i);
    for (size_t j=1; j<args.numArgs(); ++j)
    {
      bool bj = args[j].satisfiesCondition(i);
      args.updateStatus(bj, j-1, ret);
    }
    if (ret)
    {
      assignD->setVal(i, assignV);
    }
  }
}

//---------------------------------------------------------------------
static void _simpleLogicalMultiplesAssignVarToVar(const Filter &filter,
						  MathData *rdata)
{
  // pull the pairs of comparison values and numbers (or missing), the and/ors,
  // and the assignment variable and that number (or missing) out of the filter
  string assignToName, assignFromName;
  LogicalArgs args;
  if (!filter._filter->getMultiCompare(args, assignToName, assignFromName))
  {				       
    LOG(ERROR) << "Could not get data";
    return;
  }
  MathLoopData *assignToD = rdata->dataPtr(assignToName);
  MathLoopData *assignFromD = rdata->dataPtr(assignFromName);
  if (assignToD == NULL || assignFromD == NULL)
  {
    LOG(ERROR) << "No data for an input " 
	       << assignToName << " " << assignFromName;
    return;
  }

  for (size_t i=0; i<args.numArgs(); ++i)
  {
    if (!args[i].synch(rdata))
    {
      return;
    }
  }

  // figure out status left to right
  for (int i=0; i< assignToD->numData(); ++i)
  {
    bool ret = args[0].satisfiesCondition(i);
    for (size_t j=1; j<args.numArgs(); ++j)
    {
      bool bj = args[j].satisfiesCondition(i);
      args.updateStatus(bj, j-1, ret);
    }
    if (ret)
    {
      double v2;
      if (assignFromD->getVal(i, v2))
      {
	assignToD->setVal(i, v2);
      }
      else
      {
	assignToD->setMissing(i);
      }
    }
  }
}

//---------------------------------------------------------------------
static void _simpleLogicalAssignNumberToVar(const Filter &filter,
					    MathData *rdata)
{
  // pull the comparison value and number (or missing), and the assignment
  // variable and that number (or missing) out of the filter
  string compareName, assignName;
  double compareV, assignV;
  bool compareMissing, assignMissing;
  MathFindSimple::Compare_t c;
  
  if (!filter._filter->getSimpleCompare(compareName, compareV, compareMissing,
					c, assignName, assignV, assignMissing))
  {				       
    LOG(ERROR) << "Could not get data";
    return;
  }

  MathLoopData *compareD = rdata->dataPtr(compareName);
  MathLoopData *assignD = rdata->dataPtr(assignName);
  if (compareD == NULL || assignD == NULL)
  {
    LOG(ERROR) << "No data for inputs " << compareName << " " << assignName;
    return;
  }
  if (compareMissing)
  {
    compareV = compareD->getMissingValue();
    if (c != MathFindSimple::EQ)
    {
      LOG(ERROR) << "Only equality for missing comparison";
      return;
    }
  }
  if (assignMissing)
  {
    assignV = assignD->getMissingValue();
  }
  switch (c)
  {
  case MathFindSimple::GT:
    for (int i=0; i<assignD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v > compareV)
	{
	  assignD->setVal(i, assignV);
	}
      }
    }
    break;
  case MathFindSimple::GE:
    for (int i=0; i<assignD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v >= compareV)
	{
	  assignD->setVal(i, assignV);
	}
      }
    }
    break;
  case MathFindSimple::EQ:
    for (int i=0; i<assignD->numData(); ++i)
    {
      double v;
      if (!compareD->getVal(i, v) != compareMissing)
      {
	assignD->setVal(i, assignV);
      }
    }
    break;
  case MathFindSimple::LE:
    for (int i=0; i<assignD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v <= compareV)
	{
	  assignD->setVal(i, assignV);
	}
      }
    }
    break;
  case MathFindSimple::LT:
    for (int i=0; i<assignD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v < compareV)
	{
	  assignD->setVal(i, assignV);
	}
      }
    }
    break;
  default:
    break;
  }    
}

//-----------------------------------------------------------------------
static void _simpleLogicalAssignVarToVar(const Filter &filter,
					 MathData *rdata)
{
  // pull the comparison value and number (or missing), and the assignment
  // variable and the copy from variable out of the filter
  string compareName, assignToName, assignFromName;
  double compareV;
  bool compareMissing;
  MathFindSimple::Compare_t c;
  
  if (!filter._filter->getSimpleCompare(compareName, compareV, compareMissing,
					c, assignToName, assignFromName))
  {				       
    LOG(ERROR) << "Could not get data";
    return;
  }

  MathLoopData *compareD = rdata->dataPtr(compareName);
  MathLoopData *assignToD = rdata->dataPtr(assignToName);
  MathLoopData *assignFromD = rdata->dataPtr(assignFromName);
  if (compareD == NULL || assignToD == NULL || assignFromD == NULL)
  {
    LOG(ERROR) << "No data for an input " << compareName << " "
	       << assignToName << " " << assignFromName;
    return;
  }
  if (compareMissing)
  {
    compareV = compareD->getMissingValue();
    if (c != MathFindSimple::EQ)
    {
      LOG(ERROR) << "Only equality for missing comparison";
      return;
    }
  }
  switch (c)
  {
  case MathFindSimple::GT:
    for (int i=0; i<assignToD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v > compareV)
	{
	  double v2;
	  if (assignFromD->getVal(i, v2))
	  {
	    assignToD->setVal(i, v2);
	  }
	  else
	  {
	    assignToD->setMissing(i);
	  }
	}
      }
    }
    break;
  case MathFindSimple::GE:
    for (int i=0; i<assignToD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v >= compareV)
	{
	  double v2;
	  if (assignFromD->getVal(i, v2))
	  {
	    assignToD->setVal(i, v2);
	  }
	  else
	  {
	    assignToD->setMissing(i);
	  }
	}
      }
    }
    break;
  case MathFindSimple::EQ:
    for (int i=0; i<assignToD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v) != compareMissing)
      {
	double v2;
	if (assignFromD->getVal(i, v2))
	{
	  assignToD->setVal(i, v2);
	}
	else
	{
	  assignToD->setMissing(i);
	}
      }
    }
    break;
  case MathFindSimple::LE:
    for (int i=0; i<assignToD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v <= compareV)
	{
	  double v2;
	  if (assignFromD->getVal(i, v2))
	  {
	    assignToD->setVal(i, v2);
	  }
	  else
	  {
	    assignToD->setMissing(i);
	  }
	}
      }
    }
    break;
  case MathFindSimple::LT:
    for (int i=0; i<assignToD->numData(); ++i)
    {
      double v;
      if (compareD->getVal(i, v))
      {
	if (v < compareV)
	{
	  double v2;
	  if (assignFromD->getVal(i, v2))
	  {
	    assignToD->setVal(i, v2);
	  }
	  else
	  {
	    assignToD->setMissing(i);
	  }
	}
      }
    }
    break;
  default:
    break;
  }    
}

//---------------------------------------------------------------------
static void _simpleAssignNumberToVar(const Filter &filter, MathData *rdata,
				     bool missing)
{
  // pull the number and the var name out of filter
  string name;
  double number;
  bool lmissing;
  if (!filter._filter->getSimpleAssign(name, number, lmissing))
  {
    LOG(ERROR) << "Couldn't get the data";
    return;
  }
  if (missing != lmissing)
  {
    LOG(ERROR) << "Mismatch in missing status";
    return;
  }

  // want to assign either missing or a number to the entire input named data
  MathLoopData *data =  rdata->dataPtr(name);
  if (data == NULL)
  {
    return;
  }
  if (missing)
  {
    number = data->getMissingValue();
  }
  
  for (int i=0; i<data->numData(); ++i)
  {
    data->setVal(i, number);
  }
}

//--------------------------------------------------------------------------
static void _simpleAssignVarToVar(const Filter &filter, MathData *rdata)
{
  // pull the 'to' and 'from' vars out of filter
  string from, to;
  if (!filter._filter->getSimpleAssign(from, to))
  {
    LOG(ERROR) << "Couldn't get the data";
    return;
  }

  // want to assign either missing or a number to the entire input named data
  MathLoopData *fromData =  rdata->dataPtr(from);
  MathLoopData *toData =  rdata->dataPtr(to);
  if (fromData == NULL || toData == NULL)
  {
    LOG(ERROR) << " NO data for " << from << " " << to;
    return;
  }
  
  for (int i=0; i<toData->numData(); ++i)
  {
    double v;
    if (fromData->getVal(i, v))
    {
      toData->setVal(i, v);
    }
    else
    {
      toData->setMissing(i);
    }
  }
}

//--------------------------------------------------------------------
static void _augmentInputs(const ProcessingNode &p,
			   const std::vector<std::string> &outputs,
			   std::vector<std::string> &inputs)
{
  vector<string> name;
  p.inputFields(name);
  for (size_t j=0; j<name.size(); ++j)
  {
    if (find(outputs.begin(), outputs.end(), name[j]) == outputs.end())
    {
      if (find(inputs.begin(), inputs.end(), name[j]) == inputs.end())
      {
	inputs.push_back(name[j]);
      }
    }
  }
}    

//--------------------------------------------------------------------
static void _augmentOutputs(const ProcessingNode &p, vector<string> &outputs)
{
  vector<string> name;
  p.outputFields(name);
  if (name.size() > 1)
  {
    LOG(FATAL) << "Too many outputs for filter " << p.sprint();
    exit(-1);
  }
  if (name.empty())
  {
    LOG(ERROR) << "no output for filter " << p.sprint();
  }
  if (find(outputs.begin(), outputs.end(), name[0]) == outputs.end())
  {
    outputs.push_back(name[0]);
  }
}
//-------------------------------------------------------------------
/**
 * Make sure '-' is used as an operator by checking context.
 * return false if it is part of a negative number.
 */
static bool _isOperator(const string &s, std::size_t p)
{
  if (s[p] == '-')
  {
    if (p == 0)
    {
      // unary - at beginning is not an operator
      return false;
    }
    else if (p > 0)
    {
      // check for something like 2*-3  or 2*(-3)
      string prev = s.substr(0, p-1);
      std::size_t l = prev.find_last_of("(+-*/^");
      if (l == p-1)
      {
	return false;
      }
    }
  }
  return true;
}

//-------------------------------------------------------------------
/**
 * @return number of occurances of a pattern in a string
 */
static int _num(const std::string &s, const std::string &pattern)
{
  std::size_t found, patSize=pattern.size();
  std::string loc(s);
  int count = 0;
  while ((found = loc.find(pattern)) != std::string::npos)
  {
    ++count;
    loc = loc.substr(found+patSize);
  }
  return count;
}

//-------------------------------------------------------------------
/**
 * return the last occurance of a particular operator string that is outside
 * of parens, i.e. no surplus of left parens preceding the op location
 *
 * If no occurances of op, return string::npos
 * If all occurances of op are within a paren pair, return string::npos
 */
static std::size_t
_lastOutsideOfParens(const std::string &s, const std::string &op)
{
  std::size_t p;
  string s2 = s;
  while (true)
  {
    p = s2.rfind(op);
    if (p == string::npos)
    {
      return p;
    }
    string part1 = s2.substr(0, p);
    MathParser::trim(part1);
    string part2 = s2.substr(p + 1);
    MathParser::trim(part2);

    // count # of left and right parens in part1
    int numRight = _num(part1, ")");
    int numLeft = _num(part1, "(");
    if (numLeft == numRight)
    {
      // not inside parens at this point
      return p;
    }
    else if (numLeft > numRight)
    {
      // inside parens at operator...hold off for now
      s2 = s2.substr(0, p-1);
    }
    else
    {
      LOG(ERROR) << "mismatch in parens? " << part1;
      exit(-1);
    }
  }
}

//-------------------------------------------------------------------
/**
 * assume input is the argument list for a unary operator
 * look for 'arg,arg,arg,...,arg'  or just arg and return the args
 */
static std::vector<string> _commaSeparatedArgs(const std::string &s)
{
  std::vector<string> ret;
  if (s.empty())
    return ret;
  std::size_t p=0, p2=0;
  while (true)
  {  
    p2 = s.find(",", p);
    if (p2 == string::npos)
    {
      ret.push_back(s.substr(p));
      return ret;
    }
    else
    {
      ret.push_back(s.substr(p, p2-p));
      p = p2+1;
      if (p > s.size())
      {
	return ret;
      }
    }
  }
}

//-------------------------------------------------------------------
MathParser::MathParser(void) : _outputDebugAll(true)
{
  _unaryOperators = ProcessingNode::unaryOperators();
  _binaryOperators = ProcessingNode::binaryOperators();
}

//-------------------------------------------------------------------
MathParser::~MathParser(void)
{
}

//-------------------------------------------------------------------
std::vector<std::string> MathParser::identifyOutputs(void) const
{
  vector<string> nonInput;

  for (size_t i=0; i<_volFilters.size(); ++i)
  {
    _augmentOutputs(*(_volFilters[i]._filter), nonInput);
  }

  for (size_t i=0; i<_filters2d.size(); ++i)
  {
    _augmentOutputs(*(_filters2d[i]._filter), nonInput);
  }    

  for (size_t i=0; i<_filters1d.size(); ++i)
  {
    _augmentOutputs(*(_filters1d[i]._filter), nonInput);
  }    

  for (size_t i=0; i<_volFiltersAfter.size(); ++i)
  {
    _augmentOutputs(*(_volFiltersAfter[i]._filter), nonInput);
  }

  return nonInput;
}

//-------------------------------------------------------------------
std::vector<std::string> MathParser::identifyInputs(void) const
{
  vector<string> nonInput;
  vector<string> input;

  // look at all input/output field names, and build a list of pure
  // inputs (the non nonInputs).
  for (size_t i=0; i<_volFilters.size(); ++i)
  {
    _augmentOutputs(*(_volFilters[i]._filter), nonInput);
  }

  for (size_t i=0; i<_filters2d.size(); ++i)
  {
    _augmentOutputs(*(_filters2d[i]._filter), nonInput);
  }    

  for (size_t i=0; i<_filters1d.size(); ++i)
  {
    _augmentOutputs(*(_filters1d[i]._filter), nonInput);
  }    

  for (size_t i=0; i<_filters2d.size(); ++i)
  {
    _augmentInputs(*(_filters2d[i]._filter), nonInput, input);
  }
  for (size_t i=0; i<_filters1d.size(); ++i)
  {
    _augmentInputs(*(_filters1d[i]._filter), nonInput, input);
  }

  for (size_t i=0; i<_volFilters.size(); ++i)
  {
    _augmentInputs(*(_volFilters[i]._filter), nonInput, input);
  }
  return input;
}

//-------------------------------------------------------------------
bool MathParser::parse(const std::string &s, Filter_t filterType,
		       const std::vector<std::string> &fixedConstants,
		       const std::vector<std::string> &userDataNames)
{
  ProcessingNode *p = _parse(s);
  if (p == NULL)
  {
    return false;
  }
  Filter f;

  vector<string> inputs, outputs;

  p->inputFields(inputs);
  vector<string>::iterator i;
  for (i=inputs.begin(); i!= inputs.end();)
  {
    if (find(fixedConstants.begin(), fixedConstants.end(), *i) ==
	fixedConstants.end())
    {
      i++;
    }
    else
    {
      i = inputs.erase(i);
    }
  }
	
  p->outputFields(outputs);
  if (outputs.size() != 1)
  {
    LOG(ERROR) << "Design allows only one output '" << s << "'";
    return false;
  }
  f._filter = p;
  f._output = outputs[0];
  f._inputs = inputs;
  f._pattern = p->pattern();
  f._dataType = filterType;
  switch (filterType)
  {
  case VOLUME_BEFORE:
    if (find(userDataNames.begin(), userDataNames.end(), f._output) !=
	userDataNames.end())
    {
      f._dataType = VOLUME_BEFORE_USER;
    }
    _volFilters.push_back(f);
    break;
  case LOOP2D_TO_2D:
    if (find(userDataNames.begin(), userDataNames.end(), f._output) !=
	userDataNames.end())
    {
      f._dataType = LOOP2D_TO_USER_DEFINED;
    }
    _filters2d.push_back(f);
    break;
  case LOOP1D:
    _filters1d.push_back(f);
    break;
  case VOLUME_AFTER:
    _volFiltersAfter.push_back(f);
    break;
  case LOOP2D_TO_USER_DEFINED:
  case VOLUME_BEFORE_USER:
  default:
    LOG(ERROR) << "Bad input";
    return false;
  }
  return true;
}

//-------------------------------------------------------------------
std::vector<FunctionDef> MathParser::allFunctionDefs(void) const
{
  vector<FunctionDef> ret = _binaryOperators;
  for (size_t i=0; i<_unaryOperators.size(); ++i)
  {
    ret.push_back(_unaryOperators[i]);
  }
  for (size_t i=0; i<_userUnaryOperators.size(); ++i)
  {
    ret.push_back(_userUnaryOperators[i]);
  }
  for (size_t i=0; i<_userBinaryOperators.size(); ++i)
  {
    ret.push_back(_userBinaryOperators[i]);
  }
  sort(ret.begin(), ret.end());
  return ret;
}

//-------------------------------------------------------------------
void MathParser::cleanup(void)
{
  for (size_t i=0; i<_filters2d.size(); ++i)
  {
    _filters2d[i]._filter->cleanup();
    delete _filters2d[i]._filter;
  }
  for (size_t i=0; i<_filters1d.size(); ++i)
  {
    _filters1d[i]._filter->cleanup();
    delete _filters1d[i]._filter;
  }
  for (size_t i=0; i<_volFilters.size(); ++i)
  {
    _volFilters[i]._filter->cleanup();
    delete _volFilters[i]._filter;
  }
}

//-------------------------------------------------------------------
void MathParser::processVolume(VolumeData *rdata) const
{
  for (size_t i=0; i<_volFilters.size(); ++i)
  {
    _processV(_volFilters[i], rdata);
  }
}

//-------------------------------------------------------------------
void MathParser::processVolumeAfter(VolumeData *rdata) const
{
  for (size_t i=0; i<_volFiltersAfter.size(); ++i)
  {
    _processV(_volFiltersAfter[i], rdata);
  }
}

//-----------------------------------------------------------------------
void MathParser::processOneItem2d(VolumeData *rdata, int ii) const
{
  LOG(DEBUG_VERBOSE) << "Processing 2d item " << ii;

  // make a local copy (thinking ahead to threading)
  MathData *local = rdata->initializeProcessingNode(ii, true);
  for (size_t f = 0; f< _filters2d.size(); ++f)
  {
    _processLoop(_filters2d[f], local, ii==0 || _outputDebugAll);
  }
  local->finishProcessingNode(ii, rdata);
  delete local;
}

//-----------------------------------------------------------------------
void MathParser::processOneItem2d(VolumeData *rdata, int ii,
				  TaThreadQue *thread) const
{
  LOG(DEBUG_VERBOSE) << "Processing 2d item " << ii;

  thread->lockForIO();
  // make a local copy, which needs rdata
  MathData *local = rdata->initializeProcessingNode(ii, true);
  thread->unlockAfterIO();

  // local thing doesn't need thread locking.
  for (size_t f = 0; f< _filters2d.size(); ++f)
  {
    _processLoop(_filters2d[f], local, ii==0 || _outputDebugAll);
  }

  thread->lockForIO();
  // update rdata
  local->finishProcessingNode(ii, rdata);
  thread->unlockAfterIO();

  delete local;
}

//-----------------------------------------------------------------------
void MathParser::processOneItem1d(VolumeData *rdata, int ii) const
{
  LOG(DEBUG_VERBOSE) << "Processing 1d item " << ii;

  // make a local copy (thinking ahead to threading)
  MathData *local = rdata->initializeProcessingNode(ii, false);
  for (size_t f = 0; f< _filters1d.size(); ++f)
  {
    _processLoop(_filters1d[f], local, ii==0 || _outputDebugAll);
  }
  local->finishProcessingNode(ii, rdata);
  delete local;
}

//-------------------------------------------------------------------
void MathParser::trim(string &s)
{
  string whitespace = " \t";
  std::size_t b = s.find_first_not_of(whitespace);
  if (b == string::npos)
  {
    // empty string
    return;
  }
  std::size_t b2 = s.find_last_not_of(whitespace);
  std::size_t range = b2 - b + 1;
  s = s.substr(b, range);
}

//-------------------------------------------------------------------
bool MathParser::isFloat(const std::string &s)
{
  if (s.empty())
  {
    return false;
  }
  size_t p = 0;
  if (s[p] != '-' && !isdigit(s[p]))
  {
    return false;
  }
  int numDot = 0;
  for (p=1; p<s.size(); ++p)
  {
    if (s[p] == '.')
    {
      ++numDot;
      if (numDot > 1)
      {
	return false;
      }
    }
    else if (!isdigit(s[p]))
    {
      return false;
    }
  }
  return true;
}
  
//-------------------------------------------------------------------
bool MathParser::isTrue(const std::string &s)
{
  return s == "True" || s == "TRUE" || s == "true";
}

//-------------------------------------------------------------------
bool MathParser::isFalse(const std::string &s)
{
  return s == "False" || s == "FALSE" || s == "false";
}

//-------------------------------------------------------------------
bool MathParser::parenRemove(string &s)
{
  // skip white space at begin and end
  trim(s);
  std::size_t i0 = s.find_first_of("(");
  std::size_t i1 = s.find_last_of(")");
  if (i0 == 0 && i1 == s.size()-1)
  {
    // count parens to make sure outer ones are correctly
    // ordered   (  () (()) )  is ok
    //           ()()          is not ok

    int count = 1;
    for (int i=1; i<(int)s.size(); ++i)
    {
      if (s[i] == '(')
      {
	++count;
      }
      else if (s[i] == ')')
      {
	--count;
	if (count == 0 && i < (int)i1)
	{
	  // bad
	  return false;
	}
      }
    }
    s = s.substr(1, i1-i0-1);
    return true;
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
std::string MathParser::sprintUnaryOperators(void) const
{
  string ret = "";
  
  for (size_t i=0; i<_unaryOperators.size(); ++i)
  {
    ret += _unaryOperators[i]._name;
    ret += ":\n";
    ret += _unaryOperators[i]._description;
    ret += "\n\n";
  }
  for (size_t i=0; i<_userUnaryOperators.size(); ++i)
  {
    ret += _userUnaryOperators[i]._name;
    ret += ":\n";
    ret += _userUnaryOperators[i]._description;
    ret += "\n\n";
  }
  return ret;
}

//-------------------------------------------------------------------
std::string MathParser::sprintBinaryOperators(void) const
{
  string ret = "";
  
  for (size_t i=0; i<_binaryOperators.size(); ++i)
  {
    ret += _binaryOperators[i]._name;
    ret += ":\n";
    ret += _binaryOperators[i]._description;
    ret += "\n\n";
  }
  for (size_t i=0; i<_userBinaryOperators.size(); ++i)
  {
    ret += _userBinaryOperators[i]._name;
    ret += ":\n";
    ret += _userBinaryOperators[i]._description;
    ret += "\n\n";
  }
  return ret;
}

//-------------------------------------------------------------------
ProcessingNode *MathParser::_parse(const std::string &s)
{
  string s2 = s;
  trim(s2);
  LOG(DEBUG_VERBOSE) << "Evaluating '" << s2 << "'";

  // check if it is a special if then
  if (s2.find("if") == 0)
  {
    std::size_t p = s2.find("then");
    if (p == string::npos)
    {
      LOG(ERROR) << "if statement " << s2;
      return NULL;
    }
    return _parseIfThen(s2, p);
  }

  // check if it is a special = (can only be one)
  if (s2.find("=") != string::npos)
  {
    // check for outermost matching parens before anything else
    parenRemove(s2);
    
    std::size_t p = s2.find("=");
    if (p > 0)
    {
      return _parseAssignment(s2, p);
    }
    else
    {
      LOG(ERROR) << "assignment " << s2;
      return NULL;
    }      
  }

  for (size_t i=0; i<_binaryOperators.size(); ++i)
  {
    ProcessingNode::Operator_t o = ProcessingNode::binaryOperatorValue(i);
    int io = (int)o;
    ProcessingNode *pnode =
      _parseBinaryOperator(s2, _binaryOperators[i]._name, io);
    if (pnode != NULL)
    {
      return pnode;
    }
  }

  for (size_t i=0; i<_userBinaryOperators.size(); ++i)
  {
    ProcessingNode::Operator_t o = ProcessingNode::USER;
    int io = (int)o;
    ProcessingNode *pnode =
      _parseBinaryOperator(s2, _userBinaryOperators[i]._name, io);
    if (pnode != NULL)
    {
      return pnode;
    }
  }

  // try removing parens and then parsing
  if (parenRemove(s2))
  {
    return _parse(s2);
  }

  // check for functions like sin(3+4), or specialfunction(a,b,c)
  std::size_t p = s2.find_first_of("(");
  if (p != string::npos && p > 0)
  {
    std::size_t p2 = s2.find_last_of(")");
    if (p2 == s2.size() -1)
    {
      string fname = s2.substr(0, p);
      string part2 = s2.substr(p+1,p2-p-1);
      ProcessingNode *pnode = _parseUnary(s2, fname, part2);
      if (pnode != NULL)
      {
	return pnode;
      }
    }
  }
  // all has been exausted end of the line
  return _val(s2);
}

//-------------------------------------------------------------------
ProcessingNode *MathParser::_parseIfThen(std::string &s2, std::size_t p)
{
  // s2 = 'if ( xxxxxx) then yyyyyyyy', where yyyyyy starts at position p
  string findStr = s2.substr(3, p-3);
  trim(findStr);
  string valStr = s2.substr(p+4);
  trim(valStr);
  Find f(findStr);
  ProcessingNode *v1 = _parse(valStr);
  if (v1 != NULL)
  {
    return new ProcessingNode(s2, f, v1);
  }
  else
  {
    return NULL;
  }
}

//-------------------------------------------------------------------
ProcessingNode *MathParser::_parseAssignment(std::string &s2,
					     std::size_t p)
{
  // s2 = 'variable = value' where = is at position p
  string variableStr = s2.substr(0, p);
  trim(variableStr);

  string valStr = s2.substr(p+1);
  trim(valStr);

  ProcessingNode *var = _parse(variableStr);
  if (var == NULL)
  {
    LOG(ERROR) << "Variable parse in assignment failed" << variableStr;
    return NULL;
  }
  if (var->isVariable())
  {
    ProcessingNode *val = _parse(valStr);
    if (val == NULL)
    {
      LOG(ERROR) << "Value parse in assignment failed" << valStr;
      return NULL;
    }
    else
    {
      return new ProcessingNode(s2, var->getLeafContent(),  val);
    }
  }
  else
  {
    LOG(ERROR) << "Syntax error " << s2;
    return NULL;
  }
}


//-------------------------------------------------------------------
ProcessingNode *
MathParser::_parseBinaryOperator(std::string &s2, const std::string &op,
				 int io)
{
  // search for last known operator in string, outside of parens
  std::size_t found = _lastOutsideOfParens(s2, op);
  if (found == string::npos)
  {
    return NULL;
  }

  // if it is a '-', see if it really is an operator
  if (!_isOperator(s2, found))
  {
    return NULL;
  }

  string part1 = s2.substr(0, found);
  trim(part1);
  string part2 = s2.substr(found + 1);
  trim(part2);

  // not inside parens at this point
  ProcessingNode *v1 = _parse(part1);
  ProcessingNode *v2 = _parse(part2);
  if (v1 == NULL || v2 == NULL)
  {
    if (v1 != NULL) delete v1;
    if (v2 != NULL) delete v2;
    return NULL;
  }

  ProcessingNode::Operator_t o = (ProcessingNode::Operator_t)io;
  if (o != ProcessingNode::BAD)
  {
    return new ProcessingNode(s2, v1, v2, o);
  }
  else
  {
    LOG(ERROR) << "Operator is bad";
    delete v1;
    delete v2;
    return NULL;
  }
}

//-------------------------------------------------------------------
ProcessingNode *MathParser::_parseUnary(const std::string &s2,
					std::string &fname, std::string &part2)
{
  for (size_t i=0; i<_unaryOperators.size(); ++i)
  {
    if (fname == _unaryOperators[i]._name)
    {
      bool bad;
      std::vector<ProcessingNode *> pargs = _commaSeparatedArgNodes(part2, bad);
      if (!bad)
      {
	ProcessingNode::UnaryOperator_t o =
	  ProcessingNode::unaryOperatorValue(i);
	if (o != ProcessingNode::UBAD)
	{
	  return new ProcessingNode(s2, o, pargs);
	}
      }
      else
      {
	return NULL;
      }
    }
  }
  for (size_t i=0; i<_userUnaryOperators.size(); ++i)
  {
    if (fname == _userUnaryOperators[i]._name)
    {
      bool bad;
      std::vector<ProcessingNode *> pargs = _commaSeparatedArgNodes(part2, bad);
      if (!bad)
      {
	return new ProcessingNode(s2, pargs, _userUnaryOperators[i]._name);
      }
      else
      {
	return NULL;
      }
    }
  }
  return NULL;
}

//-------------------------------------------------------------------
std::vector<ProcessingNode *>
MathParser::_commaSeparatedArgNodes(std::string &part2, bool &bad)
{      
  std::vector<ProcessingNode *> pargs;
  bad = false;
  
  // special search for multiple args comma separated here
  std::vector<string> args = _commaSeparatedArgs(part2);
  for (size_t i=0; i<args.size(); ++i)
  {
    ProcessingNode *v = _parse(args[i]);
    if (v == NULL)
    {
      bad = true;
      for (size_t j=0; j<pargs.size(); ++j)
      {
	delete pargs[j];
      }
      pargs.clear();
      return pargs;
    }
    else
    {
      pargs.push_back(v);
    }
  }
  return pargs;
}

//-------------------------------------------------------------------
ProcessingNode *MathParser::_val(const std::string &s)
{
  double v;
  if (isFloat(s))
  {
    if (sscanf(s.c_str(), "%lf", &v) == 1)
    {
      return new ProcessingNode(s, v);
    }
    else
    {
      printf("ERROR in interpreting %s as a number\n", s.c_str());
      return NULL;
    }
  }

  // assume it is a variable, or a constant
  return new ProcessingNode(s);
}

//-----------------------------------------------------------------------
void MathParser::_processLoop(const Filter &filter, MathData *rdata,
			      bool debug) const
{
  PMU_auto_register(filter._filter->sprint().c_str());
  if (debug)
  {
    LOG(DEBUG) << filter._filter->sprint();
  }

  if (!rdata->synchInputsAndOutputs(filter._output, filter._inputs))
  {
    LOG(ERROR) << "Could not synch up data";
  }

  string keyword;
  if (filter._filter->isUserAssignmentWithUnaryOp(keyword, false))
  {
    if (!rdata->synchUserDefinedInputs(keyword, filter._inputs))
    {
      LOG(ERROR) << "Could not synch up inputs";
      return;
    }
  }
  // else
  // {
  //   LOG(DEBUG) << "What does this mean, not sure";
  // }


  // loop through all the individual loop items
  

  // now process this step
  switch (filter._pattern)
  {
  case Node::SIMPLE_ASSIGN_MISSING_TO_VAR:
    _simpleAssignNumberToVar(filter, rdata, true);
    break;
  case Node::SIMPLE_ASSIGN_NUMBER_TO_VAR:
    _simpleAssignNumberToVar(filter, rdata, false);
    break;
  case Node::SIMPLE_ASSIGN_VAR_TO_VAR:
    _simpleAssignVarToVar(filter, rdata);
    break;
  case Node::LOGICAL_SIMPLE_ASSIGN_NUMBER_TO_VAR:
    _simpleLogicalAssignNumberToVar(filter, rdata);
    break;
  case Node::LOGICAL_SIMPLE_ASSIGN_VAR_TO_VAR:
    _simpleLogicalAssignVarToVar(filter, rdata);
    break;
  case Node::LOGICAL_MULTIPLE_SIMPLE_ASSIGN_NUMBER_TO_VAR:
    _simpleLogicalMultiplesAssignNumberToVar(filter, rdata);
    break;
  case Node::LOGICAL_MULTIPLE_SIMPLE_ASSIGN_VAR_TO_VAR:
    _simpleLogicalMultiplesAssignVarToVar(filter, rdata);
    break;
  case Node::SIMPLE_ASSIGN_SIMPLE_BINARY_TO_VAR:
    // Needs work
  case Node::DO_IT_THE_HARD_WAY:
  default:
    if (filter._dataType == LOOP2D_TO_USER_DEFINED)
    {
      MathUserData *s = filter._filter->processToUserDefined(rdata);
      if (s == NULL)
      {
	LOG(ERROR) << " Could not process user defined filtering";
      }
      else
      {
	if (!rdata->storeMathUserData(filter._output, s))
	{
	  delete s;
	}
      }
    }
    else
    {
      filter._filter->process(rdata);
    }
    break;
  }
}

//-----------------------------------------------------------------------
void MathParser::_processV(const Filter &filter, VolumeData *rdata) const
{
  PMU_auto_register(filter._filter->sprint().c_str());
  LOG(DEBUG) << filter._filter->sprint();
  string keyword;
  if (filter._filter->isUserAssignmentWithUnaryOp(keyword))
  {
    if (!rdata->synchUserDefinedInputs(keyword, filter._inputs))
    {
      LOG(ERROR) << "Could not synch up inputs";
      return;
    }
  }
  else
  {
    LOG(ERROR) << "Global filters must be unary operations";
    return;
  }

  // now process this step
  MathUserData *s = filter._filter->processVol(rdata);
  if (s == NULL)
  {
    LOG(ERROR) << "No special data for volume filter";
  }
  else
  {
    if (!rdata->storeMathUserData(filter._output, s))
    {
      delete s;
    }
  }
}
