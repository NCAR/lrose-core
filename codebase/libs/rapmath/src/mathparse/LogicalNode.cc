#include <cstdlib>
#include <rapmath/LogicalNode.hh>
#include <rapmath/MathLoopData.hh>
#include <rapmath/MathData.hh>
#include <rapmath/ProcessingNode.hh>
#include <rapmath/AssignmentNode.hh>
#include <toolsa/LogStream.hh>

//-------------------------------------------------------------------
LogicalNode::LogicalNode(const Find &find, ProcessingNode *action) :
  _find(find),
  _action(action)
{
  if (!action->isAssignment())
  {
    LOG(FATAL) << "LogicalNode action must be an assignment";
    exit(-1);
  }
  if ((action->pattern() == Node::SIMPLE_ASSIGN_NUMBER_TO_VAR ||
       action->pattern() == Node::SIMPLE_ASSIGN_MISSING_TO_VAR) &&
      _find.pattern() == Find::SIMPLE_COMPARE_TO_NUMBER)
  {
    _pattern = LOGICAL_SIMPLE_ASSIGN_NUMBER_TO_VAR;
  }
  else if (action->pattern() == Node::SIMPLE_ASSIGN_VAR_TO_VAR &&
      _find.pattern() == Find::SIMPLE_COMPARE_TO_NUMBER)
  {
    _pattern = LOGICAL_SIMPLE_ASSIGN_VAR_TO_VAR;
  }
  else if ((action->pattern() == Node::SIMPLE_ASSIGN_NUMBER_TO_VAR ||
	    action->pattern() == Node::SIMPLE_ASSIGN_MISSING_TO_VAR) &&
	   _find.pattern() == Find::SIMPLE_MULTIPLES)
  {
    _pattern = LOGICAL_MULTIPLE_SIMPLE_ASSIGN_NUMBER_TO_VAR;
  }
  else if (action->pattern() == Node::SIMPLE_ASSIGN_VAR_TO_VAR &&
	   _find.pattern() == Find::SIMPLE_MULTIPLES)
  {
    _pattern = LOGICAL_MULTIPLE_SIMPLE_ASSIGN_VAR_TO_VAR;
  }
}

//-------------------------------------------------------------------
LogicalNode::~LogicalNode()
{
}
 
//-------------------------------------------------------------------
void LogicalNode::cleanup(void)
{
  if (_action != NULL)
  {
    _action->cleanup();
    delete _action;
    _action = NULL;
  }
}

//-------------------------------------------------------------------
void LogicalNode::printParsedCr(void) const
{
  printParsed();
  printf("\n");
}

//-------------------------------------------------------------------
void LogicalNode::printParsed(void) const
{
  printf("(");
  printf("if (");
  _find.printTop();
  printf(") then (");
  _action->printParsed();
  printf(")");
  printf(")");
}


//-------------------------------------------------------------------
MathUserData *LogicalNode::processVol(VolumeData *data) const
{
  // expect only an assignment, or the right hand side, which must be a
  // unary operation (so far)
  LOG(ERROR) << "only assignments based on unary functions";
  return NULL;
}

//-------------------------------------------------------------------
MathUserData*LogicalNode::processToUserDefined(MathData *data) const
{
  LOG(ERROR) << "only assignments based on unary functions";
  return NULL;
}

//-------------------------------------------------------------------
bool LogicalNode::process(MathData *data) const
{
  std::string name;
  MathLoopData *l=NULL;
  if (_action->getAssignName(name))
  {
    l = data->dataPtr(name);
  }
  if (l == NULL)
  {
    LOG(ERROR) << "Logical test with a non assigment action";
    return false;
  }

  for (int i=0; i<data->numData(); ++i)
  {
    if (_find.satisfiesConditions(data, i))
    {
      double v;
      if (_action->compute(data, i, v))
      {
	l->setVal(i, v);
      }
      else
      {
	l->setMissing(i);
      }
    }
  }
  return true;
}

//-------------------------------------------------------------------
bool LogicalNode::compute(const MathData *data, int ipt, double &v) const
{
  LOG(ERROR) << "Wrong method to call for a logical node";
  return false;
}

//-------------------------------------------------------------------
void LogicalNode::outputFields(std::vector<std::string> &names) const
{
  _action->outputFields(names);
}

//-------------------------------------------------------------------
void LogicalNode::inputFields(std::vector<std::string> &names) const
{
  _find.fields(names);
  _action->inputFields(names);
}

//-------------------------------------------------------------------
bool LogicalNode::getSimpleCompare(std::string &compareName, double &compareV,
				   bool &compareMissing,
				   MathFindSimple::Compare_t &c,
				   std::string &assignName,
				   double &assignV, bool &assignMissing) const
{
  if (!_find.getSimpleCompare(compareName, c, compareV, compareMissing))
  {
    return false;
  }
  if (_action->getType() != ProcessingNode::ASSIGNMENT)
  {
    return false;
  }
  return ((AssignmentNode *)_action->nodePtr())->getSimpleAssign(assignName,
								 assignV,
								 assignMissing);
}

//-------------------------------------------------------------------
bool LogicalNode::getMultiCompare(LogicalArgs &args,
				  std::string &assignName,
				  double &assignV,
				  bool &assignMissing) const
{
  if (!_find.getMultiCompare(args))
  {
    return false;
  }

  if (_action->getType() != ProcessingNode::ASSIGNMENT)
  {
    return false;
  }
  return ((AssignmentNode *)_action->nodePtr())->getSimpleAssign(assignName,
								 assignV,
								 assignMissing);
}

//-------------------------------------------------------------------
bool LogicalNode::getMultiCompare(LogicalArgs &args,
				  std::string &assignToName,
				  std::string &assignFromName) const
{
  if (!_find.getMultiCompare(args))
  {
    return false;
  }

  if (_action->getType() != ProcessingNode::ASSIGNMENT)
  {
    return false;
  }
  return ((AssignmentNode *)_action->nodePtr())->getSimpleAssign(assignFromName,
								 assignToName);

}

//-------------------------------------------------------------------
bool LogicalNode::getSimpleCompare(std::string &compareName, double &compareV,
				   bool &compareMissing, 
				   MathFindSimple::Compare_t &c,
				   std::string &assignToName,
				   std::string &assignFromName) const
{
  if (!_find.getSimpleCompare(compareName, c, compareV, compareMissing))
  {
    return false;
  }
  if (_action->getType() != ProcessingNode::ASSIGNMENT)
  {
    return false;
  }
  return ((AssignmentNode *)_action->nodePtr())->getSimpleAssign(assignFromName,
								 assignToName);

}
