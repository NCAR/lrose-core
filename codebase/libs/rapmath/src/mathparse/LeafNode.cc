#include <rapmath/LeafNode.hh>
#include <toolsa/LogStream.hh>

//-------------------------------------------------------------------
LeafNode::LeafNode(const std::string &s) :  _leafContent(s)
{
}

//-------------------------------------------------------------------
LeafNode::LeafNode(const std::string &s, double value) : _leafContent(s, value)
{
}

//-------------------------------------------------------------------
LeafNode::~LeafNode()
{
}
 
//-------------------------------------------------------------------
void LeafNode::cleanup(void)
{
}

//-------------------------------------------------------------------
void LeafNode::printParsedCr(void) const
{
  printParsed();
  printf("\n");
}

//-------------------------------------------------------------------
void LeafNode::printParsed(void) const
{
  printf("(");
  _leafContent.print();
  printf(")");
}

//-------------------------------------------------------------------
bool LeafNode::isVariable(void) const
{
  return _leafContent.isVariable();
}
    
//-------------------------------------------------------------------
bool LeafNode::getLeafVariableName(std::string &s) const
{
  if (_leafContent.isVariable())
  {
    s = _leafContent.getName();
    return true;
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
bool LeafNode::getLeafNumberOrMissing(double &v, bool &isMissing) const
{
  if (!isVariable())
  {
    if (_leafContent.getValue(v))
    {
      isMissing = false;
    }
    else
    {
      isMissing = true;
    }
    return true;
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
MathUserData *LeafNode::processVol(VolumeData *data) const
{
  LOG(ERROR) << " Only assignments based on unary functions";
  return NULL;
}

//-------------------------------------------------------------------
MathUserData*LeafNode::processToUserDefined(MathData *data) const
{
  LOG(ERROR) << "no processing except at top level";
  return NULL;
}

//-------------------------------------------------------------------
bool LeafNode::process(MathData *data) const
{
  LOG(ERROR) << " No processing except at top level";
  return false;
}

//-------------------------------------------------------------------
bool LeafNode::compute(const MathData *data, int ipt, double &v) const
{
  return _leafContent.getValue(data, ipt, v);
}

//-------------------------------------------------------------------
void LeafNode::outputFields(std::vector<std::string> &names) const
{
}

//-------------------------------------------------------------------
void LeafNode::inputFields(std::vector<std::string> &names) const
{
  if (_leafContent.isVariable())
  {
    names.push_back(_leafContent.getName());
  }
}


