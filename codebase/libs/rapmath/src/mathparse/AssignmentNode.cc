#include <rapmath/AssignmentNode.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/LeafNode.hh>
#include <rapmath/BinaryNode.hh>
#include <rapmath/MathData.hh>
#include <rapmath/MathLoopData.hh>
#include <toolsa/LogStream.hh>

//-------------------------------------------------------------------
AssignmentNode::
AssignmentNode(const LeafContent &var, ProcessingNode *value) :
  _variable(var), _assignedValue(value)
{
  std::string s;
  double v;
  bool m;
  if (_assignedValue->getLeafVariableName(s))
  {
    _pattern = SIMPLE_ASSIGN_VAR_TO_VAR;
  }
  else if (_assignedValue->getLeafNumberOrMissing(v, m))
  {
    if (m)
    {
      _pattern = SIMPLE_ASSIGN_MISSING_TO_VAR;
    }
    else
    {
      _pattern = SIMPLE_ASSIGN_NUMBER_TO_VAR;
    }
  }
  else if (_assignedValue->isSimpleBinary())
  {
    _pattern = SIMPLE_ASSIGN_SIMPLE_BINARY_TO_VAR;
  }
}

//-------------------------------------------------------------------
AssignmentNode::~AssignmentNode()
{
}
 
//-------------------------------------------------------------------
void AssignmentNode::cleanup(void)
{
  if (_assignedValue != NULL)
  {
    _assignedValue->cleanup();
    delete _assignedValue;
    _assignedValue = NULL;
  }
}

//-------------------------------------------------------------------
void AssignmentNode::printParsedCr(void) const
{
  printParsed();
  printf("\n");
}

//-------------------------------------------------------------------
void AssignmentNode::printParsed(void) const
{
  printf("(");
  _variable.print();
  printf(")=(");
  _assignedValue->printParsed();
  printf(")");
}

//-------------------------------------------------------------------
bool AssignmentNode::isUnaryUserOpRightHandSide(std::string &keyword,
						bool warn) const
{
  return _assignedValue->isUserUnaryOp(keyword, warn);
}

//-------------------------------------------------------------------
MathUserData *
AssignmentNode::processVol(VolumeData *data) const
{
  return _assignedValue->processVol(data);
}

//-------------------------------------------------------------------
MathUserData*AssignmentNode::processToUserDefined(MathData *data) const
{
  return _assignedValue->processToUserDefined(data);
}

//-------------------------------------------------------------------
bool AssignmentNode::process(MathData *data) const
{
  std::string name = _variable.getName();
  if (name.empty())
  {
    LOG(ERROR) << "No name";
    return false;
  }
  MathLoopData *ptr = data->dataPtr(name);
  if (ptr == NULL)
  {
    LOG(ERROR) << "No data for " << name;
    return false;
  }
  // if it is a user function on the right hand of an assignment
  // we want to do that a different way
  if (_assignedValue->isUserUnaryFunction())
  {
    return _processUserFunction(data);
  }
  else if (_assignedValue->isMultiArgUnaryFunction())
  {
    return _processMultiArgUnaryFunction(ptr, data);
  }
  else
  {
    LOG(DEBUG_VERBOSE) << "Processing for an assignment npoints="
		       << data->numData();


    // we loop through and compute the rhs, assigning it to the output
    for (int i=0; i<data->numData(); ++i)
    {
      double v;
      if (compute(data, i, v))
      {
	LOG(DEBUG_VERBOSE) << "[" << i << "]=" << v;
	ptr->setVal(i, v);
      }	
      else
      {
	LOG(DEBUG_VERBOSE) << "[" << i << "]=missing";
	ptr->setMissing(i);
      }
    }
    return true;
  }
}

//-------------------------------------------------------------------
bool AssignmentNode::compute(const MathData *data,
			     int ipt, double &v) const
{
  // the returned value will be assigned to something
  return _assignedValue->compute(data, ipt, v);
}

//-------------------------------------------------------------------
void AssignmentNode::outputFields(std::vector<std::string> &names) const
{
  names.push_back(_variable.getName());
}

//-------------------------------------------------------------------
void AssignmentNode::inputFields(std::vector<std::string> &names) const
{
  _assignedValue->inputFields(names);
}

//-------------------------------------------------------------------
bool AssignmentNode::getSimpleAssign(std::string &name, double &number,
				     bool &missing) const
{
  name = _variable.getName();
  if (_assignedValue->getType() == ProcessingNode::LEAF)
  {
    const LeafNode *l = (const LeafNode *)(_assignedValue->nodePtr());
    if (l->isVariable())
    {
      return false;
    }
    return l->getLeafNumberOrMissing(number, missing);
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
bool AssignmentNode::getSimpleAssign(std::string &from, std::string &to) const
{
  to = _variable.getName();
  if (_assignedValue->getType() == ProcessingNode::LEAF)
  {
    from = ((LeafNode *)(_assignedValue->nodePtr()))->getName();
    return true;
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
bool AssignmentNode::getAssignedName(std::string &name) const
{
  if (_variable.isVariable())
  {
    name = _variable.getName();
    return true;
  }
  else
  {
    LOG(ERROR) << "Assignment to non variable";
    return false;
  }
}

//-------------------------------------------------------------------
bool AssignmentNode::getSimpleBinaryArgs(BinaryArgs &args) const
{
  if (_assignedValue->getType() == ProcessingNode::BINARY)
  {
    return ((BinaryNode *)
	    (_assignedValue->nodePtr()))->getSimpleArgs(args);
  }
  else
  {
    LOG(ERROR) << "Wrong assigned value type";
    return false;
  }
}

//-------------------------------------------------------------------
bool AssignmentNode::_processUserFunction(MathData *data) const
{
  return data->processUserLoopFunction(*_assignedValue);
}

//-------------------------------------------------------------------
bool AssignmentNode::
_processMultiArgUnaryFunction(MathLoopData *ptr, MathData *data) const
{
  ProcessingNode::UnaryOperator_t uop = _assignedValue->getUnaryOperator();
  std::vector<ProcessingNode *> *args = _assignedValue->unaryOpArgs();
  if (args == NULL)
  {
    LOG(ERROR) << "No args";
    return false;
  }
  else
  {
    if (args->size() < 2)
    {
      LOG(ERROR) << "Expect multiple args, got " << args->size();
      return false;
    }
  }
  switch (uop)
  {
  case ProcessingNode::SMOOTH:
    return data->smooth(ptr, *args);
  case ProcessingNode::SMOOTHDBZ:
    return data->smoothDBZ(ptr, *args);
  case ProcessingNode::STDDEV:
    return data->stddev(ptr, *args);
  case ProcessingNode::FUZZY:
    return data->fuzzy(ptr, *args);
  case ProcessingNode::AVERAGE:
    return data->average(ptr, *args);
  case ProcessingNode::MAX:
    return data->max(ptr, *args);
  case ProcessingNode::MAX_EXPAND:
    return data->max_expand(ptr, *args);
  case ProcessingNode::EXPAND_ANGLES_LATERALLY:
    return data->expand_angles_laterally(ptr, *args);
  case ProcessingNode::CLUMP:
    return data->clump(ptr, *args);
  case ProcessingNode::MEDIAN:
    return data->median(ptr, *args);
  case ProcessingNode::WEIGHTED_AVERAGE:
    return data->weighted_average(ptr, *args);
  case ProcessingNode::WEIGHTED_ANGLE_AVERAGE:
    return data->weighted_angle_average(ptr, *args);
  case ProcessingNode::MASK:
    return data->mask(ptr, *args);
  case ProcessingNode::MASK_MISSING_TO_MISSING:
    return data->mask_missing_to_missing(ptr, *args);
  case ProcessingNode::TRAPEZOID:
    return data->trapezoid(ptr, *args);
  case ProcessingNode::S_REMAP:
    return data->s_remap(ptr, *args);
  default:
    LOG(ERROR) << "Bad operator for method " << uop;
    return false;
  }
}
