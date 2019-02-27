#include <rapmath/BinaryNode.hh>
#include <rapmath/BinaryArgs.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
using std::string;

//-------------------------------------------------------------------
BinaryNode::BinaryNode(ProcessingNode *left,
	       ProcessingNode *right,
	       ProcessingNode::Operator_t op) :
  _left(left),
  _right(right),
  _op(op)
{
}

//-------------------------------------------------------------------
BinaryNode::BinaryNode(ProcessingNode *left,
	       ProcessingNode *right,
	       const std::string &key) :
  _left(left),
  _right(right),
  _userOpKey(key),
  _op(ProcessingNode::USER)
{
}

//-------------------------------------------------------------------
BinaryNode::~BinaryNode()
{
}
 
//-------------------------------------------------------------------
void BinaryNode::cleanup(void)
{
  if (_left != NULL)
  {
    _left->cleanup();
    delete _left;
    _left = NULL;
  }
  if (_right != NULL)
  {
    _right->cleanup();
    delete _right;
    _right = NULL;
  }
}


//-------------------------------------------------------------------
void BinaryNode::printParsedCr(void) const
{
  printParsed();
  printf("\n");
}

//-------------------------------------------------------------------
void BinaryNode::printParsed(void) const
{
  printf("(");
  _left->printParsed();
  printf("%s", ProcessingNode::sprintOp(_op).c_str());
  _right->printParsed();
  printf(")");
}

//-------------------------------------------------------------------
MathUserData *BinaryNode::processVol(VolumeData *data) const
{
  LOG(ERROR) << "only assignments based on unary functions";
  return NULL;
}

//-------------------------------------------------------------------
MathUserData*BinaryNode::processToUserDefined(MathData *data) const
{
  LOG(ERROR) << "no processing except at top level";
  return NULL;
}

//-------------------------------------------------------------------
bool BinaryNode::process(MathData *data) const
{
  LOG(ERROR) << "no processing except at top level";
  return false;
}

//-------------------------------------------------------------------
bool BinaryNode::compute(const MathData *data,
			 int ipt, double &v) const
{
  double v0, v1;
  if (_left->compute(data, ipt, v0) && _right->compute(data, ipt, v1))
  {
    LOG(DEBUG_VERBOSE) << "binary Computed '" << _left->sprint()
		       << "'=" << v0 << "'" << _right->sprint()
		       << "'=" << v1;
    switch (_op)
    {
    case ProcessingNode::ADD:
      v = v0 + v1;
      break;
    case ProcessingNode::SUB:
      v = v0 - v1;
      break;
    case ProcessingNode::MULT:
      v = v0*v1;
      break;
    case ProcessingNode::DIV:
      if (v1 != 0)
      {
	v = v0 / v1;
      }
      else
      {
	LOG(ERROR) << "divide by zero..";
	return false;
      }
      break;
    case ProcessingNode::POW:
      v = pow(v0, v1);
      break;
    default:
      LOG(ERROR) << "Unknown op";
      return false;
    }
    return true;
  }
  else
  {
    // one of the data values assumed missing
    return false;
  }
}

//-------------------------------------------------------------------
void BinaryNode::outputFields(std::vector<std::string> &names) const
{
}

//-------------------------------------------------------------------
void BinaryNode::inputFields(std::vector<std::string> &names) const
{
  _left->inputFields(names);
  _right->inputFields(names);
}

//-------------------------------------------------------------------
bool BinaryNode::isSimple(void) const
{
  string name;
  double value;
  bool missing;
  bool leftSimple = false;
  bool rightSimple = false;
  if (_left->getLeafVariableName(name) || _left->getLeafNumberOrMissing(value, missing))
  {
    leftSimple = true;
  }
  else
  {
    if (_left->getType() == ProcessingNode::BINARY)
    {
      leftSimple = ((BinaryNode *)(_left->nodePtr()))->isSimple();
    }
  }
  if (_right->getLeafVariableName(name) || _right->getLeafNumberOrMissing(value, missing))
  {
    rightSimple = true;
  }
  else
  {
    if (_right->getType() == ProcessingNode::BINARY)
    {
      rightSimple = ((BinaryNode *)(_right->nodePtr()))->isSimple();
    }
  }
  return leftSimple && rightSimple;
}

//-------------------------------------------------------------------
bool BinaryNode::getSimpleArgs(BinaryArgs &args) const
{
  if (!isSimple())
  {
    LOG (ERROR) << "Args are not simple";
    return false;
  }

  string name;
  double value;
  bool missing;
  if (_left->getLeafVariableName(name))
  {
    BinaryArg b(name);
    args.appendArg(b);
  }
  else if (_left->getLeafNumberOrMissing(value, missing))
  {
    BinaryArg b(value, missing);
    args.appendArg(b);
  }
  else
  {
    if (_left->getType() == ProcessingNode::BINARY)
    {
      if (!((BinaryNode *)(_left->nodePtr()))->getSimpleArgs(args))
      {
	return false;
      }
    }
    else
    {
      return false;
    }
  }

  args.appendOp(_op);

  if (_right->getLeafVariableName(name))
  {
    BinaryArg b(name);
    args.appendArg(b);
  }
  else if (_right->getLeafNumberOrMissing(value, missing))
  {
    BinaryArg b(value, missing);
    args.appendArg(b);
  }
  else
  {
    if (_right->getType() == ProcessingNode::BINARY)
    {
      if (!((BinaryNode *)(_right->nodePtr()))->getSimpleArgs(args))
      {
	return false;
      }
    }
    else
    {
      return false;
    }
  }
  return true;
}

