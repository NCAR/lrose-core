#include <rapmath/ProcessingNode.hh>
#include <rapmath/AssignmentNode.hh>
#include <rapmath/BinaryNode.hh>
#include <rapmath/LeafNode.hh>
#include <rapmath/LogicalNode.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FunctionDef.hh>
#include <toolsa/LogStream.hh>

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(void) : _type(NOT_SET), 
				       _pattern(Node::DO_IT_THE_HARD_WAY),
				       _content(NULL),
				       _input("")
{
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &s) : _type(LEAF),
						       _content(NULL),
						       _input(s)
{
  _content = (Node *)(new LeafNode(s));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &s, double value) :
  _type(LEAF), _content(NULL), _input(s)
{
  _content = (Node *)(new LeafNode(s, value));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &input, ProcessingNode *left,
			       ProcessingNode *right, Operator_t op):
  _type(BINARY), _content(NULL), _input(input)
{
  _content = (Node *)(new BinaryNode(left, right, op));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &input,
			       ProcessingNode *left,
			       ProcessingNode *right,
			       const std::string &key):
  _type(BINARY), _content(NULL), _input(input)
{
  _content = (Node *)(new BinaryNode(left, right, key));
  _pattern = _content->pattern();
}


//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &input,  UnaryOperator_t op,
			       std::vector<ProcessingNode *> &args) :
  _type(UNARY), _content(NULL), _input(input)
{
  _content = (Node *)(new UnaryNode(op, args));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &input,
			       std::vector<ProcessingNode *> &args,
			       const std::string &key) :
  _type(UNARY), _content(NULL), _input(input)
{
  _content = (Node *)(new UnaryNode(args, key));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &input, const Find &find,
			       ProcessingNode *value) :
  _type(LOGICAL), _content(NULL), _input(input)
{
  _content = (Node *)(new LogicalNode(find, value));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::ProcessingNode(const std::string &input, const LeafContent &var,
			       ProcessingNode *value) :
  _type(ASSIGNMENT), _content(NULL), _input(input)
{
  _content = (Node *)(new AssignmentNode(var, value));
  _pattern = _content->pattern();
}

//-------------------------------------------------------------------
ProcessingNode::~ProcessingNode()
{
  if (_content != NULL)
  {
    delete _content;
  }
}
 
//-------------------------------------------------------------------
void ProcessingNode::cleanup(void)
{
  if (_content != NULL)
  {
    _content->cleanup();
  }
}
 
//-------------------------------------------------------------------
void ProcessingNode::print(void) const
{
  printf("%s\n", _input.c_str());
}

//-------------------------------------------------------------------
std::string ProcessingNode::sprint(void) const
{
  return _input;
}

//-------------------------------------------------------------------
void ProcessingNode::printParsed(void) const
{
  if (_content != NULL)
  {
    return _content->printParsed();
  }  
}

//-------------------------------------------------------------------
void ProcessingNode::printParsedCr(void) const
{
  if (_content != NULL)
  {
    return _content->printParsedCr();
  }
}

//-------------------------------------------------------------------
std::vector<FunctionDef> ProcessingNode::unaryOperators(void)
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(sprintUOp(ABS), sprintUOpDescr(ABS)));
  ret.push_back(FunctionDef(sprintUOp(SQRT), sprintUOpDescr(SQRT)));
  ret.push_back(FunctionDef(sprintUOp(LOG10), sprintUOpDescr(LOG10)));
  ret.push_back(FunctionDef(sprintUOp(EXP), sprintUOpDescr(EXP)));
  ret.push_back(FunctionDef(sprintUOp(SMOOTH), sprintUOpDescr(SMOOTH)));
  ret.push_back(FunctionDef(sprintUOp(SMOOTHDBZ),
			    sprintUOpDescr(SMOOTHDBZ)));
  ret.push_back(FunctionDef(sprintUOp(STDDEV), sprintUOpDescr(STDDEV)));
  ret.push_back(FunctionDef(sprintUOp(FUZZY), sprintUOpDescr(FUZZY)));
  ret.push_back(FunctionDef(sprintUOp(AVERAGE), sprintUOpDescr(AVERAGE)));
  ret.push_back(FunctionDef(sprintUOp(WEIGHTED_AVERAGE),
			    sprintUOpDescr(WEIGHTED_AVERAGE)));
  ret.push_back(FunctionDef(sprintUOp(MASK), sprintUOpDescr(MASK)));
  ret.push_back(FunctionDef(sprintUOp(MASK_MISSING_TO_MISSING),
			    sprintUOpDescr(MASK_MISSING_TO_MISSING)));
  ret.push_back(FunctionDef(sprintUOp(TRAPEZOID),
			    sprintUOpDescr(TRAPEZOID)));
  ret.push_back(FunctionDef(sprintUOp(S_REMAP), sprintUOpDescr(S_REMAP)));
  ret.push_back(FunctionDef(sprintUOp(MAX), sprintUOpDescr(MAX)));
  ret.push_back(FunctionDef(sprintUOp(MEDIAN), sprintUOpDescr(MEDIAN)));
  ret.push_back(FunctionDef(sprintUOp(WEIGHTED_ANGLE_AVERAGE),
			    sprintUOpDescr(WEIGHTED_ANGLE_AVERAGE)));
  ret.push_back(FunctionDef(sprintUOp(MAX_EXPAND), sprintUOpDescr(MAX_EXPAND)));
  ret.push_back(FunctionDef(sprintUOp(EXPAND_ANGLES_LATERALLY),
			    sprintUOpDescr(EXPAND_ANGLES_LATERALLY)));
  ret.push_back(FunctionDef(sprintUOp(CLUMP), sprintUOpDescr(CLUMP)));
  return ret;
}

//-------------------------------------------------------------------
ProcessingNode::UnaryOperator_t ProcessingNode::unaryOperatorValue(int index)
{
  switch (index)
  {
  case 0:
    return ABS;
  case 1:
    return SQRT;
  case 2:
    return LOG10;
  case 3:
    return EXP;
  case 4:
    return SMOOTH;
  case 5:
    return SMOOTHDBZ;
  case 6:
    return STDDEV;
  case 7:
    return FUZZY;
  case 8:
    return AVERAGE;
  case 9:
    return WEIGHTED_AVERAGE;
  case 10:
    return MASK;
  case 11:
    return MASK_MISSING_TO_MISSING;
  case 12:
    return TRAPEZOID;
  case 13:
    return S_REMAP;
  case 14:
    return MAX;
  case 15:
    return MEDIAN;
  case 16:
    return WEIGHTED_ANGLE_AVERAGE;
  case 17:
    return MAX_EXPAND;
  case 18:
    return EXPAND_ANGLES_LATERALLY;
  case 19:
    return CLUMP;
  default:
    return UBAD;
  }
}

//-------------------------------------------------------------------
ProcessingNode::Operator_t ProcessingNode::binaryOperatorValue(int index)
{
  switch (index)
  {
  case 0:
    return ADD;
  case 1:
    return SUB;
  case 2:
    return MULT;
  case 3:
    return DIV;
  case 4:
    return POW;
  default:
    return BAD;
  }
}

//-------------------------------------------------------------------
std::vector<FunctionDef> ProcessingNode::binaryOperators(void)
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(sprintOp(ADD), sprintOpDescr(ADD)));
  ret.push_back(FunctionDef(sprintOp(SUB), sprintOpDescr(SUB)));
  ret.push_back(FunctionDef(sprintOp(MULT), sprintOpDescr(MULT)));
  ret.push_back(FunctionDef(sprintOp(DIV), sprintOpDescr(DIV)));
  ret.push_back(FunctionDef(sprintOp(POW), sprintOpDescr(POW)));
  return ret;
}


//-------------------------------------------------------------------
std::string ProcessingNode::sprintOp(const Operator_t &op)
{
  std::string s = "unknown";
  switch (op)
  {
  case ADD:
    s = "+";
    break;
  case SUB:
    s = "-";
    break;
  case MULT:
    s = "*";
    break;
  case DIV:
    s = "/";
    break;
  case POW:
    s = "^";
    break;
  case USER:
    s = "";
    break;
  default:
    s = "BAD";
    break;
  }
  return s;
}

//-------------------------------------------------------------------
std::string ProcessingNode::sprintOpDescr(const Operator_t &op)
{
  std::string s = "unknown";
  switch (op)
  {
  case ADD:
    s = "Binary operator for simple addition  A+B";
    break;
  case SUB:
    s = "Binary operator for simple subtraction A-B";
    break;
  case MULT:
    s = "Binary operator for simple multiplication A*B";
    break;
  case DIV:
    s = "Binary operator for simple divisoin  A/B";
    break;
  case POW:
    s = "Binary operator for exponents, A^B = A raised to the power B";
    break;
  case USER:
    s = "";
    break;
  default:
    s = "BAD";
    break;
  }
  return s;
}

//-------------------------------------------------------------------
std::string ProcessingNode::sprintUOp(const UnaryOperator_t &op)
{
  std::string s = "unknown";
  switch (op)
  {
  case ABS:
    s = "abs";
    break;
  case SQRT:
    s = "sqrt";
    break;
  case LOG10:
    s = "log10";
    break;
  case EXP:
    s = "exp";
    break;
  case SMOOTH:
    s = "smooth";
    break;
  case SMOOTHDBZ:
    s = "smoothDBZ";
    break;
  case STDDEV:
    s = "stddev";
    break;
  case FUZZY:
    s = "fuzzy";
    break;
  case AVERAGE:
    s = "average";
    break;
  case MAX:
    s = "maximum";
    break;
  case MAX_EXPAND:
    s = "max_expand";
    break;
  case EXPAND_ANGLES_LATERALLY:
    s = "expand_angles_laterally";
    break;
  case CLUMP:
    s = "clump";
    break;
  case MEDIAN:
    s = "median";
    break;
  case WEIGHTED_ANGLE_AVERAGE:
    s = "weighted_angle_average";
    break;
  case WEIGHTED_AVERAGE:
    s = "weighted_average";
    break;
  case MASK:
    s = "mask";
    break;
  case MASK_MISSING_TO_MISSING:
    s = "mask_missing_to_missing";
    break;
  case TRAPEZOID:
    s = "trapezoid";
    break;
  case S_REMAP:
    s = "s_remap";
    break;
 case UUSER:
    s = "";
  default:
    s = "UBAD";
    break;
  }
  return s;
}
  
//-------------------------------------------------------------------
std::string ProcessingNode::sprintUOpDescr(const UnaryOperator_t &op)
{
  std::string s = "unknown";
  switch (op)
  {
  case ABS:
    s = "abs(x) = absolute value of input argument x";
    break;
  case SQRT:
    s = "sqrt(x) = square root of input argument x";
    break;
  case LOG10:
    s = "log10(x) = log base 10 of input argument x";
    break;
  case EXP:
    s = "exp(x) = exponential function, e to the x";
    break;
  case SMOOTH:
    s = "smooth(field, nx, ny) = nx by ny average of input field";
    break;
  case SMOOTHDBZ:
    s = "smoothDBZ(field, nx, ny) = 10*log10(nx by ny average of 10^(field/10))";
    break;
  case STDDEV:
    s = "stddev(field, nx, ny) = nx by ny standard deviation of field";
    break;
  case FUZZY:
    s = "fuzzy(field, x0,y0,x1,y1,x2,y2..) = fuzzy remapping of input field, mapping defined by the x,y pairs";
    break;
  case AVERAGE:
    s = "average(nptX, field0, field1, .. fieldn) = average of data from "
      "all the input fields at each point, with the data from 0 to nptX set to missing at all y";
    break;
  case MAX:
    s = "maximum(field0, field1, .. fieldn) = maximum of data from all the input fields";
    break;
  case MAX_EXPAND:
    s = "max_expand(field, nx, ny) = s(x,y) = maximum field value in a box nx,ny centered at x,y";
    break;
  case EXPAND_ANGLES_LATERALLY:
    s = "expand_angles_laterally(field, npt) = assumes field contains polar data with y=degrees.  Expands along the orientation into areas of missing data by npt at each point";
    break;
  case CLUMP:
    s = "clump(field, npt) = produce disjoint clumps of non-missing data, each with a different value, removing clumps that are smaller than npt in size";
    break;
  case MEDIAN:
    s = "median(field0, nx, ny, binMin, binMax, binDelta) = median of field data over an nx by ny region, with histogram bins as indicated, each bin binDelta in width";
    break;
  case WEIGHTED_ANGLE_AVERAGE:
    s = "weighted_angle_average(zero_to_360, field0,weight0, field1,weight1,...) = "
      "weighted average of all input fields, assumed to be angles in degrees. If zero_to_360 is 1, the angle values are over this range, if 0, the angle values go from 0 to 180.  (field0*weight0 + field1*weight1 + ... )/ (weight0 + weight1 + ..)";
    break;
  case WEIGHTED_AVERAGE:
    s = "weighted_average(nptX, field0, weight0, field1, weight1,...) = "
      "weighted average of all fields, normalized by sum of weights. "
      " data from 0 to nptX set to missing at all y";
    break;
  case MASK:
    s = "mask(field, low0,high0, low1,high1, ...) = filter out data from field to "
      "missing if the data value is in the range [low0,high0] or "
      "[low1,high1], .. etc";
    break;
  case MASK_MISSING_TO_MISSING:
    s = "mask_missing_to_missing(data, mask) = set data to missing "
      "whereever the mask is missing";
    break;
  case TRAPEZOID:
    s = "trapezoid(field, a,b,c,d) = max(min((x-a)/(b-a), 1, (d-x)/(d-c)),0)"
      " It must be true that a < b < c < d";
    break;
  case S_REMAP:
    s = "s_remap(field, a, b) = 0 n"
      "0 for x<= a\n"
      "2*((x-a)/(b-a))^2  for a <= x and x <= (a+b)/2\n"
      "1 - 2*((x-b)/(b-a))^2  for (a+b)/2 <= x and x <= b\n"
      "1  for x >= b";
    break;
 case UUSER:
    s = "";
  default:
    s = "";
    break;
  }
  return s;
}
  
//-------------------------------------------------------------------
bool ProcessingNode::isUserUnaryFunction(void) const
{
  if (_type != UNARY)
  {
    return false;
  }
  else
  {
    return ((const UnaryNode *)_content)->isUserFunction();
  }
}
    
//-------------------------------------------------------------------
bool ProcessingNode::isMultiArgUnaryFunction(void) const
{
  if (_type != UNARY)
  {
    return false;
  }
  else
  {
    return ((const UnaryNode *)_content)->isMultiArgFunction();
  }
}
    
//-------------------------------------------------------------------
bool ProcessingNode::getLeafVariableName(std::string &s) const
{
  if (_type == LEAF)
  {
    return ((const LeafNode *)_content)->getLeafVariableName(s);
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
bool ProcessingNode::getLeafNumberOrMissing(double &v, bool &isMissing) const
{
  if (_type == LEAF)
  {
    return ((const LeafNode *)_content)->getLeafNumberOrMissing(v, isMissing);
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
bool ProcessingNode::isSimpleBinary(void) const
{
  if (_type != BINARY)
  {
    return false;
  }
  else
  {
    return ((BinaryNode *)_content)->isSimple();
  }
}

//-------------------------------------------------------------------
bool ProcessingNode::getSimpleAssign(std::string &name, double &number,
				     bool &missing) const
{
  if (_type != ASSIGNMENT)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  if (_pattern != Node::SIMPLE_ASSIGN_NUMBER_TO_VAR &&
      _pattern != Node::SIMPLE_ASSIGN_MISSING_TO_VAR)
  {
    LOG(ERROR) << "Wrong pattern";
    return false;
  }
  return ((AssignmentNode *)_content)->getSimpleAssign(name, number, missing);
}


//-------------------------------------------------------------------
bool ProcessingNode::getSimpleAssign(std::string &from, std::string &to) const
{
  if (_type != ASSIGNMENT)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  if (_pattern != Node::SIMPLE_ASSIGN_VAR_TO_VAR)
  {
    LOG(ERROR) << "Wrong pattern";
    return false;
  }
  return ((AssignmentNode *)_content)->getSimpleAssign(from, to);
}

//-------------------------------------------------------------------
bool ProcessingNode::getSimpleCompare(std::string &compareName,
				      double &compareV,  bool &compareMissing,
				      MathFindSimple::Compare_t &c,
				      std::string &assignName, double &assignV,
				      bool &assignMissing) const
{
  if (_type != LOGICAL)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  if (_pattern != Node::LOGICAL_SIMPLE_ASSIGN_NUMBER_TO_VAR)
  {
    LOG(ERROR) << "Wrong pattern";
    return false;
  }
  return ((LogicalNode *)_content)->getSimpleCompare(compareName, compareV,
						     compareMissing, c,
						     assignName, assignV,
						     assignMissing);
}

//-------------------------------------------------------------------
bool ProcessingNode::getMultiCompare(LogicalArgs &args, std::string &assignName,
				     double &assignV, bool &assignMissing) const
{
  if (_type != LOGICAL)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  if (_pattern != Node::LOGICAL_MULTIPLE_SIMPLE_ASSIGN_NUMBER_TO_VAR)
  {
    LOG(ERROR) << "Wrong pattern";
    return false;
  }
  return ((LogicalNode *)_content)->getMultiCompare(args, assignName,
						    assignV, assignMissing);
}

//-------------------------------------------------------------------
bool ProcessingNode::getMultiCompare(LogicalArgs &args,
				     std::string &assignToName,
				     std::string &assignFromName) const
{
  if (_type != LOGICAL)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  if (_pattern != Node::LOGICAL_MULTIPLE_SIMPLE_ASSIGN_NUMBER_TO_VAR)
  {
    LOG(ERROR) << "Wrong pattern";
    return false;
  }
  return ((LogicalNode *)_content)->getMultiCompare(args,  assignToName,
						    assignFromName);
}

//-------------------------------------------------------------------
bool ProcessingNode::getAssignName(std::string &name) const
{
  if (_type != ASSIGNMENT)
  {
    return false;
  }
  return ((AssignmentNode *)_content)->getAssignedName(name);
}

//-------------------------------------------------------------------
bool ProcessingNode::getAssignSimpleBinaryArgs(BinaryArgs &args) const
{
  if (_type != ASSIGNMENT)
  {
    return false;
  }
  else
  {
    return ((AssignmentNode *)_content)->getSimpleBinaryArgs(args);
  }
}

//-------------------------------------------------------------------
bool ProcessingNode::getSimpleCompare(std::string &compareName,
				      double &compareV,  bool &compareMissing, 
				      MathFindSimple::Compare_t &c,
				      std::string &assignToName,
				      std::string &assignFromName) const
{
  if (_type != LOGICAL)
  {
    LOG(ERROR) << "Wrong method";
    return false;
  }
  if (_pattern != Node::LOGICAL_SIMPLE_ASSIGN_VAR_TO_VAR)
  {
    LOG(ERROR) << "Wrong pattern";
    return false;
  }
  return ((LogicalNode *)_content)->getSimpleCompare(compareName, compareV,
						     compareMissing, c,
						     assignToName,
						     assignFromName);
}


//-------------------------------------------------------------------
MathUserData *ProcessingNode::processVol(VolumeData *data) const

{
  return _content->processVol(data);
}

//-------------------------------------------------------------------
bool ProcessingNode::process(MathData *data) const
{
  return _content->process(data);
}

//-------------------------------------------------------------------
MathUserData *ProcessingNode::processToUserDefined(MathData *data) const
{
  return _content->processToUserDefined(data);
}

//-------------------------------------------------------------------
bool ProcessingNode::compute(const MathData *data, int ipt, double &v) const
{
  return _content->compute(data, ipt, v);
}

//-------------------------------------------------------------------
void ProcessingNode::outputFields(std::vector<std::string> &names) const
{
  _content->outputFields(names);
}

//-------------------------------------------------------------------
void
ProcessingNode::inputFields(std::vector<std::string> &names) const
{
  _content->inputFields(names);
}

//-------------------------------------------------------------------
bool ProcessingNode::isVariable(void) const
{
  if (_type == LEAF)
  {
    return ((LeafNode *)(_content))->isVariable();
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
LeafContent ProcessingNode::getLeafContent(void) const
{
  if (_type == LEAF)
  {
    return ((LeafNode *)(_content))->getLeafContent();
  }
  else
  {
    LOG(WARNING) << "Unexpected failure to get item";
    return LeafContent();
  }
}  

//-------------------------------------------------------------------
bool ProcessingNode::isUserAssignmentWithUnaryOp(std::string &keyword,
						 bool warn) const
{
  if (_type == ASSIGNMENT)
  {
    return ((AssignmentNode *)(_content))->isUnaryUserOpRightHandSide(keyword,
								      warn);
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
bool ProcessingNode::isUserUnaryOp(std::string &keyword, bool warn) const
{
  if (_type == UNARY)
  {
    return ((UnaryNode *)(_content))->getUserUnaryKeyword(keyword, warn);
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
std::vector<ProcessingNode *> *ProcessingNode::unaryOpArgs(void)
{
  if (_type == UNARY)
  {
    return ((UnaryNode *)(_content))->unaryOpArgs();
  }
  else
  {
    return NULL;
  }
}

//-------------------------------------------------------------------
bool ProcessingNode::getValue(double &v) const
{
  if (_type == LEAF)
  {
    return ((LeafNode *)(_content))->getValue(v);
  }
  else
  {
    return false;
  }
}

//-------------------------------------------------------------------
std::string ProcessingNode::leafName(void) const
{
  if (_type == LEAF)
  {
    return ((LeafNode *)(_content))->getName();
  }
  else
  {
    return "";
  }
}

//-------------------------------------------------------------------
ProcessingNode::UnaryOperator_t ProcessingNode::getUnaryOperator(void) const
{
  if (_type == UNARY)
  {
    return ((UnaryNode *)(_content))->getOp();
  }
  else
  {
    return UBAD;
  }
}  
