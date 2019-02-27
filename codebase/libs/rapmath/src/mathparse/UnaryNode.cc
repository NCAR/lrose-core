/**
 * @file UnaryNode.cc
 */
#include <rapmath/UnaryNode.hh>
#include <rapmath/VolumeData.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//-------------------------------------------------------------------
UnaryNode::UnaryNode(ProcessingNode::UnaryOperator_t op,
		     std::vector<ProcessingNode *> &args) :
  _userUopKey(""),
  _uop(op),
  _value(args)
{
}

//-------------------------------------------------------------------
UnaryNode::UnaryNode(std::vector<ProcessingNode *> &args,
		     const std::string &key) :
  _userUopKey(key),
  _uop(ProcessingNode::UUSER),
  _value(args)
{
}

//-------------------------------------------------------------------
UnaryNode::~UnaryNode(void)
{
}
 
//-------------------------------------------------------------------
void UnaryNode::cleanup(void)
{
  for (size_t i=0; i<_value.size(); ++i)
  {
    if (_value[i] != NULL)
    {
      delete _value[i];
      _value[i] = NULL;
    }
  }
  _value.clear();
}

//-------------------------------------------------------------------
void UnaryNode::printParsedCr(void) const
{
  printParsed();
  printf("\n");
}

//-------------------------------------------------------------------
void UnaryNode::printParsed(void) const
{
  printf("(");
  printf("%s(", ProcessingNode::sprintUOp(_uop).c_str());
  _value[0]->printParsed();
  printf(")");
  printf(")");
}

//-------------------------------------------------------------------
MathUserData *UnaryNode::processVol(VolumeData *data) const
{
  // expect only an assignment, or the right hand side, which must be a
  // unary operation (so far)
  // in fact, so far only user defined unary functions are in the state
  if (_userUopKey.empty())
  {
    LOG(ERROR) << "only user defined unaries currently implemented for volumes";
    return NULL;
  }
  else
  {
    // the data now takes over and does its thing
    return data->processUserVolumeFunction(*this);
  }
}

//-------------------------------------------------------------------
MathUserData*UnaryNode::processToUserDefined(MathData *data) const
{
  // expect only an assignment, or the right hand side, which must be a
  // unary operation (so far)
  return data->processUserLoopFunctionToUserData(*this);
}

//-------------------------------------------------------------------
bool UnaryNode::process(MathData *data) const
{
  LOG(ERROR) << " no processing except at top level";
  return false;
}

//-------------------------------------------------------------------
bool UnaryNode::compute(const MathData *inputs, int ipt, double &v) const
{
  if (!_userUopKey.empty())
  {
    LOG(ERROR) << "Compute method incorrect when a user unary op";
    return false;
  }

  if (_value.size() != 1)
  {
    LOG(ERROR) << "Wrong methd, multiple arguments to unary";
    return false;
  }
  if (_value[0]->compute(inputs, ipt, v))
  {
    LOG(DEBUG_VERBOSE) << "uop=" << _uop << " Arg '" << _value[0]->sprint()
		       << "[" << ipt << "]=" << v;
    switch (_uop)
    {
    case ProcessingNode::ABS:
      v = fabs(v);
      break;
    case ProcessingNode::SQRT:
      v = sqrt(v);
      break;
    case ProcessingNode::LOG10:
      v = log10(v);
      break;
    case ProcessingNode::EXP:
      v = exp(v);
      break;
    default:
      LOG(ERROR) << "wrong op for this method " << _uop;
      return false;
    }
    LOG(DEBUG_VERBOSE) << "result = " << v;
    return true;
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
void UnaryNode::outputFields(std::vector<std::string> &names) const
{
}

//-------------------------------------------------------------------
void UnaryNode::inputFields(std::vector<std::string> &names) const
{
  for (size_t i=0; i<_value.size(); ++i)
  {
    _value[i]->inputFields(names);
  }
}

//-------------------------------------------------------------------
bool UnaryNode::getUserUnaryKeyword(std::string &keyword, bool warn) const
{
  if (_uop != ProcessingNode::UUSER)
  {
    if (warn)
    {
      LOG(DEBUG) << "Not user function";
    }
    return false;
  }
  if (_userUopKey.empty())
  {
    if (warn)
    {
      LOG(DEBUG) << "No key";
    }
    return false;
  }
  keyword = _userUopKey;
  return true;
}

//-------------------------------------------------------------------
std::vector<std::string> UnaryNode::getUnaryNodeArgStrings(void) const
{
  std::vector<std::string> ret;
  for (size_t i=0; i<_value.size(); ++i)
  {
    ret.push_back(_value[i]->getInput());
  }
  return ret;
}

//-------------------------------------------------------------------
bool UnaryNode::isUserFunction(void) const
{
  return !_userUopKey.empty() && _uop == ProcessingNode::UUSER;
}

//-------------------------------------------------------------------
bool UnaryNode::isMultiArgFunction(void) const
{
  switch (_uop)
  {
  case ProcessingNode::SMOOTH:
  case ProcessingNode::SMOOTHDBZ:
  case ProcessingNode::STDDEV:
  case ProcessingNode::FUZZY:
  case ProcessingNode::AVERAGE:
  case ProcessingNode::MAX:
  case ProcessingNode::MAX_EXPAND:
  case ProcessingNode::EXPAND_ANGLES_LATERALLY:
  case ProcessingNode::CLUMP:
  case ProcessingNode::MEDIAN:
  case ProcessingNode::WEIGHTED_AVERAGE:
  case ProcessingNode::WEIGHTED_ANGLE_AVERAGE:
  case ProcessingNode::MASK:
  case ProcessingNode::MASK_MISSING_TO_MISSING:
  case ProcessingNode::TRAPEZOID:
  case ProcessingNode::S_REMAP:
    return true;
  default:
    return false;
  }
}
    
